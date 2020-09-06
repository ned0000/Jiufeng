/**
 *  @file jf_rand.c
 *
 *  @brief Implementation file for random number object.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_rand.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static void _seedRandomNumber(void)
{
    static boolean_t bRandomNumSeeded = FALSE;

    if (! bRandomNumSeeded)
    {
        ol_srand(ol_time(NULL));
        bRandomNumSeeded = TRUE;
    }
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_rand_getU32InRange(u32 u32Lower, u32 u32Upper)
{
    u32 u32Ret = u32Lower;

    assert(u32Lower < u32Upper);

    _seedRandomNumber();

    u32Ret = ol_random();
    u32Ret %= u32Upper - u32Lower;
    u32Ret += u32Lower;

    return u32Ret;
}

u32 jf_rand_getU32(void)
{
    u32 u32Ret = 0;

    _seedRandomNumber();

    u32Ret = ol_random();

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


