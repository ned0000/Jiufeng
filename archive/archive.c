/**
 *  @file archive.c
 *
 *  @brief The archive library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_archive.h"
#include "jf_err.h"
#include "jf_linklist.h"
#include "jf_logger.h"
#include "jf_jiukun.h"

#include "archivecommon.h"
#include "arfile.h"
#include "create.h"
#include "extract.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_archive_create(
    jf_linklist_t * pMemberFile, olchar_t * pstrArchiveName, jf_archive_create_param_t * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Fullpath = NULL;
    u8 * pu8Buffer = NULL;
    ar_file_param_t afp;
    ar_file_t * paf = NULL;
	jf_linklist_node_t * pNode = NULL;

    assert((pMemberFile != NULL) && (pstrArchiveName != NULL) && (pParam != NULL));

    u32Ret = jf_jiukun_allocMemory((void **)&pu8Buffer, MAX_ARCHIVE_BUFFER_LEN);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(&afp, 0, sizeof(ar_file_param_t));
        u32Ret = createArFile(pstrArchiveName, &afp, &paf);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
		pNode = jf_linklist_getFirstNode(pMemberFile);
		while ((pNode != NULL) && (u32Ret == JF_ERR_NO_ERROR))
		{
            pu8Fullpath = (u8 *)jf_linklist_getDataFromNode(pNode);
            if (pu8Fullpath != NULL)
            {
                u32Ret = writeToArchive(
                    paf, pu8Fullpath, pu8Buffer, MAX_ARCHIVE_BUFFER_LEN, pParam);
            }

			pNode = jf_linklist_getNextNode(pNode);
        }
    }

    if (pu8Buffer != NULL)
        jf_jiukun_freeMemory((void **)&pu8Buffer);

    if (paf != NULL)
        destroyArFile(&paf);

    return u32Ret;
}

u32 jf_archive_extract(
    olchar_t * pstrArchiveName, jf_archive_extract_param_t * pParam)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u8 * pu8Buffer = NULL;
    ar_file_param_t afp;
    ar_file_t * paf = NULL;

    assert((pstrArchiveName != NULL) && (pParam != NULL));

    u32Ret = jf_jiukun_allocMemory((void **)&pu8Buffer, MAX_ARCHIVE_BUFFER_LEN);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&afp, sizeof(ar_file_param_t));
        afp.afp_bExtract = TRUE;
        u32Ret = createArFile(pstrArchiveName, &afp, &paf);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = extractFromArchive(paf, pu8Buffer, MAX_ARCHIVE_BUFFER_LEN, pParam);
    }

    if (pu8Buffer != NULL)
        jf_jiukun_freeMemory((void **)&pu8Buffer);

    if (paf != NULL)
        destroyArFile(&paf);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

