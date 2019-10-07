/**
 *  @file serv.c
 *
 *  @brief The service library
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
#include "jf_mem.h"
#include "jf_sharedmemory.h"
#include "jf_filestream.h"
#include "jf_process.h"
#include "jf_attask.h"
#include "jf_time.h"
#include "jf_serv.h"

#include "servmgmtcommon.h"
#include "servmgmtmsg.h"

/* --- private data/data structure section ------------------------------------------------------ */


typedef struct
{
    /*the shared memory contains the status of all services*/
    boolean_t is_bInitialized;
    u8 is_u8Reserved[7];

    u32 is_u32Reserved[4];
} internal_serv_t;

static internal_serv_t ls_isServ;

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 jf_serv_init(jf_serv_init_param_t * pjsip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;

    assert(pjsip != NULL);

    jf_logger_logInfoMsg("init serv");

    if (u32Ret == JF_ERR_NO_ERROR)
        pis->is_bInitialized = TRUE;
    else
        jf_serv_fini();

    return u32Ret;
}

u32 jf_serv_fini(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;

    jf_logger_logInfoMsg("fini serv mgmt");

    pis->is_bInitialized = FALSE;

    return u32Ret;
}

static u32 _initServMgmtMsgHeader(servmgmt_msg_header_t * pHeader, internal_serv_t * pis)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

//    pHeader->smh_jpiSourceId = jf_process_getCurrentId();

    return u32Ret;
}

u32 jf_serv_getInfoList(jf_serv_info_list_t * pjsil)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_serv_t * pis = &ls_isServ;
    servmgmt_get_info_list_req_t sgilr;

    ol_memset(&sgilr, 0, sizeof(sgilr));
    _initServMgmtMsgHeader(&sgilr.sgilr_smhHeader, pis);

//    u32Ret = jf_sharedmemory_create


    return u32Ret;
}

u32 jf_serv_getInfo(const olchar_t * pstrName, jf_serv_info_t * pjsi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

u32 jf_serv_stopServ(const olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

u32 jf_serv_startServ(const olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

const olchar_t * jf_serv_getStringServStatus(const u8 u8Status)
{
    return getStringServStatus(u8Status);
}

const olchar_t * jf_serv_getStringServStartupType(const u8 u8StartupType)
{
    return getStringServStartupType(u8StartupType);
}

u32 jf_serv_getServStartupTypeFromString(const olchar_t * pstrType, u8 * pu8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = getServStartupTypeFromString(pstrType, pu8StartupType);

    return u32Ret;
}

u32 jf_serv_setServStartupType(const olchar_t * pstrName, const u8 u8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


