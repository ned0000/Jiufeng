/**
 *  @file clrmem.c
 *
 *  @brief Clear the memory according to a pattern
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "clrmem.h"

/* --- private data/data structure section --------------------------------- */
static u8 ls_u8ClearMemChar = 0;

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */
void clearMemory(void * ptr, u32 u32Len)
{
	u8 * p = ptr;
	u32 loop = u32Len, ctr = ls_u8ClearMemChar;

	while (loop--)
    {
		*(p++) = (u8)ctr;
#if defined(JIUFENG_64BIT)
		ctr += (17 + ((olsize_t)(u64)p & 0xF));
#else
		ctr += (17 + ((olsize_t)p & 0xF));
#endif
    }

	p = ol_memchr(ptr, (u8)ctr, u32Len);
	if (p != NULL)
    {
#if defined(JIUFENG_64BIT)
		ctr += (63 + (olsize_t)(u64)p);
#else
		ctr += (63 + (olsize_t)p);
#endif  
    }
    
    ls_u8ClearMemChar = (u8)ctr;
}

/*---------------------------------------------------------------------------*/



