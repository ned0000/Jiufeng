/**
 *  @file sharedmemory-test-worker.c
 *
 *  @brief Test file containing worker for shared memory function defined in jf_sharedmemory common
 *   object. 
 *
 *  @author Min Zhang
 *
 *  @note
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

static u32 _sharedmemoryTestWorker(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_sharedmemory_id_t * pjsi;
    olchar_t * pstrShared;

    u32Ret = jf_sharedmemory_create(&pjsi, 4000);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Succeed to create shared memory\n");
        ol_printf("Shared memory ID: %s\n", pjsi);

        u32Ret = jf_sharedmemory_attach(pjsi, (void **)&pstrShared);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Succeed to attach shared memory\n");
        ol_strcpy(pstrShared, "have a good day");
        ol_printf("Write to shared memory\n");
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Sleep ....\n");
        sleep(60);

        u32Ret = jf_sharedmemory_detach((void **)&pstrShared);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("succeed to detach the shared memory\n");
    }

    if (pjsi != NULL)
        u32Ret = jf_sharedmemory_destroy(&pjsi);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("succeed to destroy shared memory\n");
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
        u32Ret = _sharedmemoryTestWorker();

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
