/**
 *  @file filestream.c
 *
 *  @brief Implementation file of file stream API.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <errno.h>

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_filestream.h"
#include "jf_err.h"
#include "jf_string.h"

#include "common.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

static u32 _writeVec(
    jf_filestream_t * pjf, olsize_t vecoffset, jf_datavec_t * pjdData, olsize_t * psData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t total = * psData, ndata = 0, size = 0;
    jf_datavec_entry_t * entry = NULL;
    olint_t index = 0;

    while ((total > 0) && (u32Ret == JF_ERR_NO_ERROR) && (index <= pjdData->jd_u16NumOfEntry))
    {
        entry = &pjdData->jd_jdeEntry[index ++];

        /*No data in this entry.*/
        if (entry->jde_sOffset == 0)
            break;

        /*Test if the offset is reached.*/
        if (vecoffset >= entry->jde_sOffset)
        {
            vecoffset -= entry->jde_sOffset;
            continue;
        }

        /*Calculate the size we can write.*/
        size = entry->jde_sOffset - vecoffset;
        if (size > total)
            size = total;

        /*Write data.*/
        u32Ret = jf_filestream_writen(pjf, entry->jde_pu8Data + vecoffset, size);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ndata += size;
            total -= size;
            vecoffset = 0;
        }
    }

    *psData = ndata;

    return u32Ret;
}

static u32 _readVec(jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t * psData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t total = *psData, ndata = 0, size = 0;
    jf_datavec_entry_t * entry = NULL;

    while ((total > 0) && (! jf_filestream_isEndOfFile(pjf)) && (u32Ret == JF_ERR_NO_ERROR))
    {
        entry = &pjdData->jd_jdeEntry[pjdData->jd_u16NumOfEntry];

        /*Calculate the size we can read.*/
        size = entry->jde_sData - entry->jde_sOffset;
        if (size > total)
            size = total;

        /*Read data from the file stream.*/
        u32Ret = jf_filestream_readn(pjf, entry->jde_pu8Data + entry->jde_sOffset, &size);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ndata += size;
            total -= size;

            entry->jde_sOffset += size;

            /*Move to the next entry if the entry is full.*/
            if (entry->jde_sOffset == entry->jde_sData)
            {
                pjdData->jd_u16NumOfEntry ++;
                if (pjdData->jd_u16NumOfEntry == pjdData->jd_u16MaxEntry)
                    break;
            }
        }
    }

    *psData = ndata;

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_filestream_open(
    const olchar_t * pstrFilename, const olchar_t * mode, jf_filestream_t ** ppjf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pstrFilename != NULL) && (mode != NULL) && (ppjf != NULL));

    *ppjf = fopen(pstrFilename, mode);

    if (*ppjf == NULL)
        u32Ret = JF_ERR_FAIL_OPEN_FILE;

    return u32Ret;
}

u32 jf_filestream_flush(jf_filestream_t * pjf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet = 0;

    nRet = fflush(pjf);

    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_FLUSH_FILE;

    return u32Ret;
}

olsize_t jf_filestream_printf(jf_filestream_t * pjf, const olchar_t * format, ...)
{
    olsize_t sRet = 0;
    va_list vlParam;

    va_start(vlParam, format);
    sRet = vfprintf(pjf, format, vlParam);
    va_end(vlParam);

    return sRet;
}

u32 jf_filestream_seek(jf_filestream_t * pjf, long offset, olint_t whence)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t ret = 0;

    assert(pjf != NULL);

    ret = fseek(pjf, offset, whence);

    if (ret != 0)
        u32Ret = JF_ERR_FAIL_SEEK_FILE;

    return u32Ret;
}

u32 jf_filestream_close(jf_filestream_t ** pjf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    assert((pjf != NULL) && (*pjf != NULL));

    fclose(*pjf);
    *pjf = NULL;

    return u32Ret;
}

boolean_t jf_filestream_isEndOfFile(jf_filestream_t * pjf)
{
    boolean_t bRet = FALSE;
    olint_t ret = 0;

    ret = feof(pjf);

    if (ret != 0)
        bRet = TRUE;

    return bRet;
}

