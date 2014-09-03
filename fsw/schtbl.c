/* 
** File:
**   $Id: $
**
** Purpose: Implement the scheduler table
**
** Notes
**   1. Adopted from the GPM MCP750 scheduler application
**
** References:
**   1. Core Flight Executive Application Developers Guide.
**   2. The GN&C FSW Framework Programmer's Guide
**
**
** $Date: $
** $Revision: $
** $Log: $
*/

/*
** Include Files:
*/

#include "schtbl.h"
#include "cfe_time_msg.h"


#if (OS_SUCCESS != CFE_SUCCESS)
   #error Code assumes OS_SUCCESS equals CFE_SUCCESS
#endif


/*
** File Function Prototypes
*/

static void   MajorFrameCallback(void);
static void   MinorFrameCallback(uint32 TimerId);
static uint32 GetCurrentSlotNumber(void);
static uint32 GetMETSlotNumber(void);
static int32  ProcessNextSlot(void);


/*
** Global File Data
*/

static SCHTBL_Class*  SchTbl = NULL;

/*
** Exported Functions
*/


/******************************************************************************
** Function: SCHTBL_Constructor
**
*/
void SCHTBL_Constructor(SCHTBL_Class* SchTblPtr)
{

   int32 Status = CFE_SUCCESS;

   SchTbl = SchTblPtr;

   SchTbl->SlotsProcessedCount = 0;
   SchTbl->SkippedSlotsCount   = 0;
   SchTbl->MultipleSlotsCount  = 0;
   SchTbl->SameSlotCount       = 0;
   SchTbl->ScheduleActivitySuccessCount = 0;
   SchTbl->ScheduleActivityFailureCount = 0;

   /*
   ** Start off assuming Major Frame synch is normal
   ** and should be coming at any moment
   */
   SchTbl->SendNoisyMajorFrameMsg = TRUE;
   SchTbl->IgnoreMajorFrame       = FALSE;
   SchTbl->UnexpectedMajorFrame   = FALSE;
   SchTbl->SyncToMET              = SCHTBL_SYNCH_FALSE;
   SchTbl->MajorFrameSource       = SCHTBL_MF_SRC_NONE;
   SchTbl->NextSlotNumber         = 0;
   SchTbl->MinorFramesSinceTone   = SCHTBL_TIME_SYNC_SLOT;
   SchTbl->LastSyncMETSlot        = 0;
   SchTbl->SyncAttemptsLeft       = 0;
   SchTbl->UnexpectedMajorFrameCount   = 0;
   SchTbl->MissedMajorFrameCount       = 0;
   SchTbl->ValidMajorFrameCount        = 0;
   SchTbl->WorstCaseSlotsPerMinorFrame = 1;

   /*
   ** Configure Major Frame and Minor Frame sources
   */
   SchTbl->ClockAccuracy = SCHTBL_WORST_CLOCK_ACCURACY;

   /*
   ** Create an OSAL timer to drive the Minor Frames
   */
   Status = OS_TimerCreate(&SchTbl->TimerId,
                           SCHTBL_TIMER_NAME,
                           &SchTbl->ClockAccuracy,
                           MinorFrameCallback);

    if (Status != OS_SUCCESS) {

        CFE_EVS_SendEvent(SCHTBL_MINOR_FRAME_TIMER_CREATE_ERR_EID, CFE_EVS_ERROR,
                          "Error creating Minor Frame Timer (RC=0x%08X)",
                          Status);
    }
    else {

       /*
       ** Determine if the timer has an acceptable clock accuracy
       */
       if (SchTbl->ClockAccuracy > SCHTBL_WORST_CLOCK_ACCURACY) {

          CFE_EVS_SendEvent(SCHTBL_MINOR_FRAME_TIMER_ACC_WARN_EID, CFE_EVS_INFORMATION,
                          "OS Timer Accuracy (%d > reqd %d usec) requires Minor Frame MET sync",
                          SchTbl->ClockAccuracy, SCHTBL_WORST_CLOCK_ACCURACY);

           /* Synchronize Minor Frame Timing with Mission Elapsed Time to keep from losing slots */
           SchTbl->SyncToMET = SCHTBL_SYNCH_TO_MINOR;

           /* Calculate how many slots we may have to routinely process on each Minor Frame Wakeup */
           SchTbl->WorstCaseSlotsPerMinorFrame = ((SchTbl->ClockAccuracy * 2) / SCHTBL_NORMAL_SLOT_PERIOD) + 1;

       } /* End if bad accuracy */

       /*
       ** Create main task semaphore (given by MajorFrameCallback and MinorFrameCallback)
       */

       Status = OS_BinSemCreate(&SchTbl->TimeSemaphore, SCHTBL_SEM_NAME, SCHTBL_SEM_VALUE, SCHTBL_SEM_OPTIONS);

       if (Status != CFE_SUCCESS) {

          CFE_EVS_SendEvent(SCHTBL_SEM_CREATE_ERR_EID, CFE_EVS_ERROR,
                          "Error creating Main Loop Timing Semaphore (RC=0x%08X)",
                          Status);

       } /* End if binary semaphore created */

    } /* End if minor frame timer created */

    /* LoadTestTable();  /* TODO - Remove once had real table processing */

} /* End SCHTBL_Constructor() */

