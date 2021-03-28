/**
 *  @file servmgmt.h
 *
 *  @brief Service mamagement header file, provide some functional routine for service management.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Create the setting and status files. The location of those files are in current working
 *   directory.
 */

#ifndef DONGYUAN_SERVMGMT_H
#define DONGYUAN_SERVMGMT_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_serv.h"
#include "jf_network.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Parameter for initializing the service management module.
 */
typedef struct
{
    /**The full path of the setting file.*/
    olchar_t * smip_pstrSettingFile;
    /**The network chain.*/
    jf_network_chain_t * smip_pjncChain;
    u8 smip_u8Reserved[32];
} serv_mgmt_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the service management module.
 *
 *  @param psip [in] The parameter for initializing the module.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 */
u32 initServMgmt(serv_mgmt_init_param_t * psip);

/** Finalize the service management module.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 finiServMgmt(void);

/** Start the service management module.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 startServMgmt(void);

/** Stop the service management module.
 *
 *  @note
 *  -# The routine is called in signal handler, only simple stuff is allowed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 stopServMgmt(void);

/** Get all services' information list.
 *
 *  @param pjsil [in/out] The service information list.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 getServMgmtServInfoList(jf_serv_info_list_t * pjsil);

/** Get one service information list.
 *
 *  @param pstrName [in] The service name.
 *  @param pjsi [out] The service information object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 getServMgmtServInfo(const olchar_t * pstrName, jf_serv_info_t * pjsi);

/** Stop a service.
 *
 *  @param pstrName [in] The service name.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 stopServMgmtServ(const olchar_t * pstrName);

/** Stop a service.
 *
 *  @param pstrName [in] The service name.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 startServMgmtServ(const olchar_t * pstrName);

/** Set the startup type of the service.
 *
 *  @param pstrName [in] The service name.
 *  @param u8StartupType [in] The startup type.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 setServMgmtServStartupType(const olchar_t * pstrName, const u8 u8StartupType);

/** Handle the signal for service management module.
 *
 *  @note
 *  -# SIGCHLD is handled.
 *
 *  @param sig [in] The signal.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 handleServMgmtSignal(olint_t sig);

#endif /*DONGYUAN_SERVMGMT_H*/

/*------------------------------------------------------------------------------------------------*/
