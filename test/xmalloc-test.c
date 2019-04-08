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
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_mem.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buffer;

    u32Ret = jf_mem_alloc((void **)&pu8Buffer, 50);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Succeed to allocate memory\n");
    }

    if (pu8Buffer != NULL)
    {
        ol_printf("free the memory\n");
        jf_mem_free((void **)pu8Buffer);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