/*******************************************************************
** Function: SCHTBL_ResetStatus
**
*/
void SCHTBL_ResetStatus()
{

   SchTbl->SlotsProcessedCount          = 0;
   SchTbl->SkippedSlotsCount            = 0;
   SchTbl->MultipleSlotsCount           = 0;
   SchTbl->SameSlotCount                = 0;
   SchTbl->ScheduleActivitySuccessCount = 0;
   SchTbl->ScheduleActivityFailureCount = 0;
   SchTbl->ValidMajorFrameCount         = 0;
   SchTbl->MissedMajorFrameCount        = 0;
   SchTbl->UnexpectedMajorFrameCount    = 0;
   SchTbl->TablePassCount               = 0;
   SchTbl->ConsecutiveNoisyFrameCounter = 0;
   SchTbl->IgnoreMajorFrame             = FALSE;

} /* End SCHTBL_ResetStatus() */


/*******************************************************************
** Function: SCHTBL_GetTblPtr
**
*/
const SCHTBL_Table* SCHTBL_GetTblPtr()
{

   return &(SchTbl->Table);

} /* End SCHTBL_GetTblPtr() */


/******************************************************************************
** Function: SCHTBL_StartTimers
**
*/
int32 SCHTBL_StartTimers(void)
{

   int32 Status = CFE_SUCCESS;

   /*
   ** Connect to cFE TIME's time reference marker (typically 1 Hz)
   ** to use it as the Major Frame synchronization source
   */

   Status = CFE_TIME_RegisterSynchCallback((CFE_TIME_SynchCallbackPtr_t)&MajorFrameCallback);

   if (Status != CFE_SUCCESS) {

      CFE_EVS_SendEvent(SCHTBL_MAJOR_FRAME_SUB_ERR_EID, CFE_EVS_ERROR,
                       "Error Subscribing to Major Frame Tone (RC=0x%08X)",
                       Status);
   }
   else {

      /*
      ** Start the Minor Frame Timer with an extended delay to allow a Major Frame Sync
      ** to start processing.  If the Major Frame Sync fails to arrive, then we will
      ** start when this timer expires and synch ourselves to the MET clock.
      */
      Status = OS_TimerSet(SchTbl->TimerId, SCHTBL_STARTUP_PERIOD, 0);

   }

   return (Status);

} /* End SCHTBL_StartTimers() */


