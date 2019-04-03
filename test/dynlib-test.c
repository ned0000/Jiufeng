/**
 *  @file dynlib-test.c
 *
 *  @brief test file for dynlib common object
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "xmalloc.h"
#include "dynlib.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    dyn_lib_t * pdl = NULL;
    oldouble_t (*pow)(oldouble_t x, oldouble_t y);
    olchar_t strErrMsg[300];

    u32Ret = loadDynLib ("libm.so", &pdl);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = getSymbolAddress(pdl, "powss", (void **)&pow);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        printf ("%f\n", (*pow)(3, 10));
    }

    if (pdl != NULL)
        freeDynLib(&pdl);

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

