/**
 *  @file dynlib-test.c
 *
 *  @brief Test file for dynamic library function defined in jf_dynlib common object.
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
#include "jf_dynlib.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

#if defined(WINDOWS)
static olchar_t * ls_pstrDynLib = "jf_string.dll";
#elif defined(LINUX)
static olchar_t * ls_pstrDynLib = "/lib/x86_64-linux-gnu/libm.so.6";
#endif

/* --- private routine section ------------------------------------------------------------------ */

static u32 _testDynlib(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
#if defined(WINDOWS)
    jf_dynlib_t * pjd = NULL;
    const olchar_t * (*getStringTrue)(const boolean_t bTrue);
    oldouble_t (*pow)(oldouble_t x, oldouble_t y);

    u32Ret = jf_dynlib_load(ls_pstrDynLib, &pjd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dynlib_getSymbolAddress(pjd, "jf_string_getStringTrue", (void **)&getStringTrue);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("%s\n", (*getStringTrue)(TRUE));
        ol_printf("%s\n", (*getStringTrue)(FALSE));
    }

    if (pjd != NULL)
        jf_dynlib_unload(&pjd);

#elif defined(LINUX)
    jf_dynlib_t * pjd = NULL;
    oldouble_t (*pow)(oldouble_t x, oldouble_t y);

    u32Ret = jf_dynlib_load(ls_pstrDynLib, &pjd);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_dynlib_getSymbolAddress(pjd, "pow", (void **)&pow);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("%f\n", (*pow)(3, 10));
    }

    if (pjd != NULL)
        jf_dynlib_unload(&pjd);
#endif

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
        u32Ret = _testDynlib();

        jf_jiukun_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