/*******************************************************************
**
** SCHTBL_ProcessTable
**
*/
boolean SCHTBL_ProcessTable(void)
{
   uint32  CurrentSlot;
   uint32  ProcessCount;
   int32   Result = CFE_SUCCESS;

   /* Wait for the next slot (Major or Minor Frame) */
   Result = OS_BinSemTake(SchTbl->TimeSemaphore);

   if (Result == OS_SUCCESS)
   {

      /* TODO - CFE_EVS_SendEvent(999, CFE_EVS_INFORMATION, "ProcessTable::OS_BinSemTake() success");*/

      if (SchTbl->IgnoreMajorFrame)
      {
         if (SchTbl->SendNoisyMajorFrameMsg)
         {
            CFE_EVS_SendEvent(SCHTBL_NOISY_MAJOR_FRAME_ERR_EID, CFE_EVS_ERROR,
                              "Major Frame Sync too noisy (Slot %d). Disabling synchronization.",
                              SchTbl->MinorFramesSinceTone);
            SchTbl->SendNoisyMajorFrameMsg = FALSE;
         }
      } /* End if ignore Major Frame */
      else
      {
         SchTbl->SendNoisyMajorFrameMsg = TRUE;
      }

      CurrentSlot = GetCurrentSlotNumber();

      /* Compute the number of slots we need to process (watch for rollover) */
      if (CurrentSlot < SchTbl->NextSlotNumber)
      {
         ProcessCount = SCHTBL_TOTAL_SLOTS - SchTbl->NextSlotNumber;
         ProcessCount += (CurrentSlot + 1);
      }
      else
      {
         ProcessCount = (CurrentSlot - SchTbl->NextSlotNumber) + 1;
      }

      /* CFE_EVS_SendEvent(999, CFE_EVS_INFORMATION, "ProcessTable::CurrentSlot=%d, First ProcessCount=%d", CurrentSlot, ProcessCount); */

      /*
      ** Correct for the following conditions observed when minor frame driven
      ** by a clock with poor accuracy
      **
      **   1) Wake up a little too late for just 1 slot
      **      symptom = multi slots event followed by same slot event
      **
      **   2) Wake up a little too early for just 1 slot
      **      symptom = same slot event followed by multi slots event
      */
      if (ProcessCount == 2)
      {
         /*
         ** If we want to do 2 slots but last time was OK then assume we
         **    are seeing condition #1 above.  By doing just 1 slot now,
         **    there will still be 1 to do when the next wakeup occurs
         **    and we will avoid both events.  But, if we really are in
         **    a delayed state, we will process both slots when we wake
         **    up next time because then the last time will NOT be OK.
         */
         if (SchTbl->LastProcessCount == 1)
         {
            ProcessCount = 1;
         }
         SchTbl->LastProcessCount = 2;
      }
      else if (ProcessCount == SCHTBL_TOTAL_SLOTS)
      {
         /* Same as previous comment except in reverse order. */
         if (SchTbl->LastProcessCount != SCHTBL_TOTAL_SLOTS)
         {
            ProcessCount = 1;
         }
         SchTbl->LastProcessCount = SCHTBL_TOTAL_SLOTS;
      }
      else
      {
         SchTbl->LastProcessCount = ProcessCount;
      }

      /*
      ** If current slot = next slot - 1, assume current slot did not increment
      */
      if (ProcessCount == SCHTBL_TOTAL_SLOTS)
      {
         SchTbl->SameSlotCount++;

         CFE_EVS_SendEvent(SCHTBL_SAME_SLOT_EID, CFE_EVS_DEBUG,
                           "Slot did not increment: slot = %d",
                           CurrentSlot);
         ProcessCount = 0;
      }

      /* If we are too far behind, jump forward and do just the current slot */
      if (ProcessCount > SCHTBL_MAX_LAG_COUNT)
      {
         SchTbl->SkippedSlotsCount++;

         CFE_EVS_SendEvent(SCHTBL_SKIPPED_SLOTS_EID, CFE_EVS_ERROR,
                           "Slots skipped: slot = %d, count = %d",
                           SchTbl->NextSlotNumber, (ProcessCount - 1));

         /*
         ** Update the pass counter if we are skipping the rollover slot
         */
         if (CurrentSlot < SchTbl->NextSlotNumber)
         {
            SchTbl->TablePassCount++;
         }

         /*
         ** Process ground commands if we are skipping the time synch slot
         ** NOTE: This assumes the Time Synch Slot is the LAST Schedule slot
         **       (see definition of SCH_TIME_SYNC_SLOT in sch_app.h)
         ** Ground commands should only be processed at the end of the schedule table
         ** so that Group Enable/Disable commands do not change the state of entries
         ** in the middle of a schedule.
         */
         if ((SchTbl->NextSlotNumber + ProcessCount) > SCHTBL_TIME_SYNC_SLOT)
         {
             /* TODO -  Move to App level Result = SCH_ProcessCommands(); */
         }

         SchTbl->NextSlotNumber = CurrentSlot;
         ProcessCount = 1;

      } /* End if (ProcessCount > SCHTBL_MAX_LAG_COUNT) */

      /*
      ** Don't try to catch up all at once, just do a couple
      */
      if (ProcessCount > SCHTBL_MAX_SLOTS_PER_WAKEUP)
      {
         ProcessCount = SCHTBL_MAX_SLOTS_PER_WAKEUP;
      }

      /* Keep track of multi-slot processing */
      if (ProcessCount > 1)
      {
         SchTbl->MultipleSlotsCount++;

         /* Generate an event message if not syncing to MET or when there is more than two being processed */
         if ((ProcessCount > SchTbl->WorstCaseSlotsPerMinorFrame) || (SchTbl->SyncToMET == SCHTBL_SYNCH_FALSE))
         {
            CFE_EVS_SendEvent(SCHTBL_MULTI_SLOTS_EID, CFE_EVS_INFORMATION,
                             "Multiple slots processed: slot = %d, count = %d",
                             SchTbl->NextSlotNumber, ProcessCount);
         }

      } /* End if ProcessCount > 1) */

      /* TODO - CFE_EVS_SendEvent(999, CFE_EVS_INFORMATION, "ProcessTable::Final ProcessCount=%d", ProcessCount);*/
      /* Process the slots (most often this will be just one) */
      while ((ProcessCount != 0) && (Result == CFE_SUCCESS))
      {
         Result = ProcessNextSlot();
         ProcessCount--;
      }

   } /* End Semaphore */

   return(Result == CFE_SUCCESS);

} /* End of SCHTBL_ProcessTable() */


