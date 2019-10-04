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

u32 jf_serv_getInfo(jf_serv_t * pjs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

u32 jf_serv_stopServ(olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

u32 jf_serv_startServ(olchar_t * pstrName)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

const olchar_t * jf_serv_getStringServStatus(u8 u8Status)
{
    return getStringServStatus(u8Status);
}

const olchar_t * jf_serv_getStringServStartupType(u8 u8StartupType)
{
    return getStringServStartupType(u8StartupType);
}

u32 jf_serv_getServStartupTypeFromString(const olchar_t * pstrType, u8 * pu8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = getServStartupTypeFromString(pstrType, pu8StartupType);

    return u32Ret;
}

u32 jf_serv_setServStartupType(olchar_t * pstrName, u8 u8StartupType)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


