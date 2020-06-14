/**
 *  @file utimer-test.c
 *
 *  @brief Test file for utimer function defind in jf_network library.
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
#include "jf_network.h"
#include "jf_string.h"
#include "jf_process.h"
#include "jf_thread.h"
#include "jf_time.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

static jf_network_chain_t * ls_pjncUtChain = NULL;

static jf_network_utimer_t * ls_pjnuUtUtimer = NULL;

static boolean_t ls_bToTerminateUt = FALSE;

/* --- private routine section ------------------------------------------------------------------ */

static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    if (ls_pjncUtChain != NULL)
        jf_network_stopChain(ls_pjncUtChain);

    ls_bToTerminateUt = TRUE;
}

JF_THREAD_RETURN_VALUE _utThread(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("_utThread starts\n");

    u32Ret = jf_network_createChain(&ls_pjncUtChain);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_createUtimer(ls_pjncUtChain, &ls_pjnuUtUtimer, "utimer-test");
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_startChain(ls_pjncUtChain);
    }

    if (ls_pjnuUtUtimer != NULL)
        jf_network_destroyUtimer(&ls_pjnuUtUtimer);

    if (ls_pjncUtChain != NULL)
        jf_network_destroyChain(&ls_pjncUtChain);
    
    JF_THREAD_RETURN(u32Ret);
}

static u32 _onUtCallbackOfUtimerItem(void * pData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("ut timer is triggered, data: %s\n", (olchar_t *)pData);

    return u32Ret;
}

static u32 _destroyUtUtimerItemData(void ** ppData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("ut destroy utimer data, data: %s\n", (olchar_t *)*ppData);
    jf_jiukun_freeMemory(ppData);

    return u32Ret;
}

static u32 _addUtUtimerItem()
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pData1 = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pData1, 64);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pData1, 64);
        ol_strcpy(pData1, "_addUtUtimerItem");

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData1, 50, _onUtCallbackOfUtimerItem, _destroyUtUtimerItemData);
    }

    return u32Ret;
}

static u32 _addAndRemoveUtUtimerItem()
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pData1 = NULL, * pData2 = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pData1, 64);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pData1, 64);
        ol_strcpy(pData1, "_addAndRemoveUtUtimerItem 1");

        u32Ret = jf_jiukun_allocMemory((void **)&pData2, 64);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_strcpy(pData2, "_addAndRemoveUtUtimerItem 2");

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData1, 50, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData1, 140, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData1, 20, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData1, 170, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData1, 80, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData1, 110, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData2, 10, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_removeUtimerItem(ls_pjnuUtUtimer, (void *)pData2);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData2, 40, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_removeUtimerItem(ls_pjnuUtUtimer, (void *)pData2);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData2, 1000, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_removeUtimerItem(ls_pjnuUtUtimer, (void *)pData2);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData2, 10, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData2, 40, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_addUtimerItem(
            ls_pjnuUtUtimer, pData2, 1000, _onUtCallbackOfUtimerItem, NULL);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

        u32Ret = jf_network_removeUtimerItem(ls_pjnuUtUtimer, (void *)pData2);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_dumpUtimerItem(ls_pjnuUtUtimer);

    }

    if (pData1 != NULL)
    {
        jf_network_removeUtimerItem(ls_pjnuUtUtimer, (void *)pData1);
        jf_jiukun_freeMemory((void **)&pData1);
    }

    if (pData2 != NULL)
    {
        jf_network_removeUtimerItem(ls_pjnuUtUtimer, (void *)pData2);
        jf_jiukun_freeMemory((void **)&pData2);
    }
    
    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_thread_id_t threadid;
    u32 u32RetCode = 0;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "UTIMER-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    jf_logger_init(&jlipParam);

    u32Ret = jf_jiukun_init(&jjip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_process_initSocket();
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_process_registerSignalHandlers(_terminate);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_thread_create(&threadid, NULL, _utThread, NULL);
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                jf_time_sleep(3);

                u32Ret = _addUtUtimerItem();
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = _addAndRemoveUtUtimerItem();
            }

            if (u32Ret == JF_ERR_NO_ERROR)
            {
                while (! ls_bToTerminateUt)
                {
                    jf_time_sleep(3);
                }
            }

            jf_process_finiSocket();
            jf_thread_waitForThreadTermination(threadid, &u32RetCode);
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
