/*
 * File:
 *   $Id: $ 
 *
 * Purpose: This file contains a unit test cases for sch-tbl.c
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
#include "schtbl.h"


SCHTBL_Class  SchTbl;

SCHTBL_Table  TstSchTbl;

/*
** Function Definitions
*/

/*******************************************************************
** Function: LoadTestTable
**
*/
static void LoadTestTable(void)
{

   int i;
   SCHTBL_Entry *Entry;

   for (i=0; i < SCHTBL_MAX_ENTRIES; i++)
   {
      Entry = &TstSchTbl.Entry[i];
      Entry->Enabled       = FALSE;
      Entry->Frequency     = 1;
      Entry->MsgTblEntryId = 0;
      Entry->Offset        = 0;

   } /* End Entry Loop */

   /* test basic cFE, CI, and TO messages. This has assumptions about the table slot definitions */
   TstSchTbl.Entry[0].Enabled = TRUE;
   TstSchTbl.Entry[0].MsgTblEntryId = 1;
   TstSchTbl.Entry[SCHTBL_MAX_ENTRIES/2].Enabled = TRUE;
   TstSchTbl.Entry[SCHTBL_MAX_ENTRIES/2].MsgTblEntryId = SCHTBL_MAX_ENTRIES/2;
   TstSchTbl.Entry[SCHTBL_MAX_ENTRIES-1].Enabled = TRUE;
   TstSchTbl.Entry[SCHTBL_MAX_ENTRIES-1].MsgTblEntryId = SCHTBL_MAX_ENTRIES;

} /* End LoadTestTable() */


/* SCHTBL_Test01  - Constructor */
void SCHTBL_Test01(void)
{

   SCHTBL_Constructor (&SchTbl);

} /* End SCHTBL_Test01() */

/* SCHTBL_Test02  - TBD Description */
void SCHTBL_Test02(void)
{
   return;

} /* End SCHTBL_Test01() */

/* Initialize test environment to default state for every test */
void SCHTBL_Setup(void)
{
   SCHTBL_Constructor (&SchTbl);
   LoadTestTable();

}

void SCHTBL_TearDown(void)
{
    /* cleanup test environment */
}

void SCHTBL_AddTestCase(void)
{
    UtTest_Add(SCHTBL_Test01, SCHTBL_Setup, SCHTBL_TearDown, "SCHTBL_Test01 - Constructor");
    UtTest_Add(SCHTBL_Test02, SCHTBL_Setup, SCHTBL_TearDown, "SCHTBL_Test02 - TBD Description");
}
