/**
 *  @file jf_mem.c
 *
 *  @brief Implementation file for memory allocation routines
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_mem_alloc(void ** pptr, olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pptr != NULL) && (size > 0));

    *pptr = malloc(size);
    if (*pptr == NULL)
        u32Ret = JF_ERR_OUT_OF_MEMORY;

    return u32Ret;
}

u32 jf_mem_calloc(void ** pptr, olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pptr != NULL) && (size > 0));

    *pptr = malloc(size);
    if (*pptr == NULL)
        u32Ret = JF_ERR_OUT_OF_MEMORY;
    else
        memset(*pptr, 0, size);

    return u32Ret;
}

u32 jf_mem_realloc(void ** pptr, olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    void * ptr;

    assert((pptr != NULL) && (size > 0));

    ptr = realloc(*pptr, size);
    *pptr = ptr;

    return u32Ret;
}

u32 jf_mem_free(void ** pptr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert(pptr != NULL);

    free(*pptr);
    *pptr = NULL;

    return u32Ret;
}

u32 jf_mem_duplicate(void ** pptr, const u8 * pu8Buffer, const olsize_t size)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pptr != NULL) && (pu8Buffer != NULL) && (size > 0));

    u32Ret = jf_mem_alloc(pptr, size);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memcpy(*pptr, pu8Buffer, size);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


