/**
 *  @file files/common.h
 *
 *  @brief Files header file, common data structure and routines for files
 *   library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef FILES_COMMON_H
#define FILES_COMMON_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */
#if defined(WINDOWS)
    #define ssize_t olint_t
#elif defined(LINUX)

#endif

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */


#endif /*FILES_COMMON_H*/

/*---------------------------------------------------------------------------*/


