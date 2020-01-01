/**
 *  @file messaging.c
 *
 *  @brief The implementation file for the messaging library.
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
#include "jf_ipaddr.h"
#include "jf_mutex.h"

#include "dispatchercommon.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    boolean_t im_bInitialized;
    u8 im_u8Reserved[7];

    u32 im_u32Reserved[4];

    jf_ipaddr_t im_jiServer;

    jf_mutex_t im_jmLock;
    u32 im_u32TransactionId;

} internal_messaging_t;

static internal_messaging_t ls_imMessaging;

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 jf_messaging_init(jf_messaging_init_param_t * pjsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_messaging_t * pim = &ls_imMessaging;

    assert(pjsip != NULL);

    jf_logger_logInfoMsg("init messaging");

    jf_ipaddr_setUdsAddr(&pim->im_jiServer, DISPATCHER_UDS_DIR);

    u32Ret = jf_mutex_init(&pim->im_jmLock);

    if (u32Ret == JF_ERR_NO_ERROR)
        pim->im_bInitialized = TRUE;
    else
        jf_messaging_fini();

    return u32Ret;
}

u32 jf_messaging_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_messaging_t * pim = &ls_imMessaging;

    jf_logger_logInfoMsg("fini messaging");

    u32Ret = jf_mutex_fini(&pim->im_jmLock);
    
    pim->im_bInitialized = FALSE;

    return u32Ret;
}

u32 jf_messaging_sendMsg(u8 * pu8Msg, olsize_t sMsg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_messaging_t * pim = &ls_imMessaging;


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


