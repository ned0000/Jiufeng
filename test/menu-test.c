/**
 *  @file menu-test.c
 *
 *  @brief the test file for menu object
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if defined(LINUX)
    #include <unistd.h>
#elif defined(WINDOWS)

#endif

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
    jf_menu_t *topMenu = NULL;
    jf_menu_t * pInstall, * pQuery;
    olint_t install = 1;

    u32Ret = jf_menu_createTopMenu(NULL, NULL, NULL, &topMenu);
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
        jf_menu_start(topMenu);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = jf_jiukun_init(&jjip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _testMenu();

        jf_jiukun_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

