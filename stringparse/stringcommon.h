/**
 *  @file stringparse/stringcommon.h
 *
 *  @brief Header file for common definitions and routines in string library.
 *
 *  @author Min Zhang
 *
 */

#ifndef STRING_STRINGCOMMON_H
#define STRING_STRINGCOMMON_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_messaging.h"
#include "jf_hlisthead.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the string for enabled.
 */
#define JF_STRING_ENABLED                           "Enabled"

/** Define the string for disabled.
 */
#define JF_STRING_DISABLED                          "Disabled"

/** Define the string for yes.
 */
#define JF_STRING_YES                               "Yes"

/** Define the string for no.
 */
#define JF_STRING_NO                                "No"

/** Define the string for true.
 */
#define JF_STRING_TRUE                              "True"

/** Define the string for false.
 */
#define JF_STRING_FALSE                             "False"


#ifdef WINDOWS
    #define ONE_TEGABYTE                            (0x10000000000i64)
#else
    #define ONE_TEGABYTE                            (0x10000000000LL)
#endif
#define ONE_GIGABYTE                                (0x40000000)
#define ONE_MEGABYTE                                (0x100000)
#define ONE_KILOBYTE                                (0x400)

#ifdef WINDOWS
    #define ONE_TEGABYTE_1000_BASED                 (0xE8D4A51000i64)
#else
    #define ONE_TEGABYTE_1000_BASED                 (0xE8D4A51000LL)
#endif
#define ONE_GIGABYTE_1000_BASED                     (0x3B9ACA00)
#define ONE_MEGABYTE_1000_BASED                     (0xF4240)
#define ONE_KILOBYTE_1000_BASED                     (0x3E8)



/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */



#endif /*STRING_STRINGCOMMON_H*/

/*------------------------------------------------------------------------------------------------*/