/*******************************************************************
** Function: MajorFrameCallback
**
*/
void MajorFrameCallback(void)
{
    /*
    ** Synchronize slot zero to the external tone signal
    */
    uint16 StateFlags;

    CFE_EVS_SendEvent(999, CFE_EVS_INFORMATION, "MajorFrameCallback()");
    
    /*
    ** If cFE TIME is in FLYWHEEL mode, then ignore all synchronization signals
    */
    StateFlags = CFE_TIME_GetClockInfo();

    if ((StateFlags & CFE_TIME_FLAG_FLYING) == 0)
    {
        /*
        ** Determine whether the major frame is noisy or not
        **
        ** Conditions below are as follows:
        **    If we are NOT synchronized to the MET (i.e. - the Minor Frame timer
        **    has an acceptable resolution), then the Major Frame signal should
        **    only occur in the last slot of the schedule table.
        **
        **    If we ARE synchronized to the MET (i.e. - the Minor Frame timer is
        **    not as good as we would like), then the Major Frame signal should
        **    occur within a window of slots at the end of the table.
        */
        if (((SchTbl->SyncToMET == SCHTBL_SYNCH_FALSE) &&
             (SchTbl->MinorFramesSinceTone != SCHTBL_TIME_SYNC_SLOT)) ||
            ((SchTbl->SyncToMET == SCHTBL_SYNCH_TO_MINOR) &&
             (SchTbl->NextSlotNumber != 0) &&
             (SchTbl->NextSlotNumber <
              (SCHTBL_TOTAL_SLOTS - SchTbl->WorstCaseSlotsPerMinorFrame - 1))))
        {
            /*
            ** Count the number of consecutive noisy major frames and the Total number
            ** of noisy major frames.  Also, indicate in telemetry that this particular
            ** Major Frame signal is considered noisy.
            */
            SchTbl->UnexpectedMajorFrame = TRUE;
            SchTbl->UnexpectedMajorFrameCount++;

            /*
            ** If the Major Frame is not being ignored yet, then increment the consecutive noisy
            ** Major Frame counter.
            */
            if (!SchTbl->IgnoreMajorFrame)
            {
                SchTbl->ConsecutiveNoisyFrameCounter++;

                /*
                ** If the major frame is too "noisy", then send event message and ignore future signals
                */
                if (SchTbl->ConsecutiveNoisyFrameCounter >= SCHTBL_MAX_NOISY_MF)
                {
                    SchTbl->IgnoreMajorFrame = TRUE;
                }
            }
        }
        else /* Major Frame occurred when expected */
        {
            SchTbl->UnexpectedMajorFrame = FALSE;
            SchTbl->ConsecutiveNoisyFrameCounter = 0;
        }

        /*
        ** Ignore this callback if SCH has detected a noisy Major Frame Synch signal
        */
        if (SchTbl->IgnoreMajorFrame == FALSE)
        {
            /*
            ** Stop Minor Frame Timer (which should be waiting for an unusually long
            ** time to allow the Major Frame source to resynchronize timing) and start
            ** it again with nominal Minor Frame timing
            */
            OS_TimerSet(SchTbl->TimerId, SCHTBL_NORMAL_SLOT_PERIOD, SCHTBL_NORMAL_SLOT_PERIOD);

            /*
            ** Increment Major Frame process counter
            */
            SchTbl->ValidMajorFrameCount++;

            /*
            ** Set current slot = zero to synchronize activities
            */
            SchTbl->MinorFramesSinceTone = 0;

            /*
            ** Major Frame Source is now from CFE TIME
            */
            SchTbl->MajorFrameSource = SCHTBL_MF_SOURCE_CFE_TIME;

            /* Clear any Major Frame In Sync with MET flags */
            /* But keep the Minor Frame In Sync with MET flag if it is set */
            SchTbl->SyncToMET &= SCHTBL_SYNCH_TO_MINOR;

            /*
            ** Give "wakeup SCH" semaphore
            */
            OS_BinSemGive(SchTbl->TimeSemaphore);

        } /* End if IgnoreMajorFrame == FLASE */

    } /* End if clock not fly wheeling */

    /*
    ** We should assume that the next Major Frame will be in the same MET slot as this
    */
    SchTbl->LastSyncMETSlot = GetMETSlotNumber();

    return;

} /* End MajorFrameCallback() */


