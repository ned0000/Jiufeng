/**
 *  @file sharedmemory-test-consumer.c
 *
 *  @brief Test file containing consumer for shared memory function defined in jf_sharedmemory
 *   common object. 
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
#include "jf_sharedmemory.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

static u32 _sharedmemoryTestConsumer(jf_sharedmemory_id_t * pjsi)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * pstrShared;

    ol_printf("Shared memory ID: %s\n", pjsi);
    u32Ret = jf_sharedmemory_attach(pjsi, (void **)&pstrShared);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Succeed to attach shared memroy\n");
        ol_printf("%s\n", pstrShared);

        u32Ret = jf_sharedmemory_detach((void **)&pstrShared);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Succeed to detach shared memory\n");
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_jiukun_init_param_t jjip;

    if (argc < 2)
    {
        ol_printf("Missing parameter\n");
        ol_printf("sharedmemory-test-consumer sharedmemory-identifier\n");
        exit(0);
    }

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    u32Ret = jf_jiukun_init(&jjip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _sharedmemoryTestConsumer(argv[1]);

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
