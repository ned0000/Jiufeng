/**
 *  @file jf_rand.h
 *
 *  @brief The random number common object header file.
 *
 *  @author Min Zhang
 *  
 *  @note Routines declared in this file are included in jf_rand object
 *
 */

#ifndef JIUTAI_RAND_H
#define JIUTAI_RAND_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */


/* --- functional routines ------------------------------------------------- */

/** Get random u32 number in range
 *  
 *  @param u32Lower [in] the lower edge of the range
 *  @param u32Upper [in] the upper edge of the range
 *
 *  @return the random u32 number
 */
u32 jf_rand_getU32InRange(u32 u32Lower, u32 u32Upper);


#endif /*JIUTAI_RAND_H*/

/*---------------------------------------------------------------------------*/


