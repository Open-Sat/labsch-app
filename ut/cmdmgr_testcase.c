/*
 * File:
 *   $Id: $
 *
 * Purpose: This file contains a unit test cases for cmdmgr.c
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
#include "cmdmgr.h"

CMDMGR_Class  CmdMgr;

/*
** Function Definitions
*/


/* CMDMGR_Test01  - Constructor */
void CMDMGR_Test01(void)
{

   CMDMGR_Constructor (&CmdMgr);

} /* End CMDMGR_Test01() */


/* CMDMGR_Test02  - TBD  */
void CMDMGR_Test02(void)
{


} /* End CMDMGR_Test02() */

/* CMDMGR_Test03  - Functional  */
void CMDMGR_Test03(void)
{

   /* TODO - Functional test as used by Scheduler Lab */

} /* End CMDMGR_Test03() */

/* Initialize test environment to default state for every test */
void CMDMGR_Setup(void)
{
   CMDMGR_Constructor (&CmdMgr);

}

void CMDMGR_TearDown(void)
{
    /* cleanup test environment */
}

void CMDMGR_AddTestCase(void)
{
    UtTest_Add(CMDMGR_Test01, CMDMGR_Setup, CMDMGR_TearDown, "CMDMGR_Test01 - Constructor");
    UtTest_Add(CMDMGR_Test02, CMDMGR_Setup, CMDMGR_TearDown, "CMDMGR_Test02 - TBD");
}
