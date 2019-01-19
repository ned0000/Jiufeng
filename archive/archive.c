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

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "archive.h"
#include "errcode.h"
#include "bases.h"
#include "logger.h"
#include "files.h"
#include "xmalloc.h"
#include "archivecommon.h"
#include "arfile.h"
#include "create.h"
#include "extract.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */
u32 createArchive(
    link_list_t * pMemberFile, olchar_t * pstrArchiveName,
    create_archive_param_t * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 * pu8Fullpath;
    u8 * pu8Buffer;
    ar_file_param_t afp;
    ar_file_t * paf = NULL;
	link_list_node_t * pNode;

    assert(
        (pMemberFile != NULL) && (pstrArchiveName != NULL) && (pParam != NULL));

    u32Ret = xmalloc((void **)&pu8Buffer, MAX_ARCHIVE_BUFFER_LEN);
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&afp, 0, sizeof(ar_file_param_t));
        u32Ret = createArFile(pstrArchiveName, &afp, &paf);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
		pNode = getFirstNodeOfLinkList(pMemberFile);
		while ((pNode != NULL) && (u32Ret == OLERR_NO_ERROR))
		{
            pu8Fullpath = (u8 *)getDataFromLinkListNode(pNode);
            if (pu8Fullpath != NULL)
            {
                u32Ret = writeToArchive(
                    paf, pu8Fullpath, pu8Buffer, MAX_ARCHIVE_BUFFER_LEN,
                    pParam);
            }

			pNode = getNextNodeOfLinkList(pNode);
        }
    }

    if (pu8Buffer != NULL)
        xfree((void **)&pu8Buffer);

    if (paf != NULL)
        destroyArFile(&paf);

    return u32Ret;
}

u32 extractArchive(olchar_t * pstrArchiveName, extract_archive_param_t * pParam)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u8 * pu8Buffer;
    ar_file_param_t afp;
    ar_file_t * paf = NULL;

    assert((pstrArchiveName != NULL) && (pParam != NULL));

    u32Ret = xmalloc((void **)&pu8Buffer, MAX_ARCHIVE_BUFFER_LEN);
    if (u32Ret == OLERR_NO_ERROR)
    {
        memset(&afp, 0, sizeof(ar_file_param_t));
        afp.afp_bExtract = TRUE;
        u32Ret = createArFile(pstrArchiveName, &afp, &paf);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = extractFromArchive(
            paf, pu8Buffer, MAX_ARCHIVE_BUFFER_LEN, pParam);
    }

    if (pu8Buffer != NULL)
        xfree((void **)&pu8Buffer);

    if (paf != NULL)
        destroyArFile(&paf);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

