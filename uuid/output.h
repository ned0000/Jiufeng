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

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "uuid.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */
u32 outputUuid(uuid_obj_t * puo, uuid_fmt_t fmt, u8 * pu8Uuid, u32 u32Len);

#endif /*UUID_OUTPUT_H*/

/*---------------------------------------------------------------------------*/


