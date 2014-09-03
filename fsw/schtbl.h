/*
** $Id: $
** 
** Purpose: Define a scheduler table.
**
** Notes:
**   1. This design intentionally decouples the scheduler table from application
**      specific processing such as command callback functions and file processing.
**
** References:
**   1. Core Flight Executive Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/

#ifndef _schtbl_
#define _schtbl_

/*
** Includes
*/

#include "app_config.h"
#include "common_types.h"
#include "cfe.h"
#include "msgtbl.h"


/*
**  Macro Definitions
*/

#define SCHTBL_TIMER_NAME   "SCHTBL_MINOR_TIMER"
#define SCHTBL_SEM_NAME     "SCH_TIME_SEM"
#define SCHTBL_SEM_VALUE    0
#define SCHTBL_SEM_OPTIONS  0

#define SCHTBL_INDEX(slot,entry)  ((slot*SCHTBL_ENTRIES_PER_SLOT) + entry)

/*
** Synchronized to Mission Elapsed Time States
*/
#define SCHTBL_SYNCH_FALSE          0
#define SCHTBL_SYNCH_TO_MINOR       1
#define SCHTBL_SYNCH_MAJOR_PENDING  2
#define SCHTBL_SYNCH_TO_MAJOR       4

/*
** Major Frame Signal Source Identifiers
*/
#define SCHTBL_MF_SOURCE_NONE               0
#define SCHTBL_MF_SOURCE_CFE_TIME           1
#define SCHTBL_MF_SOURCE_MINOR_FRAME_TIMER  2

/*
** Minor Frame Slot Characteristics
*/

#define SCHTBL_NORMAL_SLOT_PERIOD (SCHTBL_MICROS_PER_MAJOR_FRAME / SCHTBL_TOTAL_SLOTS)
#define SCHTBL_SYNC_SLOT_PERIOD   (SCHTBL_NORMAL_SLOT_PERIOD + SCHTBL_SYNC_SLOT_DRIFT_WINDOW)
#define SCHTBL_SHORT_SLOT_PERIOD  (SCHTBL_NORMAL_SLOT_PERIOD - SCHTBL_SYNC_SLOT_DRIFT_WINDOW)

/*
** Major Frame Signal Source Identifiers
*/
#define SCHTBL_MF_SRC_NONE               0
#define SCHTBL_MF_SRC_CFE_TIME           1
#define SCHTBL_MF_SRC_MINOR_FRAME_TIMER  2

#define SCHTBL_TIME_SYNC_SLOT   (SCHTBL_TOTAL_SLOTS-1)  /* Slot processing algorithm assumes this is set to the last slot */

/*
** Maximum allowed error in minor frame timing. Worst accuracy determined to be
** the amount of drift that would cause the loss of a minor frame over one
** major frame
*/
#define SCHTBL_WORST_CLOCK_ACCURACY  (SCHTBL_NORMAL_SLOT_PERIOD/(SCHTBL_TOTAL_SLOTS-1))

/*
** Maximum number of minor frames to sample looking for subsecs = 0. Maximum
** number of minor frame timer expirations to allow before giving up on
** finding the slot whose MET subseconds field is zero.  It is assumed that
** three complete major frames should be sufficient for finding such a slot.
*/

#define SCHTBL_MAX_SYNC_ATTEMPTS   (SCHTBL_TOTAL_SLOTS * 3)


#define SCHTBL_MAX_ENTRIES (SCHTBL_TOTAL_SLOTS * SCHTBL_ENTRIES_PER_SLOT)

/*
** Event Message IDs
*/

#define SCHTBL_MINOR_FRAME_TIMER_CREATE_ERR_EID      (SCHTBL_BASE_EID + 0)
#define SCHTBL_MINOR_FRAME_TIMER_ACC_WARN_EID        (SCHTBL_BASE_EID + 1)
#define SCHTBL_SEM_CREATE_ERR_EID                    (SCHTBL_BASE_EID + 2)
#define SCHTBL_MAJOR_FRAME_SUB_ERR_EID               (SCHTBL_BASE_EID + 3)
#define SCHTBL_NOISY_MAJOR_FRAME_ERR_EID             (SCHTBL_BASE_EID + 4)

#define SCHTBL_SAME_SLOT_EID                         (SCHTBL_BASE_EID + 5)
#define SCHTBL_MULTI_SLOTS_EID                       (SCHTBL_BASE_EID + 6)
#define SCHTBL_SKIPPED_SLOTS_EID                     (SCHTBL_BASE_EID + 7)

#define SCHTBL_PACKET_SEND_ERR_EID                   (SCHTBL_BASE_EID + 8)

#define SCHTBL_TOTAL_EID  9


/*
** Type Definitions
*/

typedef struct {

   boolean Enabled;
   uint16  Frequency;
   uint16  Offset;
   uint16  MsgTblEntryId;

} SCHTBL_Entry;