/*******************************************************************
** Function: MinorFrameCallback
**
*/
void MinorFrameCallback(uint32 TimerId)
{
    uint32  CurrentSlot;


    /* TODO - CFE_EVS_SendEvent(999, CFE_EVS_INFORMATION, "MinorFrameCallback()");*/
    
    /*
    ** If this is the very first timer interrupt, then the initial
    ** Major Frame Synchronization timed out.  This can occur when
    ** either the signal is not arriving or the clock has gone into
    ** FLYWHEEL mode.  We should synchronize to the MET time instead.
    */
    if (SchTbl->MajorFrameSource == SCHTBL_MF_SOURCE_NONE)
    {
        SchTbl->MajorFrameSource = SCHTBL_MF_SOURCE_MINOR_FRAME_TIMER;

        /* Synchronize timing to MET */
        SchTbl->SyncToMET |= SCHTBL_SYNCH_MAJOR_PENDING;
        SchTbl->SyncAttemptsLeft = SCHTBL_MAX_SYNC_ATTEMPTS;
        SchTbl->LastSyncMETSlot = 0;
    }

    /* If attempting to synchronize the Major Frame with MET, then wait for zero subsecs before starting */
    if (((SchTbl->SyncToMET & SCHTBL_SYNCH_MAJOR_PENDING) != 0) &&
        (SchTbl->MajorFrameSource == SCHTBL_MF_SOURCE_MINOR_FRAME_TIMER))
    {
        /* Whether we have found the Major Frame Start or not, wait another slot */
        OS_TimerSet(SchTbl->TimerId, SCHTBL_NORMAL_SLOT_PERIOD, SCHTBL_NORMAL_SLOT_PERIOD);

        /* Determine if this was the last attempt */
        SchTbl->SyncAttemptsLeft--;

        CurrentSlot = GetMETSlotNumber();
        if ((CurrentSlot != 0) && (SchTbl->SyncAttemptsLeft > 0))
        {
            return;
        }
        else  /* Synchronization achieved (or at least, aborted) */
        {
            /* Clear the pending synchronization flag and set the "Major In Sync" flag */
            SchTbl->SyncToMET &= ~SCHTBL_SYNCH_MAJOR_PENDING;
            SchTbl->SyncToMET |= SCHTBL_SYNCH_TO_MAJOR;

            /* CurrentSlot should be equal to zero.  If not, this is the best estimate we can use */
            SchTbl->MinorFramesSinceTone = CurrentSlot;
            SchTbl->LastSyncMETSlot = 0;
        }
    }
    else
    {
        /*
        ** If we are already synchronized with MET or don't care to be, increment current slot
        */
        SchTbl->MinorFramesSinceTone++;
    }

    if (SchTbl->MinorFramesSinceTone >= SCHTBL_TOTAL_SLOTS)
    {
        /*
        ** If we just rolled over from the last slot to slot zero,
        ** It means that the Major Frame Callback did not cancel the
        ** "long slot" timer that was started in the last slot
        **
        ** It also means that we may now need a "short slot"
        ** timer to make up for the previous long one
        */
        OS_TimerSet(SchTbl->TimerId, SCHTBL_SHORT_SLOT_PERIOD, SCHTBL_NORMAL_SLOT_PERIOD);

        SchTbl->MinorFramesSinceTone = 0;

        SchTbl->MissedMajorFrameCount++;
    }

    /*
    ** Determine the timer delay value for the next slot
    */
    if (SchTbl->MinorFramesSinceTone == SCHTBL_TIME_SYNC_SLOT)
    {
        /*
        ** Start "long slot" timer (should be stopped by Major Frame Callback)
        */
        OS_TimerSet(SchTbl->TimerId, SCHTBL_SYNC_SLOT_PERIOD, 0);
    }

    /*
    ** Note that if this is neither the first "short" minor frame nor the
    ** last "long" minor frame, the timer is not modified.  This should
    ** provide more stable timing than introducing the dither associated
    ** with software response times to timer interrupts.
    */

    /*
    ** Give "wakeup SCH" semaphore
    */
    OS_BinSemGive(SchTbl->TimeSemaphore);

    return;

} /* End MinorFrameCallback() */

