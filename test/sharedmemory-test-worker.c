/**
 *  @file sharedmemory-test-worker.c
 *
 *  @brief test file for shared memory worker 
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_sharedmemory.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_sharedmemory_id_t * pjsi;
    olchar_t * pstrShared;

    u32Ret = jf_sharedmemory_create(&pjsi, 4000);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("succeed to create shared memory\n");
        ol_printf("shm id: %s\n", pjsi);

        u32Ret = jf_sharedmemory_attach(pjsi, (void **)&pstrShared);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("succeed to attach shared memory\n");
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
    else
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

