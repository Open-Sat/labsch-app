
/*
 * Filename: sch-lab_testrunner.c
 *
 * Purpose: Run the unit tests for the scheduler lab application.
 *
 */

/*
 * Includes
 */

#include "uttest.h"

/*
 *  Text parsers text cases
 */

void MSGTBL_AddTestCase(void);
void SCHTBL_AddTestCase(void);
void TBLPAR_AddTestCase(void);

/*
 * Function Definitions
 */

int main(void)
{   

   MSGTBL_AddTestCase();
   SCHTBL_AddTestCase();
   TBLMGR_AddTestCase();

    return(UtTest_Run());
}