u32 jf_filestream_readn(jf_filestream_t * pjf, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t nread = 0;

    assert((pjf != NULL) && (pBuffer != NULL) && (*psRead > 0));

    nread = fread(pBuffer, 1, *psRead, pjf);
    if (nread != *psRead)
    {
        /*Test if end of file.*/
        if (! feof(pjf))
        {
            /*It's error if expected data is not read and not end of file.*/
            u32Ret = JF_ERR_FAIL_READ_FILE;
        }
        else
        {
            /*Read exactly 0 byte.*/
            if (nread == 0)
                u32Ret = JF_ERR_END_OF_FILE;
        }
    }

    *psRead = nread;

    return u32Ret;
}

u32 jf_filestream_readVec(jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Reinitialize the data vector.*/
    jf_datavec_reinit(pjdData);

    /*Read vector.*/
    u32Ret = _readVec(pjf, pjdData, psRead);

    return u32Ret;
}

u32 jf_filestream_writeVec(jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sWrite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size = sWrite;

    /*Write data to vector.*/
    u32Ret = _writeVec(pjf, 0, pjdData, &size);

    return u32Ret;
}

u32 jf_filestream_readVecOffset(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sVecOffset, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Set offset to the data vector.*/
    jf_datavec_set(pjdData, sVecOffset);

    /*Read vector with the offset.*/
    u32Ret = _readVec(pjf, pjdData, psRead);

    return u32Ret;
}

u32 jf_filestream_writeVecOffset(
    jf_filestream_t * pjf, jf_datavec_t * pjdData, olsize_t sVecOffset, olsize_t sWrite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size = sWrite;

    u32Ret = _writeVec(pjf, sVecOffset, pjdData, &size);

    return u32Ret;
}

u32 jf_filestream_writen(jf_filestream_t * pjf, const void * pBuffer, olsize_t sWrite)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t nwritten = 0;

    assert((pjf != NULL) && (pBuffer != NULL) && (sWrite > 0));

    nwritten = fwrite(pBuffer, 1, sWrite, pjf);

    /*Check the actually written data size.*/
    if (nwritten != sWrite)
        u32Ret = JF_ERR_FAIL_WRITE_FILE;

    return u32Ret;
}

u32 jf_filestream_readLine(jf_filestream_t * pjf, void * pBuffer, olsize_t * psRead)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 n = 0, maxlen = 0;
    olsize_t nread = 0;
    olchar_t c = 0, * p = NULL;

    assert((pjf != NULL) && (pBuffer != NULL) && (*psRead > 0));

    maxlen = *psRead;
    p = pBuffer;
    for (n = 0; n < maxlen - 1; n++)
    {
        /*Read 1 character.*/
        nread = 1;
        u32Ret = jf_filestream_readn(pjf, (void *)&c, &nread);

        /*Save the character.*/
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            if (nread == 1) 
            {
                *p++ = c;
                if (c == JF_STRING_LINE_FEED_CHAR)
                    break;   /*Newline is stored.*/
            }
            /*No need to check the case (nread == 0), as JF_ERR_END_OF_FILE is returned.*/
        }

        if (u32Ret != JF_ERR_NO_ERROR)
            break;
    }

    /*Null terminated is always appended.*/
    *p = JF_STRING_NULL_CHAR;

    *psRead = n;

    return u32Ret;
}

u32 jf_filestream_copyFile(
    jf_filestream_t * pjfDest, const olchar_t * pstrSourceFile, u8 * pu8Buffer, olsize_t sBuf)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_filestream_t * sourcejf = NULL;
    olsize_t size = 0;

    /*Open the source file.*/
    u32Ret = jf_filestream_open(pstrSourceFile, "rb", &sourcejf);

    /*Copy the data.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Loop until end of source file is met.*/
        while ((u32Ret == JF_ERR_NO_ERROR) && (! feof(sourcejf)))
        {
            /*Read data from source file.*/
            size = sBuf;
            u32Ret = jf_filestream_readn(sourcejf, pu8Buffer, &size);

            /*Write data to destination file.*/
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                u32Ret = jf_filestream_writen(pjfDest, pu8Buffer, size);
            }
        }

        if (u32Ret == JF_ERR_END_OF_FILE)
            u32Ret = JF_ERR_NO_ERROR;

        jf_filestream_close(&sourcejf);
    }

    return u32Ret;
}

olint_t jf_filestream_getChar(jf_filestream_t * pjf)
{
    return fgetc(pjf);
}

/*------------------------------------------------------------------------------------------------*/
