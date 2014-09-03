/* 
** File:
**   $Id: $
**
** Purpose: Implement Lab Scheduler application.
**
** Notes:
**   1. This is non-flight code so an attempt has been made to balance keeping
**      it simple while making it robust. Limiting the number of configuration
**      parameters and integration items (message IDs, perf IDs, etc) was
**      also taken into consideration.
**   2. Event message filters are not used since this is for test environments.
**      This may be reconsidered if event flooding ever becomes a problem.
**   3. Performance traces are not included.
**   4. Most functions are global to assist in unit testing
**   4. Functions I removed from original that need to be thought thru
**        SCH_ValidateMessageData()
**        SCH_ValidateScheduleData()
**        SCH_ProcessCommands()
**        SCH_TblInit()
**        InitEventFilters()
**
** References:
**   1. Core Flight Executive Application Developers Guide.
**   2. GN&C FSW Framework Programmer's Guide
**
** $Date: $
** $Revision: $
** $Log: $
**
*/

/*
** Includes
*/

#include "labsch.h"


/*
** Local Function Prototypes
*/

static int32 InitApp(void);
static void ProcessCmdPkt(CFE_SB_MsgPtr_t CmdMsgPtr);

/*
** Global Data
*/

LABSCH_Class  LabSch;

LABSCH_HkPkt  LabSchHk;

/******************************************************************************
** Function: LABSCH_Main
**
*/
void LABSCH_Main(void)
{
    int32  Status    = CFE_SEVERITY_ERROR;
    uint32 RunStatus = CFE_ES_APP_ERROR;
    CFE_SB_Msg_t* CmdMsgPtr;

    /*
    ** Register application
    */
    Status = CFE_ES_RegisterApp();

    CFE_EVS_Register(NULL,0,0);

    /*
    ** Perform application specific initialization
    */
    if (Status == CFE_SUCCESS)
    {
        Status = InitApp();
    }

    /* If no errors were detected during initialization, then wait for everyone to start */
    if (Status == CFE_SUCCESS)
    {

       /*
        * Flight version includes a call to CFE_ES_WaitForStartupSync(). Since
        * this will be used in a dynamic test environment the idea is let the
        * default scheduler start and add applications as needed from the user
        * console as opposed to trying to synchronize everything in the embedded
        * system.
        */

       Status = SCHTBL_StartTimers();

    } /* End if App initialized successfully */

    if (Status == CFE_SUCCESS) RunStatus = CFE_ES_APP_RUN;

    /*
    ** Main process loop
    */
    while (CFE_ES_RunLoop(&RunStatus))
    {

       /* TODO - Add command processing */

       if (!SCHTBL_ProcessTable())
       {
            RunStatus = CFE_ES_APP_ERROR;
       }

       Status = CFE_SB_RcvMsg(&CmdMsgPtr, LabSch.CmdPipe, CFE_SB_POLL);
        
       if (Status == CFE_SUCCESS)
       {
          ProcessCmdPkt(CmdMsgPtr);
       }

    } /* End CFE_ES_RunLoop */


    /* Write to system log in case events not working */

    CFE_ES_WriteToSysLog("LABSCH App terminating, err = 0x%08X\n", Status);

    CFE_EVS_SendEvent(LABSCH_APP_EXIT_EID, CFE_EVS_CRITICAL, "LABSCH App: terminating, err = 0x%08X", Status);

    CFE_ES_ExitApp(RunStatus);  /* Let cFE kill the task (and any child tasks) */

} /* End of LABSCH_Main() */


/******************************************************************************
** Function: LABSCH_NoOpCmd
**
*/

boolean LABSCH_NoOpCmd(const CFE_SB_MsgPtr_t MsgPtr)
{

   CFE_EVS_SendEvent (666,
                      CFE_EVS_INFORMATION,
                      "No operation command received for Scheduler Lab version %d.%d",
                      LABSCH_MAJOR_VERSION,LABSCH_MINOR_VERSION);

   return TRUE;


} /* End LABSCH_NoOpCmd() */


