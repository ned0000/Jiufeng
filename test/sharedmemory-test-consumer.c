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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "sharedmemory.h"
#include "process.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    shm_id_t * psi;
    olchar_t * pstrShared;
    olchar_t strErrMsg[300];

    if (argc < 2)
    {
        ol_printf("Missing parameter\n");
        ol_printf("sharedmemory-test-consumer sharedmemory-identifier\n");
        exit(0);
    }
    sleep(20);
    psi = argv[1];
    u32Ret = attachSharedMemory(psi, (void **)&pstrShared);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("succeed to attach shared memroy\n");
        ol_printf("%s\n", pstrShared);

        u32Ret = detachSharedMemory((void **)&pstrShared);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("succeed to detach shared memory\n");
    }
    else
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

