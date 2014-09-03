/* 
** File:
**   $Id: $
**
** Purpose: Manage command dispatching for the application.
**
** Notes
**   1. 
**
** References:
**   1. Core Flight Executive Application Developers Guide.
**
**
** $Date: $
** $Revision: $
** $Log: $
*/

/*
** Include Files:
*/

#include "cmdmgr.h"


/*
** File Function Prototypes
*/

static boolean UnusedFuncCode(const CFE_SB_MsgPtr_t MsgPtr);

/*
** Global Data
*/

CMDMGR_Class* CmdMgr;

/******************************************************************************
** Function: CMDMGR_Constructor
**
** Notes:
**    1. 
**
*/
void CMDMGR_Constructor(CMDMGR_Class* CmdMgrPtr)
{

   int i;
   CmdMgr = CmdMgrPtr;

   CFE_PSP_MemSet(CmdMgrPtr, 0, sizeof(CMDMGR_Class));
   for (i=0; i < CMDMGR_CMD_FUNC_TOTAL; i++)
   {
      CmdMgr->Cmd[i].FuncPtr = UnusedFuncCode;
   }

} /* End CMDMGR_Constructor() */

/******************************************************************************
** Function: CMDMGR_RegisterFunc
**
** Notes:
**    1.
**
*/
void CMDMGR_RegisterFunc(uint16 FuncCode, CMDMGR_CmdFuncPtr FuncPtr, uint16 UserDataLen)
{

   if (FuncCode < CMDMGR_CMD_FUNC_TOTAL)
   {

      CmdMgr->Cmd[FuncCode].FuncPtr = FuncPtr;
      CmdMgr->Cmd[FuncCode].UserDataLen = UserDataLen;

   }
   else
   {
      CFE_EVS_SendEvent (CMDMGR_REG_INVALID_FUNC_CODE_ERR_EID, CFE_EVS_ERROR,
      "Attempt to register function code %d which is greater than max %d",
      FuncCode,(CMDMGR_CMD_FUNC_TOTAL-1));
   }

} /* End CMDMGR_RegisterFunc() */

/******************************************************************************
** Function: CMDMGR_ResetStatus
**
** Notes:
**    1.
**
*/
void CMDMGR_ResetStatus(void)
{

   CmdMgr->ValidCmdCnt = 0;
   CmdMgr->InvalidCmdCnt = 0;

} /* End CMDMGR_ResetStatus() */

/******************************************************************************
** Function: CMDMGR_DispatchFunc
**
** Notes:
**    1.
**
*/
boolean CMDMGR_DispatchFunc (const CFE_SB_MsgPtr_t  MsgPtr)
{

   boolean  ValidCmd = FALSE;

   uint16 UserDataLen = CFE_SB_GetUserDataLength(MsgPtr);
   uint16 FuncCode    = CFE_SB_GetCmdCode(MsgPtr);
   uint32 Checksum    = CFE_SB_GetChecksum(MsgPtr);

   if (FuncCode < CMDMGR_CMD_FUNC_TOTAL)
   {

      if (UserDataLen == CmdMgr->Cmd[FuncCode].UserDataLen)
      {

         if (CFE_SB_ValidateChecksum(MsgPtr) == TRUE)
         {

            ValidCmd = (CmdMgr->Cmd[FuncCode].FuncPtr)(MsgPtr);

         } /* End if valid checksum */
         else
         {

            CFE_EVS_SendEvent (CMDMGR_DISPATCH_INVALID_CHECKSUM_ERR_EID, CFE_EVS_ERROR,
                               "Invalid command checksum %d", Checksum);
         }

      } /* End if valid length */
      else
      {

         CFE_EVS_SendEvent (CMDMGR_DISPATCH_INVALID_LEN_ERR_EID, CFE_EVS_ERROR,
                            "Invalid command user data length %d, expected %d",
                            UserDataLen, CmdMgr->Cmd[FuncCode].UserDataLen);

      }

   } /* End if valid function code */
   else
   {
      CFE_EVS_SendEvent (CMDMGR_DISPATCH_INVALID_FUNC_CODE_ERR_EID, CFE_EVS_ERROR,
                         "Invalid command function code %d is greater than max %d",
                         FuncCode, (CMDMGR_CMD_FUNC_TOTAL-1));

   } /* End if invalid function code */

   ValidCmd ? CmdMgr->ValidCmdCnt++ : CmdMgr->InvalidCmdCnt++;
   
   return ValidCmd;

} /* End CMDMGR_DispatchFunc() */


/******************************************************************************
** Function: UnusedFuncCode
**
** Notes:
**    1.
**
*/
static boolean UnusedFuncCode(const CFE_SB_MsgPtr_t MsgPtr)
{

   CFE_EVS_SendEvent (CMDMGR_DISPATCH_UNUSED_FUNC_CODE_ERR_EID, CFE_EVS_ERROR,
                      "Unused command function code %d received",CFE_SB_GetCmdCode(MsgPtr));

   return FALSE;

} /* End UnusedFuncCode() */

/* end of file */
