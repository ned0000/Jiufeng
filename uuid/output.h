/**
 *  @file output.h
 *
 *  @brief output UUID with specified format
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef UUID_OUTPUT_H
#define UUID_OUTPUT_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_uuid.h"
#include "common.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 outputUuid(uuid_obj_t * puo, jf_uuid_fmt_t fmt, u8 * pu8Uuid, u32 u32Len);

#endif /*UUID_OUTPUT_H*/

/*------------------------------------------------------------------------------------------------*/


