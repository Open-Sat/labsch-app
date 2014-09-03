/*
 * File:
 *   $Id: $
 *
 * Purpose: This file contains a unit test cases for msg-tbl.c
 *
 * $Date: $
 * $Revision: $
 * $Log: $
 */

/*
 * Includes
 */

#include "common_types.h"
#include "utassert.h"
#include "uttest.h"
#include "msgtbl.h"

MSGTBL_Class  MsgTbl;

MSGTBL_Table  TstMsgTbl;

/*
** Function Definitions
*/

/*******************************************************************
** Function: LoadTestTable
**
** TODO - Move to unit test now that load table function available.
**
*/
static void LoadTestTable(void)
{

   int i;
   MSGTBL_Entry *Entry;


   for (i=0; i < MSGTBL_MAX_ENTRY_ID; i++)
   {
      Entry = &(TstMsgTbl.Entry[i]);
      CFE_PSP_MemSet(Entry, 0, sizeof(MSGTBL_Entry));
      Entry->Buffer[0] = MSGTBL_UNUSED_MSG_ID;

   } /* End Entry Loop */

   TstMsgTbl.Entry[0].Buffer[0] =  1;
   TstMsgTbl.Entry[0].Buffer[1] =  2;
   TstMsgTbl.Entry[0].Buffer[MSGTBL_MAX_MSG_WORDS-2] = MSGTBL_MAX_MSG_WORDS-1;
   TstMsgTbl.Entry[0].Buffer[MSGTBL_MAX_MSG_WORDS-1] = MSGTBL_MAX_MSG_WORDS;

   TstMsgTbl.Entry[1].Buffer[0] =  11;
   TstMsgTbl.Entry[1].Buffer[1] =  12;
   TstMsgTbl.Entry[1].Buffer[MSGTBL_MAX_MSG_WORDS-2] = 10 + MSGTBL_MAX_MSG_WORDS-1;
   TstMsgTbl.Entry[1].Buffer[MSGTBL_MAX_MSG_WORDS-1] = 10 + MSGTBL_MAX_MSG_WORDS;

   /* Entry #7: TO HK */
   TstMsgTbl.Entry[MSGTBL_MAX_ENTRY_ID/2].Buffer[0] = MSGTBL_MAX_ENTRY_ID/2 + 1;
   TstMsgTbl.Entry[MSGTBL_MAX_ENTRY_ID/2].Buffer[1] = MSGTBL_MAX_ENTRY_ID/2 + 2;
   TstMsgTbl.Entry[MSGTBL_MAX_ENTRY_ID/2].Buffer[MSGTBL_MAX_MSG_WORDS-2] = MSGTBL_MAX_ENTRY_ID/2 + MSGTBL_MAX_MSG_WORDS-1;
   TstMsgTbl.Entry[MSGTBL_MAX_ENTRY_ID/2].Buffer[MSGTBL_MAX_MSG_WORDS-1] = MSGTBL_MAX_ENTRY_ID/2 + MSGTBL_MAX_MSG_WORDS;

   /* Entry #7: TO HK */
   TstMsgTbl.Entry[MSGTBL_MAX_ENTRY_ID-1].Buffer[0] = MSGTBL_MAX_ENTRY_ID-1 + 1;
   TstMsgTbl.Entry[MSGTBL_MAX_ENTRY_ID-1].Buffer[1] = MSGTBL_MAX_ENTRY_ID-1 + 2;
   TstMsgTbl.Entry[MSGTBL_MAX_ENTRY_ID-1].Buffer[MSGTBL_MAX_MSG_WORDS-2] = MSGTBL_MAX_ENTRY_ID-1 + MSGTBL_MAX_MSG_WORDS-1;
   TstMsgTbl.Entry[MSGTBL_MAX_ENTRY_ID-1].Buffer[MSGTBL_MAX_MSG_WORDS-1] = MSGTBL_MAX_ENTRY_ID-1 + MSGTBL_MAX_MSG_WORDS;

} /* End LoadTestTable() */

/* MSGTBL_Test01  - Constructor, ResetStatus, GetTblPtr */
void MSGTBL_Test01(void)
{
   MSGTBL_Table* MsgTblPtr = NULL;

   /* Constructor called as part of setup */
   UtAssert_True(MsgTbl.Table.Entry[0].Buffer[0] == 0, "MsgTbl.Table.Entry[0] == 0");

   MsgTblPtr = MSGTBL_GetTblPtr();
   UtAssert_True(MsgTblPtr == &(MsgTbl.Table), "MsgTbl.Table.Entry[0] == 0");

   MSGTBL_Constructor (&MsgTbl);

} /* End MSGTBL_Test01() */


/* MSGTBL_Test02  - LoadTable, LoadTableEntry, SendMsg  */
void MSGTBL_Test02(void)
{


} /* End MSGTBL_Test02() */

/* MSGTBL_Test03  - Functional  */
void MSGTBL_Test03(void)
{

   /* TODO - Functional test as used by Scheduler Lab */

} /* End MSGTBL_Test03() */

/* Initialize test environment to default state for every test */
void MSGTBL_Setup(void)
{
   MSGTBL_Constructor (&MsgTbl);
   LoadTestTable();
}

void MSGTBL_TearDown(void)
{
    /* cleanup test environment */
}

void MSGTBL_AddTestCase(void)
{
    UtTest_Add(MSGTBL_Test01, MSGTBL_Setup, MSGTBL_TearDown, "MSGTBL_Test01 - Constructor, ResetStatus, GetTblPtr");
    UtTest_Add(MSGTBL_Test02, MSGTBL_Setup, MSGTBL_TearDown, "MSGTBL_Test02 - LoadTable, LoadTableEntry, SendMsg");
}