/*******************************************************************
**
** GetCurrentSlotNumber
**
*/
static uint32 GetCurrentSlotNumber(void)
{
    uint32  CurrentSlot;

    if (SchTbl->SyncToMET != SCHTBL_SYNCH_FALSE)
    {
        CurrentSlot = GetMETSlotNumber();

        /*
        ** If we are only concerned with synchronizing the minor frames to an MET,
        ** then we need to adjust the current slot by whatever MET time is prevalent
        ** when the Major Frame Signal is received.
        ** If we are synchronizing the Major Frame, then, by definition, LastSyncMETSlot
        ** would be a zero and the current slot would be appropriate.
        */
        if (CurrentSlot < SchTbl->LastSyncMETSlot)
        {
            CurrentSlot = CurrentSlot + SCHTBL_TOTAL_SLOTS - SchTbl->LastSyncMETSlot;
        }
        else
        {
            CurrentSlot = CurrentSlot - SchTbl->LastSyncMETSlot;
        }
    }
    else
    {
        CurrentSlot = SchTbl->MinorFramesSinceTone;
    }

    return CurrentSlot;

} /* End GetCurrentSlotNumber() */

/*******************************************************************
**
** GetMETSlotNumber
**
*/
static uint32 GetMETSlotNumber(void)
{
    uint32 SubSeconds = 0;
    uint32 MicroSeconds;
    uint32 Remainder;
    uint32 METSlot;

    /*
    ** Use MET rather than current time to avoid time changes
    */
    SubSeconds = CFE_TIME_GetMETsubsecs();

    /*
    ** Convert sub-seconds to micro-seconds
    */
    MicroSeconds = CFE_TIME_Sub2MicroSecs(SubSeconds);

    /*
    ** Calculate schedule table slot number
    */
    METSlot = (MicroSeconds / SCHTBL_NORMAL_SLOT_PERIOD);

    /*
    ** Check to see if close enough to round up to next slot
    */
    Remainder = MicroSeconds - (METSlot * SCHTBL_NORMAL_SLOT_PERIOD);

    /*
    ** Add one more microsecond and see if it is sufficient to add another slot
    */
    Remainder += 1;
    METSlot += (Remainder / SCHTBL_NORMAL_SLOT_PERIOD);

    /*
    ** Check to see if the Current Slot number needs to roll over
    */
    if (METSlot == SCHTBL_TOTAL_SLOTS)
    {
        METSlot = 0;
    }

    return METSlot;

} /* end GetMETSlotNumber() */


