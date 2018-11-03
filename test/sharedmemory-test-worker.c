/**
 *  @file sharedmemory-test-worker.c
 *
 *  @brief test file for shared memory worker 
 *
 *  @author Min Zhang
 *
 *  @note
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
#include "xtime.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    shm_id_t * psi;
    olchar_t * pstrShared;

    u32Ret = createSharedMemory(&psi, 4000);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("succeed to create shared memory\n");
        ol_printf("shm id: %s\n", psi);

        u32Ret = attachSharedMemory(psi, (void **)&pstrShared);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("succeed to attach shared memory\n");
        ol_strcpy(pstrShared, "have a good day");
        ol_printf("Write to shared memory\n");
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Sleep ....\n");
        sleep(60);

        u32Ret = detachSharedMemory((void **)&pstrShared);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("succeed to detach the shared memory\n");
    }

    if (psi != NULL)
        u32Ret = destroySharedMemory(&psi);

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("succeed to destroy shared memory\n");
    }
    else
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

