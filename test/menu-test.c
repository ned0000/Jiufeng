/**
 *  @file menu-test.c
 *
 *  @brief Test file for menu function define in jf_menu common object.
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
#include "jf_menu.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _startInstall(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t install = *(olint_t *)pArg;

    ol_printf("install: %d\n", install);

    return u32Ret;
}

static u32 _testMenu(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_menu_t * topMenu = NULL;
    jf_menu_t * pInstall = NULL, * pQuery = NULL;
    olint_t install = 1;

    u32Ret = jf_menu_create(NULL, NULL, NULL, &topMenu);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_menu_addSubMenu(
            topMenu, "Install", NULL, 0, NULL, NULL, NULL, &pInstall);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_menu_addSubMenu(
            pInstall, "query", NULL, 0, NULL, NULL, NULL, &pQuery);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_menu_addEntry(
            pInstall, "Start", NULL, 0, _startInstall, (void *)&install);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_menu_run(topMenu);
    }

    if (topMenu != NULL)
        jf_menu_destroy(&topMenu);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_jiukun_init_param_t jjip;
    jf_logger_init_param_t jlipParam;

    ol_bzero(&jlipParam, sizeof(jlipParam));
    jlipParam.jlip_pstrCallerName = "MENU-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DEBUG;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    jf_logger_init(&jlipParam);

    u32Ret = jf_jiukun_init(&jjip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _testMenu();

        jf_jiukun_fini();
    }

    jf_logger_fini();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

