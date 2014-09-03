/*
** $Id: $
** 
** Purpose: Define configurations for the Scheduler Lab application
**
** Notes:
**   1. These macros can only be build with the application and can't
**      have a platform scope because the same file name is used for
**      all applications following the object-based application design.
**
** References:
**   1. CFS Object-based Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/

#ifndef _app_config_
#define _app_config_

/*
** Includes
*/

#include "labsch_mission_cfg.h"
#include "labsch_platform_cfg.h"

/******************************************************************************
** Scheduler Application Macros
*/

#define  LABSCH_MAJOR_VERSION      1
#define  LABSCH_MINOR_VERSION      0
#define  LABSCH_REVISION           0
#define  LABSCH_MISSION_REV        0

/******************************************************************************
** Command Macros
*/

#define LABSCH_CMD_RESET_FC            0
#define LABSCH_CMD_NOOP_FC             1

#define LABSCH_CMD_MSG_TBL_LOAD_FC     2
#define LABSCH_CMD_MSG_TBL_DUMP_FC     3

#define LABSCH_CMD_SCH_TBL_CONFIG_FC   4
#define LABSCH_CMD_SCH_TBL_LOAD_FC     5
#define LABSCH_CMD_SCH_TBL_DUMP_FC     6

#define LABSCH_CMD_TOTAL_FC            7

#define CMDMGR_CMD_FUNC_TOTAL   10
#define CMDMGR_PIPE_DEPTH       10
#define CMDMGR_PIPE_NAME        "LABSCH_CMD_PIPE"
#define CMDMGR_CMD_MSG_TOTAL    2

/******************************************************************************
** Event Macros
*/

#define LABSCH_BASE_EID   0  /* Used by labsch.h */
#define CMDMGR_BASE_EID  10  /* Used by cmdmgr.h */
#define SCHTBL_BASE_EID  20  /* Used by schtbl.h */
#define MSGTBL_BASE_EID  30  /* Used by msgtbl.h */
#define TBLMGR_BASE_EID  40  /* Used by tblpar.h */


/******************************************************************************
** tblmgr.h Configurations
*/

#define TBLMGR_DEF_MSG_TBL_DUMP_FILE LABSCH_DEF_MSGTBL_FILE_NAME
#define TBLMGR_DEF_SCH_TBL_DUMP_FILE LABSCH_DEF_SCHTBL_FILE_NAME

#endif /* _app_config_ */
