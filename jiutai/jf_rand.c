/**
 *  @file jf_rand.c
 *
 *  @brief The random number common object implementation file
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
#if defined(LINUX)
    #include <sys/time.h>
#elif defined(WINDOWS)
    #include <time.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_rand.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

u32 jf_rand_getU32InRange(u32 u32Lower, u32 u32Upper)
{
    u32 u32Ret = u32Lower;
    static boolean_t bRandomNumSeeded = FALSE;

    assert(u32Lower < u32Upper);

    if (! bRandomNumSeeded)
    {
        ol_srand(ol_time(NULL));
        bRandomNumSeeded = TRUE;
    }

    u32Ret = ol_random();
    u32Ret %= u32Upper - u32Lower;
    u32Ret += u32Lower;

    return u32Ret;
}


/*---------------------------------------------------------------------------*/


