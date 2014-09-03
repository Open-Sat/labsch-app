/*
** $Id: $
** 
** Purpose: Define the Scheduler Lab's Command Manager class.
**
** Notes:
**   1. 
**
** References:
**   1. CFS Object-based Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/

#ifndef _cmd_mgr_
#define _cmd_mgr_

/*
** Includes
*/

#include "app_config.h"
#include "cfe.h"

/*
** Event Message IDs
*/

#define CMDMGR_REG_INVALID_FUNC_CODE_ERR_EID       (CMDMGR_BASE_EID + 0)
#define CMDMGR_DISPATCH_UNUSED_FUNC_CODE_ERR_EID   (CMDMGR_BASE_EID + 1)
#define CMDMGR_DISPATCH_INVALID_CHECKSUM_ERR_EID   (CMDMGR_BASE_EID + 2)
#define CMDMGR_DISPATCH_INVALID_LEN_ERR_EID        (CMDMGR_BASE_EID + 3)
#define CMDMGR_DISPATCH_INVALID_FUNC_CODE_ERR_EID  (CMDMGR_BASE_EID + 4)
#define CMDMGR_TOTAL_EID  9

/*
** Type Definitions
*/

typedef boolean (*CMDMGR_CmdFuncPtr) (const CFE_SB_MsgPtr_t MsgPtr);


typedef struct
{

   uint16             UserDataLen;    /* User data length in bytes */
   CMDMGR_CmdFuncPtr  FuncPtr;

} CMDMGR_Cmd;

typedef struct
{

   uint16      ValidCmdCnt;       /* Number of valid messages received since init or reset */
   uint16      InvalidCmdCnt;     /* Number of invalid messages received since init or reset */
   CMDMGR_Cmd  Cmd[CMDMGR_CMD_FUNC_TOTAL];

} CMDMGR_Class;

/*
** Exported Functions
*/

/******************************************************************************
** Function: CMDMGR_Constructor
**
** Notes:
**    1.
**
*/
void CMDMGR_Constructor(CMDMGR_Class* CmdMgrPtr);


/******************************************************************************
** Function: CMDMGR_RegisterFunc
**
** Notes:
**    1.
**
*/
void CMDMGR_RegisterFunc(uint16 FuncCode, CMDMGR_CmdFuncPtr FuncPtr, uint16 UserDataLen);


/******************************************************************************
** Function: CMDMGR_ResetStatus
**
** Notes:
**    1.
**
*/
void CMDMGR_ResetStatus(void);


/******************************************************************************
** Function: CMDMGR_DispatchFunc
**
** Notes:
**    1.
**
*/
boolean CMDMGR_DispatchFunc (const CFE_SB_MsgPtr_t  MsgPtr);


#endif /* _cmd_mgr_ */