/******************************************************************************
** Function: LABSCH_ResetAppCmd
**
*/

boolean LABSCH_ResetAppCmd(const CFE_SB_MsgPtr_t MsgPtr)
{

   MSGTBL_ResetStatus();
   SCHTBL_ResetStatus();

   TBLMGR_ResetStatus();
   CMDMGR_ResetStatus();

   return TRUE;

} /* End LABSCH_ResetAppCmd() */


/******************************************************************************
** Function: LABSCH_SendHousekeepingPkt
**
*/
void LABSCH_SendHousekeepingPkt(void)
{

   /*
   ** LABSCH Data
   */

   LabSchHk.ValidCmdCnt   = LabSch.CmdMgr.ValidCmdCnt;
   LabSchHk.InvalidCmdCnt = LabSch.CmdMgr.InvalidCmdCnt;

   /*
   ** TBLMGR Data
   */

   LabSchHk.MsgTblLoadActive     = LabSch.TblMgr.MsgTbl.LoadActive;
   LabSchHk.MsgTblLastLoadValid  = LabSch.TblMgr.MsgTbl.LastLoadValid;
   LabSchHk.MsgTblAttrErrCnt     = LabSch.TblMgr.MsgTbl.AttrErrCnt;
   LabSchHk.SchTblLoadActive     = LabSch.TblMgr.SchTbl.LoadActive;
   LabSchHk.SchTblLastLoadValid  = LabSch.TblMgr.SchTbl.LastLoadValid;
   LabSchHk.SchTblAttrErrCnt     = LabSch.TblMgr.SchTbl.AttrErrCnt;

   /*
   ** SCH-TBL Data
   ** - At a minimum every sch-tbl variable effected by a reset must be included
   ** - These have been rearranged to align data words
   */

   LabSchHk.SlotsProcessedCount          = LabSch.SchTbl.SlotsProcessedCount;
   LabSchHk.ScheduleActivitySuccessCount = LabSch.SchTbl.ScheduleActivitySuccessCount;
   LabSchHk.ScheduleActivityFailureCount = LabSch.SchTbl.ScheduleActivityFailureCount;
   LabSchHk.ValidMajorFrameCount         = LabSch.SchTbl.ValidMajorFrameCount;
   LabSchHk.MissedMajorFrameCount        = LabSch.SchTbl.MissedMajorFrameCount;
   LabSchHk.UnexpectedMajorFrameCount    = LabSch.SchTbl.UnexpectedMajorFrameCount;
   LabSchHk.TablePassCount               = LabSch.SchTbl.TablePassCount;
   LabSchHk.ConsecutiveNoisyFrameCounter = LabSch.SchTbl.ConsecutiveNoisyFrameCounter;
   LabSchHk.SkippedSlotsCount            = LabSch.SchTbl.SkippedSlotsCount;
   LabSchHk.MultipleSlotsCount           = LabSch.SchTbl.MultipleSlotsCount;
   LabSchHk.SameSlotCount                = LabSch.SchTbl.SameSlotCount;
   LabSchHk.SyncAttemptsLeft             = LabSch.SchTbl.SyncAttemptsLeft;
   LabSchHk.LastSyncMETSlot              = LabSch.SchTbl.LastSyncMETSlot;
   LabSchHk.IgnoreMajorFrame             = LabSch.SchTbl.IgnoreMajorFrame;
   LabSchHk.UnexpectedMajorFrame         = LabSch.SchTbl.UnexpectedMajorFrame;

   CFE_SB_TimeStampMsg((CFE_SB_Msg_t *) &LabSchHk);
   CFE_SB_SendMsg((CFE_SB_Msg_t *) &LabSchHk);

} /* End LABSCH_SendHousekeepingPkt() */

