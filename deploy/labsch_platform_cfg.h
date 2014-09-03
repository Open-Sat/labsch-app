/*
** $Id: $
** 
** Purpose: Define platform configurations for the Scheduler Lab application
**
** Notes:
**   1. This file should only include macros that must be defined with a platform scope
**
** References:
**   1. CFS Object-based Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/
#ifndef _labsch_platform_cfg_
#define _labsch_platform_cfg_

/******************************************************************************
** Scheduler Application Macros
*/

#define  LABSCH_DEF_MSGTBL_FILE_NAME "/cf/labsch_msgtbl.xml"
#define  LABSCH_DEF_SCHTBL_FILE_NAME "/cf/labsch_schtbl.xml"

#define  LABSCH_CMD_MID         0x1882
#define  LABSCH_SEND_HK_MID     0x1883
#define  LABSCH_TLM_HK_MID      0x0882
         

/******************************************************************************
** Scheduler Table Configurations
*/


/*
** Number of minor frame slots within each Major Frame. Must be 2 or more and less than 65536.
*/
#define SCHTBL_TOTAL_SLOTS      5

/*
** Maximum number of Activities per Minor Frame. Must be greater than zero.
*/
#define SCHTBL_ENTRIES_PER_SLOT  10


/*
** Number of Minor Frames that will be processed in "Catch Up"
** mode before giving up and skipping ahead.
*/
#define SCHTBL_MAX_LAG_COUNT  (SCHTBL_TOTAL_SLOTS / 2)

/*
** Maximum number of slots scheduler will process when trying to
** "Catch Up" to the correct slot for the current time. Must be greater than zero.
*/
#define SCHTBL_MAX_SLOTS_PER_WAKEUP      5

/*
** Number of microseconds in a Major Frame. Used as a "wake-up" period. Must be greater than zero.
*/
#define SCHTBL_MICROS_PER_MAJOR_FRAME    1000000


/*
** Defines the additional time allowed in the Synchronization Slot to allow
** the Major Frame Sync signal to be received and re-synchronize processing.
** Must be less than the normal slot period.
*/
#define SCHTBL_SYNC_SLOT_DRIFT_WINDOW   5000


/*
** Defines the timeout for the #CFE_ES_WaitForStartupSync call that scheduler
** uses to wait for all of the Applications specified in the startup script to
** finish initialization. The scheduler will wait this amount of time before
** assuming all startup script applications have been started and will then
** begin nominal schedule processing.
*/
#define SCHTBL_STARTUP_SYNC_TIMEOUT   10000


/*
** Defines the time allowed for the first Major Frame sync signal to arrive
** before assuming it is not going to occur and switching to a free-wheeling
** mode. Must be greater than or equal to the Major Frame Period
*/
#define SCHTBL_STARTUP_PERIOD   (5*SCHTBL_MICROS_PER_MAJOR_FRAME)


/*
** Defines the number of consecutive "Noisy" Major Frame Signals (i.e. -
** signals that occur outside the expected window of their occurrence)
** until the Major Frame signal is automatically ignored and the Minor
** Frame Timer is used instead. This value should never be set to less
** than two because a single "noisy" Major Frame signal is likely when
** turning on or switching the 1 Hz signal on the spacecraft.
*/
#define SCHTBL_MAX_NOISY_MF   2

/******************************************************************************
** Message Table Configurations
*/

/*
** Maximum Number of Message Definitions in Message Definition Table. Must be greater than zero.
*/

#define MSGTBL_MAX_ENTRY_ID      230
#define MSGTBL_UNUSED_MSG_ID    (CFE_SB_HIGHEST_VALID_MSGID+1)

/*
** Max message length in words.  Must be at least large enough to hold the smallest possible message header
** (see #CFE_SB_TLM_HDR_SIZE and #CFE_SB_CMD_HDR_SIZE)
*/
#define MSGTBL_MAX_MSG_WORDS      32


#endif /* _labsch_platform_cfg_ */
