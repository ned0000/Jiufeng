/**
 *  @file attask-test.c
 *
 *  @brief Test file for jf_attask common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_rand.h"
#include "jf_time.h"
#include "jf_jiukun.h"
#include "jf_attask.h"
#include "jf_process.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bToTerminateTestAttask = FALSE;

static u32 ls_u32TestAttaskIndex = 0;

/* --- private routine section ------------------------------------------------------------------ */

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    ls_bToTerminateTestAttask = TRUE;
}

static u32 _onCallbackOfTestAttaskItem(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("data: %s", (olchar_t *)pData);

    return u32Ret;
}

static u32 _destroyTestAttaskItemData(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("data: %s", (olchar_t *)*ppData);
    jf_jiukun_freeMemory(ppData);

    return u32Ret;
}

static u32 _addTestAttaskItem(jf_attask_t * pAttask)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pData = NULL;
    u32 u32Seconds = 0;

    u32Seconds = jf_rand_getU32InRange(0, 30);

    u32Ret = jf_jiukun_allocMemory((void **)&pData, 64);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_sprintf(pData, "attask item data %u", ls_u32TestAttaskIndex);
        ls_u32TestAttaskIndex ++;

        JF_LOGGER_DEBUG("data: %s, expire: %u", pData, u32Seconds * 1000);

        u32Ret = jf_attask_addItem(
            pAttask, pData, u32Seconds * 1000, _onCallbackOfTestAttaskItem,
            _destroyTestAttaskItemData);
    }

    return u32Ret;
}

static u32 _enterTestAttaskLoop(jf_attask_t * pAttask)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Blocktime = 0;

    while (! ls_bToTerminateTestAttask)
    {
        u32Ret = _addTestAttaskItem(pAttask);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _addTestAttaskItem(pAttask);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_attask_check(pAttask, &u32Blocktime);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            JF_LOGGER_INFO("Sleep %u milli-seconds", u32Blocktime);
            jf_time_milliSleep(u32Blocktime);
        }
    }

    return u32Ret;
}

static u32 _testAttask(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_attask_t * pAttask = NULL;

    u32Ret = jf_attask_create(&pAttask);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _enterTestAttaskLoop(pAttask);

    if (pAttask != NULL)
        jf_attask_destroy(&pAttask);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "ATTASK-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    jf_logger_init(&jlipParam);

    u32Ret = jf_jiukun_init(&jjip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_process_registerSignalHandlers(_terminate);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = _testAttask();
        }

        jf_jiukun_fini();
    }

    jf_logger_fini();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
