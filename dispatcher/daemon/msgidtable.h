/**
 *  @file msgidtable.h
 *
 *  @brief Message ID table header file
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef DISPATCHER_MSG_ID_TABLE_H
#define DISPATCHER_MSG_ID_TABLE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

typedef struct
{
    u8 dmit_u8Shift;
    u8 dmit_u8Reserved;
    u16 dmit_u16TableSize;
    u16 dmit_u16Reserved[2];

    jf_hlisthead_t * dmit_pjhMsgId;
} dispatcher_msg_id_table_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 addDispatcherMsgToTable(dispatcher_msg_id_table_t * pTable, u32 u32MsgId, olchar_t * pstrDesc);

u32 findDispatcherMsgInTable(
    dispatcher_msg_id_table_t * pTable, u32 u32MsgId, dispatcher_msg_t ** ppMsg);

#endif /*DISPATCHER_MSG_ID_TABLE_H*/

/*------------------------------------------------------------------------------------------------*/


