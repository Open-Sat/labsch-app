/*
** $Id: $
** 
** Purpose: Manage the table (Message Table) that defines the messages to be sent
**          by the scheduler.
**
** Notes:
**   1. 
**
** References:
**   1. Core Flight Executive Application Developers Guide.
**
** $Date: $
** $Revision: $
** $Log: $
**
*/

#ifndef _msgtbl_
#define _msgtbl_

/*
** Includes
*/

#include "app_config.h"
#include "common_types.h"
#include "cfe.h"



/*
** Event Message IDs
*/

#define MSGTBL_SB_SEND_ERR_EID  (MSGTBL_BASE_EID + 0)

#define MSGTBL_TOTAL_EID  1

/*
** Type Definitions
*/


/*
** Message definition table entry
*/
typedef struct
{
    uint16   Buffer[MSGTBL_MAX_MSG_WORDS];

} MSGTBL_Entry;

typedef struct
{
   MSGTBL_Entry Entry[MSGTBL_MAX_ENTRY_ID];

} MSGTBL_Table;

typedef struct {

   MSGTBL_Table  Table;

} MSGTBL_Class;

/*
** Exported Functions
*/

/******************************************************************************
** Function: MSGTBL_Constructor
**
** Construct a PKTMGR object. All table entries are cleared and the LoadTable()
** function should be used to load an initial table.
**
** Notes:
**   1. This must be called prior to any other function.
**
*/
void MSGTBL_Constructor(MSGTBL_Class *MsgTblPtr);


/******************************************************************************
** Function: MSGTBL_ResetStatus
**
** Reset counters and status flags to a known reset state.
**
** Notes:
**   1. See the MSGTBL_Class definition for the effected data.
**
*/
void MSGTBL_ResetStatus(void);


/******************************************************************************
** Function: MSGTBL_GetTblPtr
**
** Return a pointer to the message table.
**
*/
const MSGTBL_Table* MSGTBL_GetTblPtr(void);


/******************************************************************************
** Function: MSGTBL_SendMsg
**
** Send a SB message containing the message table entry at location EntryId.
**
** Notes:
**  1. Range checking is performed on EntryId and an event message is sent for
**     an invalid ID.
**
**
*/
boolean MSGTBL_SendMsg(uint16  EntryId);


/******************************************************************************
** Function: MSGTBL_LoadTable
**
** Load the entire message table
**
*/
void MSGTBL_LoadTable(MSGTBL_Table* NewTable);


/******************************************************************************
** Function: MSGTBL_LoadTableEntry
**
** Load a single message table entry
**
** Notes:
**   1. Range checking is not performed on the parameters.
**
*/
void MSGTBL_LoadTableEntry(uint16 EntryId, MSGTBL_Entry* NewEntry);


#endif /* _msgtbl_ */
