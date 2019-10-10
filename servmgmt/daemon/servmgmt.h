/**
 *  @file servmgmt.h
 *
 *  @brief Service mamagement header file, provide some functional routine for service management
 *
 *  @author Min Zhang
 *
 *  @note Create the setting and status files. The location of those files are in current working
 *   directory.
 *  @note User libxml2-dev as the XML parser to parse setting file
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

typedef struct
{
    olchar_t * smip_pstrSettingFile;
    jf_network_chain_t * smip_pjncChain;
    u8 smip_u8Reserved[32];
} serv_mgmt_init_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 initServMgmt(serv_mgmt_init_param_t * psip);

u32 finiServMgmt(void);

u32 startServMgmt(void);

u32 stopServMgmt(void);

u32 getServMgmtServInfoList(jf_serv_info_list_t * pjsil);

u32 getServMgmtServInfo(const olchar_t * pstrName, jf_serv_info_t * pjsi);

u32 stopServMgmtServ(const olchar_t * pstrName);

u32 startServMgmtServ(const olchar_t * pstrName);

u32 setServMgmtServStartupType(const olchar_t * pstrName, const u8 u8StartupType);

u32 handleServMgmtSignal(olint_t sig);

#endif /*DONGYUAN_SERVMGMT_H*/

/*------------------------------------------------------------------------------------------------*/


