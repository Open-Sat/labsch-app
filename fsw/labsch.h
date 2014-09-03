/*
** $Id: $
** 
** Purpose: Define the lab scheduler application.
**
** Notes:
**   1. Most functions are defined as global to assist in unit testing.
**
** References:
**   1. Core Flight Executive Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/
#ifndef _labsch_
#define _labsch_

/*
** Includes
*/

#include "app_config.h"
#include "cmdmgr.h"
#include "schtbl.h"
#include "msgtbl.h"
#include "tblmgr.h"

/*
** Macro Definitions
*/

#define LABSCH_INITSTATS_INF_EID       (LABSCH_BASE_EID + 0)
#define LABSCH_APP_EXIT_EID            (LABSCH_BASE_EID + 1)
#define LABSCH_APP_INVALID_MID_ERR_EID (LABSCH_BASE_EID + 2)
#define LABSCH_TOTAL_EID  1


/*
** Type Definitions
*/

typedef struct
{

   CMDMGR_Class CmdMgr;
   SCHTBL_Class SchTbl;
   MSGTBL_Class MsgTbl;
   TBLMGR_Class TblMgr;

   CFE_SB_PipeId_t CmdPipe;

} LABSCH_Class;

typedef struct {

   uint8    Header[CFE_SB_TLM_HDR_SIZE];

   /*
   ** CMDMGR Data
   */
   uint16   ValidCmdCnt;
   uint16   InvalidCmdCnt;

   /*
   ** TBLMGR Data
   */

   boolean  MsgTblLoadActive;
   boolean  MsgTblLastLoadValid;
   uint16   MsgTblAttrErrCnt;
   boolean  SchTblLoadActive;
   boolean  SchTblLastLoadValid;
   uint16   SchTblAttrErrCnt;

   /*
   ** SCHTBL Data
   ** - At a minimum every sch-tbl variable effected by a reset must be included
   ** - These have been rearranged to align data words
   */

   uint32  SlotsProcessedCount;
   uint32  ScheduleActivitySuccessCount;
   uint32  ScheduleActivityFailureCount;
   uint32  ValidMajorFrameCount;
   uint32  MissedMajorFrameCount;
   uint32  UnexpectedMajorFrameCount;
   uint32  TablePassCount;
   uint32  ConsecutiveNoisyFrameCounter;
   uint16  SkippedSlotsCount;
   uint16  MultipleSlotsCount;
   uint16  SameSlotCount;
   uint16  SyncAttemptsLeft;
   uint16  LastSyncMETSlot;
   boolean IgnoreMajorFrame;
   boolean UnexpectedMajorFrame;

} OS_PACK LABSCH_HkPkt;

#define LABSCH_HK_TLM_LEN sizeof (LABSCH_HkPkt)

typedef struct {

   uint8    Header[CFE_SB_TLM_HDR_SIZE];

   /*
   ** SCHTBL Data
   */
   uint32  LastProcessCount;
   uint32  TimerId;
   uint32  TimeSemaphore;
   uint32  ClockAccuracy;
   uint32  WorstCaseSlotsPerMinorFrame;
   boolean IgnoreMajorFrame;
   uint8   SyncToMET;
   uint8   MajorFrameSource;

} OS_PACK LABSCH_DiagPkt;

#define LABSCH_DIAG_TLM_LEN sizeof (LABSCH_DiagPkt)


/*
** Exported Data
*/

extern LABSCH_Class  LabSch;


/*
** Exported Functions
*/

/******************************************************************************
** Function: LABSCH_Main
**
*/
void LABSCH_Main(void);


/******************************************************************************
** Function: LABSCH_NoOpCmd
**
*/
boolean LABSCH_NoOpCmd(const CFE_SB_MsgPtr_t MsgPtr);


/******************************************************************************
** Function: LABSCH_ResetAppCmd
**
*/
boolean LABSCH_ResetAppCmd(const CFE_SB_MsgPtr_t MsgPtr);


#endif /* _labsch_ */

