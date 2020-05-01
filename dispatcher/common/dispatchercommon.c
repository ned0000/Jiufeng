/**
 *  @file dispatchercommon.c
 *
 *  @brief The common routine in dispatcher.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_messaging.h"
#include "jf_jiukun.h"
#include "jf_process.h"
#include "jf_dir.h"

#include "dispatchercommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 createUdsDir(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrDir = DISPATCHER_UDS_DIR;

    u32Ret = jf_dir_create(pstrDir, JF_DIR_DEFAULT_CREATE_MODE);
    if (u32Ret == JF_ERR_DIR_ALREADY_EXIST)
        /*It's ok if the directory is already existing.*/
        u32Ret = JF_ERR_NO_ERROR;

    return u32Ret;
}

u32 destroyDispatcherMsg(dispatcher_msg_t ** ppMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logDebugMsg("destroy dispatcher msg");

    jf_jiukun_freeMemory((void **)ppMsg);

    return u32Ret;
}

u32 createDispatcherMsg(dispatcher_msg_t ** ppMsg, u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dispatcher_msg_t * pdm = NULL;
    u8 * pu8Start = NULL;

    JF_LOGGER_DEBUG("smsg: %d", sMsg);

    u32Ret = jf_jiukun_allocMemory((void **)&pdm, sizeof(*pdm) + sMsg);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pdm, sizeof(*pdm));
        pdm->dm_nRef = 1;
        pdm->dm_sMsg = sMsg;

        pu8Start = (u8 *)pdm + sizeof(*pdm);
        ol_memcpy(pu8Start, pu8Msg, sMsg);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppMsg = pdm;
    else if (pdm != NULL)
        destroyDispatcherMsg(&pdm);

    return u32Ret;
}

boolean_t isReservedDispatcherMsg(dispatcher_msg_t * pdm)
{
    return isReservedMessagingMsg(pdm->dm_u8Msg, pdm->dm_sMsg);
}

void incDispatcherMsgRef(dispatcher_msg_t * pdm)
{
    pdm->dm_nRef ++;
}

void decDispatcherMsgRef(dispatcher_msg_t * pdm)
{
    pdm->dm_nRef --;
}

olint_t getDispatcherMsgRef(dispatcher_msg_t * pdm)
{
    return pdm->dm_nRef;
}

u32 getDispatcherMsgSourceId(dispatcher_msg_t * pdm)
{
    return getMessagingMsgSourceId(pdm->dm_u8Msg, pdm->dm_sMsg);
}

u32 getDispatcherMsgDestinationId(dispatcher_msg_t * pdm)
{
    return getMessagingMsgDestinationId(pdm->dm_u8Msg, pdm->dm_sMsg);
}

u16 getDispatcherMsgId(dispatcher_msg_t * pdm)
{
    return getMessagingMsgId(pdm->dm_u8Msg, pdm->dm_sMsg);
}

u32 freeDispatcherMsg(dispatcher_msg_t ** ppMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    decDispatcherMsgRef(*ppMsg);

    if (getDispatcherMsgRef(*ppMsg) <= 0)
    {
        destroyDispatcherMsg(ppMsg);
    }

    return u32Ret;
}

u32 fnFreeDispatcherMsg(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Destroy the message, ignore the reference number.*/
    u32Ret = destroyDispatcherMsg((dispatcher_msg_t **)ppData);
    
    return u32Ret;
}

olsize_t getMessagingSize(u8 * pu8Msg)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    return sizeof(*pHeader) + pHeader->jmh_u32PayloadSize;
}

u32 initMessagingMsgHeader(u8 * pu8Msg, u16 u16MsgId, u8 u8MsgPrio, u32 u32PayloadSize)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    ol_bzero(pHeader, sizeof(*pHeader));
    pHeader->jmh_u16MsgId = u16MsgId;
    pHeader->jmh_u8MsgPrio = u8MsgPrio;
    pHeader->jmh_u32PayloadSize = u32PayloadSize;
    pHeader->jmh_u32SourceId = (u32)jf_process_getCurrentId();

    return JF_ERR_NO_ERROR;
}

u16 getMessagingMsgId(u8 * pu8Msg, olsize_t sMsg)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    return pHeader->jmh_u16MsgId;
}

u32 setMessagingMsgId(u8 * pu8Msg, u16 u16MsgId)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    pHeader->jmh_u16MsgId = u16MsgId;

    return JF_ERR_NO_ERROR;
}

u32 setMessagingMsgSourceId(u8 * pu8Msg, u32 sourceId)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    pHeader->jmh_u32SourceId = sourceId;

    return JF_ERR_NO_ERROR;
}

u32 getMessagingMsgSourceId(u8 * pu8Msg, olsize_t sMsg)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    return pHeader->jmh_u32SourceId;
}

u32 setMessagingMsgDestinationId(u8 * pu8Msg, u32 destinationId)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    pHeader->jmh_u32DestinationId = destinationId;

    return JF_ERR_NO_ERROR;
}

u32 getMessagingMsgDestinationId(u8 * pu8Msg, olsize_t sMsg)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    return pHeader->jmh_u32DestinationId;
}

u32 setMessagingMsgTransactionId(u8 * pu8Msg, u32 transactionId)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    pHeader->jmh_u32TransactionId = transactionId;

    return JF_ERR_NO_ERROR;
}

u32 getMessagingMsgTransactionId(u8 * pu8Msg, olsize_t sMsg)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    return pHeader->jmh_u32TransactionId;
}

u32 setMessagingMsgPayloadSize(u8 * pu8Msg, u32 u32PayloadSize)
{
    jf_messaging_header_t * pHeader = (jf_messaging_header_t *)pu8Msg;

    pHeader->jmh_u32PayloadSize = u32PayloadSize;

    return JF_ERR_NO_ERROR;
}

boolean_t isReservedMessagingMsg(u8 * pu8Msg, olsize_t sMsg)
{
    boolean_t bRet = FALSE;
    u16 u16MsgId = getMessagingMsgId(pu8Msg, sMsg);

    if (u16MsgId >= JF_MESSAGING_RESERVED_MSG_ID_START)
        bRet = TRUE;

    return bRet;
}

/*------------------------------------------------------------------------------------------------*/
