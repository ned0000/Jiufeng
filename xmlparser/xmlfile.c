/**
 *  @file xmlfile.c
 *
 *  @brief Implementation file for routines related to XML file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_xmlparser.h"
#include "jf_jiukun.h"
#include "jf_filestream.h"

#include "xmlcommon.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */


/* --- public routine section ------------------------------------------------------------------- */

u32 jf_xmlparser_parseXmlFile(
    const olchar_t * pstrFilename, jf_xmlparser_xml_doc_t ** ppDoc)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    FILE * fp = NULL;
    olsize_t sSize;
    jf_file_stat_t filestat;
    olchar_t * pstrBuf = NULL;

    assert((pstrFilename != NULL) && (ppDoc != NULL));

    u32Ret = jf_file_getStat(pstrFilename, &filestat);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        sSize = (olsize_t)filestat.jfs_u64Size;
        u32Ret = jf_jiukun_allocMemory((void **)&pstrBuf, sSize);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_filestream_open(pstrFilename, "r", &fp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_filestream_readn(fp, pstrBuf, &sSize);
        if (sSize != (u32)filestat.jfs_u64Size)
            u32Ret = JF_ERR_INVALID_XML_FILE;
    }

    if (fp != NULL)
        jf_filestream_close(&fp);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_xmlparser_parseXmlDoc(pstrBuf, 0, sSize, ppDoc);
    }

    if (pstrBuf != NULL)
        jf_jiukun_freeMemory((void **)&pstrBuf);

    if (u32Ret != JF_ERR_NO_ERROR)
        tryGenXmlErrMsg(u32Ret, pstrFilename, ol_strlen(pstrFilename));

    return u32Ret;
}



/*------------------------------------------------------------------------------------------------*/

