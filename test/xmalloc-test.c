/**
 *  @file xmalloc-test.c
 *
 *  @brief test file for xmalloc object
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

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 * pu8Buffer;

    u32Ret = xmalloc((void **)&pu8Buffer, 50);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Succeed to allocate memory\n");
    }

    if (pu8Buffer != NULL)
    {
        ol_printf("free the memory\n");
        xfree((void **)pu8Buffer);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

