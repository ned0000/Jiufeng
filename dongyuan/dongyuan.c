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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "logger.h"
#include "process.h"
#include "dongyuan.h"
#include "files.h"
#include "servmgmt.h"
#include "xmalloc.h"

/* --- private data structures --------------------------------------------- */

typedef struct
{
    olchar_t * ig_pstrSettingFile;
    u32 ig_u8Reserved[8];
} internal_dongyuan_t;

/* --- private routine section --------------------------------------------- */


/* --- public routine section ---------------------------------------------- */
u32 createDongyuan(dongyuan_t ** ppDongyuan, dongyuan_param_t * pgp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_dongyuan_t * pig;
    jf_servmgmt_init_param_t jsip;
    olchar_t strExecutablePath[MAX_PATH_LEN];

    logInfoMsg("create dongyuan");

    u32Ret = xcalloc((void **)&pig, sizeof(internal_dongyuan_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        pig->ig_pstrSettingFile = pgp->gp_pstrSettingFile;

        /*change the working directory*/
        getDirectoryName(strExecutablePath, MAX_PATH_LEN, pgp->gp_pstrCmdLine);
        if (strlen(strExecutablePath) > 0)
            u32Ret = setCurrentWorkingDirectory(strExecutablePath);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_memset(&jsip, 0, sizeof(jsip));

        u32Ret = jf_servmgmt_init(&jsip);
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppDongyuan = pig;
    else if (pig != NULL)
        destroyDongyuan((dongyuan_t **)&pig);

    return u32Ret;
}

u32 destroyDongyuan(dongyuan_t ** ppDongyuan)
{
    u32 u32Ret = OLERR_NO_ERROR;

    assert((ppDongyuan != NULL) && (*ppDongyuan != NULL));


    xfree(ppDongyuan);

    return u32Ret;
}

u32 startDongyuan(dongyuan_t * pDongyuan)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_dongyuan_t * pig;
    jf_servmgmt_start_param_t jssp;

    assert(pDongyuan != NULL);

    pig = (internal_dongyuan_t *)pDongyuan;

    ol_memset(&jssp, 0, sizeof(jssp));
    jssp.jssp_pstrSettingFile = pig->ig_pstrSettingFile;

    u32Ret = jf_servmgmt_start(&jssp);

    return u32Ret;
}

u32 stopDongyuan(dongyuan_t * pDongyuan)
{
    u32 u32Ret = OLERR_NO_ERROR;
//    internal_dongyuan_t * pig;

    assert(pDongyuan != NULL);

//    pig = (internal_dongyuan_t *)pDongyuan;

    u32Ret = jf_servmgmt_stop();

    return u32Ret;
}

u32 setDefaultDongyuanParam(dongyuan_param_t * pgp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    memset(pgp, 0, sizeof(dongyuan_param_t));


    return u32Ret;
}

/*---------------------------------------------------------------------------*/