/******************************************************************************
** Function: InitApp
**
*/
static int32 InitApp(void)
{
    int32 Status = CFE_SUCCESS;

    /*
    ** Initialize 'entity' objects
    */

    MSGTBL_Constructor(&LabSch.MsgTbl);
    SCHTBL_Constructor(&LabSch.SchTbl);


    /*
    ** Initialize application managers
    */

    TBLMGR_Constructor(&LabSch.TblMgr, LABSCH_DEF_MSGTBL_FILE_NAME, LABSCH_DEF_SCHTBL_FILE_NAME);

    CFE_SB_CreatePipe(&LabSch.CmdPipe, CMDMGR_PIPE_DEPTH, CMDMGR_PIPE_NAME);
    CFE_SB_Subscribe(LABSCH_CMD_MID, LabSch.CmdPipe);
    CFE_SB_Subscribe(LABSCH_SEND_HK_MID, LabSch.CmdPipe);

    CMDMGR_Constructor(&LabSch.CmdMgr);
    CMDMGR_RegisterFunc(LABSCH_CMD_RESET_FC, LABSCH_ResetAppCmd, 0);
    CMDMGR_RegisterFunc(LABSCH_CMD_NOOP_FC,  LABSCH_NoOpCmd,     0);

    CMDMGR_RegisterFunc(LABSCH_CMD_MSG_TBL_LOAD_FC, TBLMGR_LoadMsgTable, TBLMGR_LOAD_TBL_CMD_DATA_LEN);
    CMDMGR_RegisterFunc(LABSCH_CMD_MSG_TBL_DUMP_FC, TBLMGR_DumpMsgTable, TBLMGR_DUMP_TBL_CMD_DATA_LEN);

    CMDMGR_RegisterFunc(LABSCH_CMD_SCH_TBL_CONFIG_FC, TBLMGR_ConfigSchEntryCmd, TBLMGR_CONFIG_SCH_CMD_DATA_LEN);
    CMDMGR_RegisterFunc(LABSCH_CMD_SCH_TBL_LOAD_FC,   TBLMGR_LoadSchTable,      TBLMGR_LOAD_TBL_CMD_DATA_LEN);
    CMDMGR_RegisterFunc(LABSCH_CMD_SCH_TBL_DUMP_FC,   TBLMGR_DumpSchTable,      TBLMGR_DUMP_TBL_CMD_DATA_LEN);

    CFE_SB_InitMsg(&LabSchHk, LABSCH_TLM_HK_MID, LABSCH_HK_TLM_LEN, TRUE);

    /*
    ** Application startup event message
    */
    Status = CFE_EVS_SendEvent(LABSCH_INITSTATS_INF_EID,
                               CFE_EVS_INFORMATION,
                               "Sch-LAB Initialized. Version %d.%d.%d.%d",
                               LABSCH_MAJOR_VERSION,
                               LABSCH_MINOR_VERSION,
                               LABSCH_REVISION,
                               LABSCH_MISSION_REV);

    return(Status);

} /* End of InitApp() */


/******************************************************************************
** Function: ProcessCmdPkt
**
*/
static void ProcessCmdPkt(CFE_SB_MsgPtr_t CmdMsgPtr)
{

   CFE_SB_MsgId_t  MsgId;

   MsgId = CFE_SB_GetMsgId(CmdMsgPtr);

   switch (MsgId)
   {
      case LABSCH_CMD_MID:
         CMDMGR_DispatchFunc(CmdMsgPtr);
         break;

      case LABSCH_SEND_HK_MID:
         LABSCH_SendHousekeepingPkt();
         break;

      default:
         CFE_EVS_SendEvent(LABSCH_APP_INVALID_MID_ERR_EID, CFE_EVS_ERROR,
                           "Received invalid command packet,MID = 0x%x",MsgId);

         break;

   } /* End Msgid switch */

   return;

} /* End ProcessCmdPkt() */


/* end of file */