/*******************************************************************
**
** ProcessNextSlot
**
*/
static int32 ProcessNextSlot(void)
{
    int32  Result = CFE_SUCCESS; /* TODO - Fix after resolve ground command processing */
    int32  EntryNumber;
    int32  SlotIndex;
    uint32 Remainder;
    SCHTBL_Entry *NextEntry;

    SlotIndex = SchTbl->NextSlotNumber * SCHTBL_ENTRIES_PER_SLOT;
    NextEntry = &SchTbl->Table.Entry[SlotIndex];

    /* Process each enabled entry in the schedule table slot */
    for (EntryNumber = 0; EntryNumber < SCHTBL_ENTRIES_PER_SLOT; EntryNumber++)
    {
        if (NextEntry->Enabled == TRUE)
        {

           Remainder = SchTbl->TablePassCount % NextEntry->Frequency;

           if (Remainder == NextEntry->Offset)
           {

              
               /* CFE_EVS_SendEvent(SCHTBL_PACKET_SEND_ERR_EID, CFE_EVS_INFORMATION,"Processed scheduler table slot %d, entry %d, msgid %d", SchTbl->NextSlotNumber, EntryNumber, NextEntry->MsgTblEntryId); */
               
               if (MSGTBL_SendMsg(NextEntry->MsgTblEntryId))
               {
                  SchTbl->ScheduleActivitySuccessCount++;
               }
               else
               {
                  NextEntry->Enabled = FALSE;
                  SchTbl->ScheduleActivityFailureCount++;

                  CFE_EVS_SendEvent(SCHTBL_PACKET_SEND_ERR_EID, CFE_EVS_ERROR,
                                    "Activity error: slot = %d, entry = %d, err = 0x%08X",
                                    SchTbl->NextSlotNumber, EntryNumber, Result);
               }
           } /* End if offset met */

        } /* End if entry is enabled */

        NextEntry++;

    } /* Entries per slot loop */

    /*
    ** Process ground commands in the slot reserved for time synch
    ** Ground commands should only be processed at the end of the schedule table
    ** so that Group Enable/Disable commands do not change the state of entries
    ** in the middle of a schedule.
    */
    if (SchTbl->NextSlotNumber == SCHTBL_TIME_SYNC_SLOT)
    {
        /* TODO - Move to app level Result = SCH_ProcessCommands(); */
    }

    SchTbl->NextSlotNumber++;

    if (SchTbl->NextSlotNumber == SCHTBL_TOTAL_SLOTS)
    {
       SchTbl->NextSlotNumber = 0;
       SchTbl->TablePassCount++;
    }

    SchTbl->SlotsProcessedCount++;

    return(Result);

} /* End ProcessNextSlot() */

/*******************************************************************
**
** SCHTBL_LoadTable
**
*/
void SCHTBL_LoadTable(SCHTBL_Table* NewTable)
{

   CFE_PSP_MemCpy(&(SchTbl->Table), NewTable, sizeof(SCHTBL_Table));

} /* End SCHTBL_LoadTable() */

/*******************************************************************
**
** SCHTBL_LoadTableEntry
**
*/
void SCHTBL_LoadTableEntry(uint16 SlotId, uint16 SlotEntry, SCHTBL_Entry* NewEntry)
{
   uint16 i = SCHTBL_INDEX(SlotId,SlotEntry);

   CFE_PSP_MemCpy(&(SchTbl->Table.Entry[i]),NewEntry,sizeof(SCHTBL_Entry));

} /* End SCHTBL_LoadTableEntry() */

/*******************************************************************
**
** Configure (Enable/Disable) a single entry
**
*/
void SCHTBL_ConfigureTableEntry(uint16 SlotId, uint16 SlotEntry, boolean EnableFlag)
{

   uint16 i = SCHTBL_INDEX(SlotId,SlotEntry);

   SchTbl->Table.Entry[i].Enabled = EnableFlag;


} /* End SCHTBL_ConfigureTableEntry() */

/* end of file */
