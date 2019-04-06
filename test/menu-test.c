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

/* --- standard C lib header files ----------------------------------------- */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if defined(LINUX)
    #include <unistd.h>
#elif defined(WINDOWS)

#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "menu.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

static u32 _startInstall(void * pArg)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t install = *(olint_t *)pArg;

    ol_printf("install: %d\n", install);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
olint_t main(olint_t argc, olchar_t ** argv)
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

    exit(0);
}

/*--------------------------------------------------------------------------*/

