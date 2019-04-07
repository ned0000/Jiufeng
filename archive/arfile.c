/**
 *  @file arfile.c
 *
 *  @brief The archive file
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "errcode.h"
#include "xmalloc.h"
#include "arfile.h"
#include "files.h"

/* --- private data/data structure section --------------------------------- */
#define IAF_BUF_LEN  65536

typedef struct
{
    boolean_t iaf_bExtract;
    u8 iaf_u8Reserved[7];
    jf_filestream_t * iaf_pjfArchive;
    u8 * iaf_pu8Buffer;
    olsize_t iaf_sBufLen;
    olsize_t iaf_sOffset;
    olsize_t iaf_sTotalLen;
} internal_ar_file_t;

/* --- private routine section---------------------------------------------- */
static u32 _flushArBuffer(internal_ar_file_t * piaf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_filestream_writen(
        piaf->iaf_pjfArchive, piaf->iaf_pu8Buffer, piaf->iaf_sOffset);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_filestream_flush(piaf->iaf_pjfArchive);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piaf->iaf_sOffset = 0;
    }

    return u32Ret;
}

static u32 _fillArBuffer(internal_ar_file_t * piaf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    piaf->iaf_sTotalLen = piaf->iaf_sBufLen;
    u32Ret = jf_filestream_readn(
        piaf->iaf_pjfArchive, piaf->iaf_pu8Buffer, &(piaf->iaf_sTotalLen));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piaf->iaf_sOffset = 0;
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 createArFile(
    olchar_t * pstrArchiveName, ar_file_param_t * pafp, ar_file_t ** ppaf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ar_file_t * piaf;

    u32Ret = jf_mem_alloc((void **)&piaf, sizeof(internal_ar_file_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        memset(piaf, 0, sizeof(internal_ar_file_t));

        piaf->iaf_bExtract = pafp->afp_bExtract;
        if (piaf->iaf_bExtract)
            u32Ret = jf_filestream_open(
                pstrArchiveName, "rb", &(piaf->iaf_pjfArchive));
        else
            u32Ret = jf_filestream_open(
                pstrArchiveName, "wb", &(piaf->iaf_pjfArchive));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        piaf->iaf_sBufLen = IAF_BUF_LEN;
        u32Ret = jf_mem_alloc((void **)&(piaf->iaf_pu8Buffer), IAF_BUF_LEN);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {

    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppaf = piaf;
    else
        destroyArFile((void **)&piaf);

    return u32Ret;
}

u32 destroyArFile(ar_file_t ** ppaf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ar_file_t * piaf = (internal_ar_file_t *) *ppaf;

    if (! piaf->iaf_bExtract)
    {
        if (piaf->iaf_sOffset != 0)
            _flushArBuffer(piaf);
    }

    if (piaf->iaf_pjfArchive != NULL)
        jf_filestream_close(&(piaf->iaf_pjfArchive));

    if (piaf->iaf_pu8Buffer != NULL)
        jf_mem_free((void **)&(piaf->iaf_pu8Buffer));

    jf_mem_free(ppaf);

    return u32Ret;
}

u32 readArFile(ar_file_t * paf, u8 * pu8Buffer, olsize_t * psBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ar_file_t * piaf = (internal_ar_file_t *)paf;
    u8 * pu8Start;
    olsize_t sLen, sCopy;

    assert((piaf->iaf_bExtract) && (pu8Buffer != NULL) && (*psBuf > 0));

    pu8Start = pu8Buffer;
    sLen = *psBuf;
    while ((sLen > 0) && (u32Ret == JF_ERR_NO_ERROR))
    {
        sCopy = piaf->iaf_sTotalLen - piaf->iaf_sOffset;
        if (sCopy < sLen)
        {
            if (sCopy != 0)
            {
                ol_memcpy(
                    pu8Start, piaf->iaf_pu8Buffer + piaf->iaf_sOffset, sCopy);
                pu8Start += sCopy;
                sLen -= sCopy;
                piaf->iaf_sOffset += sCopy;
            }

            u32Ret = _fillArBuffer(piaf);
        }
        else
        {
            ol_memcpy(pu8Start, piaf->iaf_pu8Buffer + piaf->iaf_sOffset, sLen);
            piaf->iaf_sOffset += sLen;
            sLen = 0;
        }
    }

    *psBuf = *psBuf - sLen;

    return u32Ret;
}

u32 writeArFile(ar_file_t * paf, u8 * pu8Buffer, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ar_file_t * piaf = (internal_ar_file_t *)paf;
    olsize_t sAll, sCopy;
    u8 * pu8Start;

    assert((! piaf->iaf_bExtract) && (pu8Buffer != NULL) && (sBuf > 0));

    pu8Start = pu8Buffer;
    sAll = sBuf;
    while ((sAll > 0) && (u32Ret == JF_ERR_NO_ERROR))
    {
        sCopy = piaf->iaf_sBufLen - piaf->iaf_sOffset;
        if (sCopy < sAll)
        {
            if (sCopy != 0)
            {
                ol_memcpy(
                    piaf->iaf_pu8Buffer + piaf->iaf_sOffset, pu8Start, sCopy);
                pu8Start += sCopy;
                sAll -= sCopy;
                piaf->iaf_sOffset += sCopy;
            }

            u32Ret = _flushArBuffer(piaf);
        }
        else
        {
            ol_memcpy(piaf->iaf_pu8Buffer + piaf->iaf_sOffset, pu8Start, sAll);
            piaf->iaf_sOffset += sAll;
            sAll = 0;
        }
    }

    return u32Ret;
}

u32 seekArFile(ar_file_t * paf, long offset, olint_t whence)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_ar_file_t * piaf = (internal_ar_file_t *)paf;
    u32 u32Offset, u32Ignore;

    assert((offset > 0) && (whence == SEEK_CUR));

    u32Offset = (u32)offset;
    while ((u32Offset > 0) && (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Ignore = piaf->iaf_sTotalLen - piaf->iaf_sOffset;
        if (u32Ignore < u32Offset)
        {
            u32Offset -= u32Ignore;
            piaf->iaf_sOffset += u32Ignore;

            u32Ret = _fillArBuffer(piaf);
        }
        else
        {
            piaf->iaf_sOffset += u32Offset;
            u32Offset = 0;
        }
    }

    return u32Ret;
}

boolean_t isEndOfArFile(ar_file_t * paf)
{
    boolean_t bRet = FALSE;
    internal_ar_file_t * piaf = (internal_ar_file_t *)paf;

    if ((jf_filestream_isEndOfFile(piaf->iaf_pjfArchive)) &&
        (piaf->iaf_sTotalLen == piaf->iaf_sOffset))
        bRet = TRUE;

    return bRet;
}

/*---------------------------------------------------------------------------*/


