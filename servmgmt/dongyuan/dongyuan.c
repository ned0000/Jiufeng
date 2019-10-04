/**
 *  @file dongyuan.c
 *
 *  @brief software management implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_process.h"
#include "jf_file.h"
#include "jf_serv.h"
#include "jf_mem.h"
#include "jf_network.h"

#include "dongyuan.h"
#include "servmgmt.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct
{
    olchar_t * ig_pstrSettingFile;
    u32 ig_u8Reserved[8];

    jf_network_chain_t * ls_pjncDongyuanChain;
    jf_network_assocket_t * ls_pjnaDongyuanAssocket;

} internal_dongyuan_t;

/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */
u32 createDongyuan(dongyuan_t ** ppDongyuan, dongyuan_param_t * pgp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_dongyuan_t * pig;
    serv_mgmt_init_param_t smip;
    olchar_t strExecutablePath[JF_LIMIT_MAX_PATH_LEN];

    jf_logger_logInfoMsg("create dongyuan");

    u32Ret = jf_mem_calloc((void **)&pig, sizeof(internal_dongyuan_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pig->ig_pstrSettingFile = pgp->gp_pstrSettingFile;

        /*change the working directory*/
        jf_file_getDirectoryName(
            strExecutablePath, JF_LIMIT_MAX_PATH_LEN, pgp->gp_pstrCmdLine);
        if (strlen(strExecutablePath) > 0)
            u32Ret = jf_process_setCurrentWorkingDirectory(strExecutablePath);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&smip, 0, sizeof(smip));

        u32Ret = initServMgmt(&smip);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppDongyuan = pig;
    else if (pig != NULL)
        destroyDongyuan((dongyuan_t **)&pig);

    return u32Ret;
}

u32 destroyDongyuan(dongyuan_t ** ppDongyuan)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((ppDongyuan != NULL) && (*ppDongyuan != NULL));


    jf_mem_free(ppDongyuan);

    return u32Ret;
}

u32 startDongyuan(dongyuan_t * pDongyuan)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_dongyuan_t * pig = (internal_dongyuan_t *)pDongyuan;

    assert(pDongyuan != NULL);



    return u32Ret;
}

u32 stopDongyuan(dongyuan_t * pDongyuan)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_dongyuan_t * pig;

    assert(pDongyuan != NULL);



    return u32Ret;
}

u32 setDefaultDongyuanParam(dongyuan_param_t * pgp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_memset(pgp, 0, sizeof(dongyuan_param_t));


    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/


