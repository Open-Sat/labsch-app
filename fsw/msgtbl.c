/* 
** File:
**   $Id: $
**
** Purpose: Manage the table (Message Table) that defines the messages to be sent
**          by the scheduler.
**
** Notes
**   1. 
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

#include "msgtbl.h"

/*
** Global File Data
*/

static MSGTBL_Class*  MsgTbl = NULL;


/******************************************************************************
** Function:
**
** Notes:
**    1. 
**
*/
void MSGTBL_Constructor(MSGTBL_Class*  MsgTblPtr)
{
 
   MsgTbl = MsgTblPtr;

   CFE_PSP_MemSet((void *)(&MsgTblPtr->Table), 0, sizeof(MSGTBL_Table));

} /* End MSGTBL_Constructor() */


/*******************************************************************
**
** MSGTBL_GetTblPtr
**
*/
const MSGTBL_Table* MSGTBL_GetTblPtr()
{

   return &(MsgTbl->Table);

} /* End MSGTBL_GetTblPtr() */


/******************************************************************************
** Function:  MSGTBL_ResetStatus
**
*/
void MSGTBL_ResetStatus()
{

   /* Nothing to do */

} /* End MSGTBL_ResetStatus() */

/******************************************************************************
** Function: MSGTBL_SendMsg
**
*/
boolean MSGTBL_SendMsg(uint16  EntryId)
{

   boolean RetStatus = FALSE;
   int32   Status;
   uint16 *MsgBuffPtr;


   if (EntryId < MSGTBL_MAX_ENTRY_ID)
   {

      MsgBuffPtr = MsgTbl->Table.Entry[EntryId].Buffer;
      Status = CFE_SB_SendMsg((CFE_SB_Msg_t *)MsgBuffPtr);

      /* CFE_EVS_SendEvent(999, CFE_EVS_INFORMATION,"MSGTBL Send: EntryId = %d, Buffer[0] = 0x%04x, SB_SendMsg Status = 0x%08X", EntryId, MsgTbl->Table.Entry[EntryId].Buffer[0], Status); */

      if (Status == CFE_SUCCESS)
      {
         RetStatus = TRUE;
      }
      else
      {
         CFE_EVS_SendEvent(MSGTBL_SB_SEND_ERR_EID, CFE_EVS_ERROR,
                           "MSGTBL Send Error: EntryId = %d, MsgId = 0x%04x, SB_SendMsg Status = 0x%08X",
                           EntryId, MsgTbl->Table.Entry[EntryId].Buffer[0], Status);
      }

   } /* End if valid EntryId */

   return RetStatus;

} /* End MSGTBL_SendMSg() */


/*******************************************************************
**
** MSGTBL_LoadTable
**
*/
void MSGTBL_LoadTable(MSGTBL_Table* NewTable)
{

   CFE_PSP_MemCpy(&(MsgTbl->Table), NewTable, sizeof(MSGTBL_Table));

} /* End MSGTBL_LoadTable() */

/*******************************************************************
**
** MSGTBL_LoadTableEntry
**
*/
void MSGTBL_LoadTableEntry(uint16 EntryId, MSGTBL_Entry* NewEntry)
{

   CFE_PSP_MemCpy(&(MsgTbl->Table.Entry[EntryId]),NewEntry,sizeof(MSGTBL_Entry));

} /* End MSGTBL_LoadTableEntry() */

/* end of file */
