/**
 *  @file dongyuan.h
 *
 *  @brief Header file for dongyuan service.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_DONGYUAN_H
#define JIUFENG_DONGYUAN_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Donyuan service name.
 */
#define DONGYUAN_SERVER_NAME                   "dongyuan"

/* --- data structures -------------------------------------------------------------------------- */

/** Define the parameter for initializing dongyuan service.
 */
typedef struct
{
    /**Command line of the service.*/
    olchar_t * dp_pstrCmdLine;
    /**Service management setting file.*/
    olchar_t * dp_pstrSettingFile;
    u8 dp_u8Reserved[16];
} dongyuan_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Set default parameter for dongyuan service.
 *
 *  @param pgp [in] The parameter for initializing the service.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 */
u32 setDefaultDongyuanParam(dongyuan_param_t * pgp);

/** Initialize dongyuan service.
 *
 *  @param pgp [in] The parameter for initializing the service.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 */
u32 initDongyuan(dongyuan_param_t * pgp);

/** Finalize the dongyuan service.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 finiDongyuan(void);

/** Start dongyuan service.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 startDongyuan(void);

/** Stop dongyuan service.
 *
 *  @note
 *  -# The routine is called in signal handler, only simple stuff is allowed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 stopDongyuan(void);

#endif /*JIUFENG_DONGYUAN_H*/

/*------------------------------------------------------------------------------------------------*/