typedef struct {

   SCHTBL_Entry Entry[SCHTBL_MAX_ENTRIES];

} SCHTBL_Table;

typedef struct {

   SCHTBL_Table Table;

   uint32  SlotsProcessedCount;           /* Total number of Schedule Slots (Minor Frames) Processed */
   uint16  SkippedSlotsCount;             /* Number of times that slot (minor frame) were skipped. NOT the number of slots that were skipped  */
   uint16  MultipleSlotsCount;            /* Number of times that multiple slots were skipped. NOT the number of slots that were skipped  */
   uint16  SameSlotCount;                 /* Number of times scheduler woke up in the same slot as last time */

   uint32  ScheduleActivitySuccessCount;  /* Number of successfully performed activities */
   uint32  ScheduleActivityFailureCount;  /* Number of unsuccessful activities attempted */

   uint32  ValidMajorFrameCount;          /* Number of valid Major Frame tones received */
   uint32  MissedMajorFrameCount;         /* Number of missing Major Frame tones */
   uint32  UnexpectedMajorFrameCount;     /* Number of unexpected Major Frame tones */

   uint32  TablePassCount;                /* Number of times Schedule Table has been processed */
   uint32  ConsecutiveNoisyFrameCounter;  /* Number of consecutive noisy Major Frames */

   uint16  MinorFramesSinceTone;          /* Number of Minor Frames since last Major Frame tone */
   uint16  NextSlotNumber;                /* Next Minor Frame to be processed */
   uint16  LastSyncMETSlot;               /* MET Slot # where Time Sync last occurred */
   uint16  SyncAttemptsLeft;              /* Timeout counter used when syncing Major Frame to MET */

   boolean SendNoisyMajorFrameMsg;        /* Flag to send noisy major frame event msg once */
   boolean IgnoreMajorFrame;              /* Major Frame too noisy to trust */
   boolean UnexpectedMajorFrame;          /* Major Frame signal was unexpected */
   uint8   SyncToMET;                     /* Slots should be aligned with subseconds */
   uint8   MajorFrameSource;              /* Major Frame Signal source identifier */

   uint32  LastProcessCount;              /* Number of Slots Processed Last Cycle */

   uint32  TimerId;                       /* OSAL assigned timer ID for minor frame timer */
   uint32  TimeSemaphore;                 /* Semaphore used by time references to control main loop */
   uint32  ClockAccuracy;                 /* Accuracy of Minor Frame Timer */
   uint32  WorstCaseSlotsPerMinorFrame;   /* When syncing to MET, worst case # of slots that may need */

} SCHTBL_Class;

/*
** Exported Functions
*/


/******************************************************************************
** Function: SCHTBL_Constructor
**
** This method creates a new scheduler table instance.
**
** Notes:
**   1. This method must be called prior to all other methods. The SchTbl
**      instance variable only needs to be passed to the constructor
**      because a reference is stored by schtbl.c.
**
*/
void SCHTBL_Constructor(SCHTBL_Class* SchTbl);


/******************************************************************************
** Function: SCHTBL_Constructor
**
** Reset counters and status flags to a known reset state.  The behavior of the scheduler
** should not be impacted. The intent is to clear counters/flags for telemetry.
**
** Notes:
**   1. See the SCHTBL_Class definition for the effected data.
**
*/
void SCHTBL_ResetStatus(void);

/******************************************************************************
** Function: SCHTBL_GetTblPtr
**
** Return a pointer to the
**
*/
const SCHTBL_Table* SCHTBL_GetTblPtr(void);


/******************************************************************************
** Function: SCHTBL_ProcessTable
**
** Process the scheduler table performing activities.
**
** Notes:
**   1.
**
*/
boolean SCHTBL_ProcessTable(void);


/******************************************************************************
** Function: SCHTBL_StartTimers
**
** Notes:
**   1.
**
*/
int32 SCHTBL_StartTimers(void);


/******************************************************************************
** Function: SCHTBL_LoadTable
**
** Load the entire scheduler table
**
** Notes:
**   1.
**
*/
void SCHTBL_LoadTable(SCHTBL_Table* NewTable);

/******************************************************************************
** Function: LoadTableEntry
**
** Load a single scheduler table entry
**
** Notes:
**   1. Range checking is not performed on the parameters.
**
*/
void SCHTBL_LoadTableEntry(uint16 SlotId, uint16 SlotEntry, SCHTBL_Entry* NewEntry);


/******************************************************************************
** Function: SCHTBL_ConfigureTableEntry
**
** Configure (enable/disable) a single entry
**
** Notes:
**   1. Range checking is not performed on the parameters.
**
*/
void SCHTBL_ConfigureTableEntry(uint16 SlotId, uint16 SlotEntry, boolean EnableFlag);


#endif /* _schtbl_ */
