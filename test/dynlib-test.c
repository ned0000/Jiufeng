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
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_mem.h"
#include "jf_dynlib.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_dynlib_t * pjd = NULL;
    oldouble_t (*pow)(oldouble_t x, oldouble_t y);
    olchar_t strErrMsg[300];

    u32Ret = jf_dynlib_load ("libm.so", &pjd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dynlib_getSymbolAddress(pjd, "powss", (void **)&pow);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        printf ("%f\n", (*pow)(3, 10));
    }

    if (pjd != NULL)
        jf_dynlib_unload(&pjd);

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

