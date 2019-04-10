/**
 *  @file sharedmemory-test-consumer.c
 *
 *  @brief test file for share memory common object
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
#include "jf_sharedmemory.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_sharedmemory_id_t * pjsi;
    olchar_t * pstrShared;
    olchar_t strErrMsg[300];

    if (argc < 2)
    {
        ol_printf("Missing parameter\n");
        ol_printf("sharedmemory-test-consumer sharedmemory-identifier\n");
        exit(0);
    }
    sleep(20);
    pjsi = argv[1];
    u32Ret = jf_sharedmemory_attach(pjsi, (void **)&pstrShared);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("succeed to attach shared memroy\n");
        ol_printf("%s\n", pstrShared);

        u32Ret = jf_sharedmemory_detach((void **)&pstrShared);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("succeed to detach shared memory\n");
    }
    else
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

