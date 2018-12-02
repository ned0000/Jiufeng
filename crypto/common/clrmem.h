/**
 *  @file clrmem.h
 *
 *  @brief clear memory header file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef CRYPTO_COMMON_CLRMEM_H
#define CRYPTO_COMMON_CLRMEM_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */
void clearMemory(void * ptr, u32 u32Len);

#endif /*CRYPTO_COMMON_CLRMEM_H*/

/*---------------------------------------------------------------------------*/


