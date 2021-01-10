/**
 *  @file jf_datavec.h
 *
 *  @brief Header file defines data structures and routines of data vector.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# All APIs defined in this file are inline functions.
 *  -# Link with jf_jiukun library for memory allocation.
 */

#ifndef JIUTAI_DATAVEC_H
#define JIUTAI_DATAVEC_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_jiukun.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Maximum entry in data vector.
 */
#define JF_DATAVEC_MAX_ENTRY               (8)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the data vector entry data type.
 */
typedef struct
{
    /**The data.*/
    u8 * jde_pu8Data;
    /**The size of the data.*/
    olsize_t jde_sData;
    /**The size of the data in this buffer.*/
    olsize_t jde_sOffset;
} jf_datavec_entry_t;

/** Define the data vector data type.
 */
typedef struct
{
    /**Max number of entries allocated.*/
    u16 jd_u16MaxEntry;
    /**Total number of entryies containing data.*/
    u16 jd_u16NumOfEntry;
    u32 jd_u32Reserved8[3];
    /**The data vector entry.*/
    jf_datavec_entry_t jd_jdeEntry[JF_DATAVEC_MAX_ENTRY];
} jf_datavec_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Initialize the data vector.
 *
 *  @param pVec [in/out] The vector to be initialized.
 *
 *  @return Void.
 */
static inline void jf_datavec_init(jf_datavec_t * pVec)
{
    ol_bzero(pVec, sizeof(*pVec));
    pVec->jd_u16MaxEntry = JF_DATAVEC_MAX_ENTRY;
}

/** Set the data into data vector.
 *
 *  @note
 *  -# No buffer is allocated, the data is used.
 *
 *  @param pVec [in/out] The vector to clone.
 *  @param ppu8Data [in] The data array to set to vector entries.
 *  @param psData [in] The data size array to set to vector entries.
 *  @param u16NumOfData [in] Number of buffer in array.
 *
 *  @return Void.
 */
static inline void jf_datavec_setData(
    jf_datavec_t * pVec, u8 ** ppu8Data, olsize_t * psData, u16 u16NumOfData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index = 0;
    jf_datavec_entry_t * entry = NULL;

    assert((u16NumOfData != 0) && (u16NumOfData < JF_DATAVEC_MAX_ENTRY));

    ol_bzero(pVec, sizeof(*pVec));
    pVec->jd_u16MaxEntry = JF_DATAVEC_MAX_ENTRY;
    pVec->jd_u16NumOfEntry = u16NumOfData;

    for (u16Index = 0; (u16Index < u16NumOfData) && (u32Ret == JF_ERR_NO_ERROR); ++ u16Index)
    {
        entry = &pVec->jd_jdeEntry[u16Index];
        entry->jde_pu8Data = ppu8Data[u16Index];
        entry->jde_sData = psData[u16Index];
        entry->jde_sOffset = entry->jde_sData;
    }
}

/** Clone the data into data vector.
 *
 *  @note
 *  -# Allocate buffer and copy the data to buffer.
 *
 *  @param pVec [in/out] The vector to clone.
 *  @param ppu8Data [in] The data array to set to vector entries.
 *  @param psData [in] The data size array to set to vector entries.
 *  @param u16NumOfData [in] Number of buffer in array.
 *
 *  @return The error code.
 */
static inline u32 jf_datavec_cloneData(
    jf_datavec_t * pVec, u8 ** ppu8Data, olsize_t * psData, u16 u16NumOfData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index = 0;
    jf_datavec_entry_t * entry = NULL;

    assert((u16NumOfData != 0) && (u16NumOfData < JF_DATAVEC_MAX_ENTRY));

    ol_bzero(pVec, sizeof(*pVec));
    pVec->jd_u16MaxEntry = JF_DATAVEC_MAX_ENTRY;

    for (u16Index = 0; (u16Index < u16NumOfData) && (u32Ret == JF_ERR_NO_ERROR); ++ u16Index)
    {
        entry = &pVec->jd_jdeEntry[u16Index];
        entry->jde_sData = psData[u16Index];
        entry->jde_sOffset = entry->jde_sData;

        u32Ret = jf_jiukun_cloneMemory(
            (void **)&entry->jde_pu8Data, ppu8Data[u16Index], psData[u16Index]);
    }

    pVec->jd_u16NumOfEntry = u16Index;

    return u32Ret;
}

/** Clone the source data vector into destination data vector.
 *
 *  @note
 *  -# Allocate buffer and copy the data to the new vector.
 *
 *  @param pSrcVec [in/out] The source vector.
 *  @param pDestVec [in] The destination vector.
 *
 *  @return The error code.
 */
static inline u32 jf_datavec_cloneVec(jf_datavec_t * pDestVec, const jf_datavec_t * pSrcVec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index = 0;
    const jf_datavec_entry_t * srcEntry = NULL;
    jf_datavec_entry_t * destEntry = NULL;

    assert((pSrcVec != NULL) && (pDestVec != NULL));

    ol_bzero(pDestVec, sizeof(*pDestVec));
    pDestVec->jd_u16MaxEntry = JF_DATAVEC_MAX_ENTRY;

    for (u16Index = 0;
         (u16Index < pSrcVec->jd_u16NumOfEntry) && (u32Ret == JF_ERR_NO_ERROR); ++ u16Index)
    {
        srcEntry = &pSrcVec->jd_jdeEntry[u16Index];
        destEntry = &pDestVec->jd_jdeEntry[u16Index];

        destEntry->jde_sData = srcEntry->jde_sData;
        destEntry->jde_sOffset = srcEntry->jde_sOffset;

        u32Ret = jf_jiukun_cloneMemory(
            (void **)&destEntry->jde_pu8Data, srcEntry->jde_pu8Data, srcEntry->jde_sData);
    }

    pDestVec->jd_u16NumOfEntry = pSrcVec->jd_u16NumOfEntry;

    return u32Ret;
}

/** Free the data in data vector.
 *
 *  @param pVec [in] The vector to free.
 *
 *  @return The error code.
 */
static inline u32 jf_datavec_freeData(jf_datavec_t * pVec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index = 0;
    jf_datavec_entry_t * entry = NULL;

    for (u16Index = 0; u16Index < pVec->jd_u16NumOfEntry; u16Index ++)
    {
        entry = &pVec->jd_jdeEntry[u16Index];

        jf_jiukun_freeMemory((void **)&entry->jde_pu8Data);
    }

    ol_bzero(pVec, sizeof(*pVec));

    return u32Ret;
}

/** Reinitialize the data vector.
 *
 *  @note
 *  -# The current entry is set to 0 and offset in each entry is set to 0 also.
 *
 *  @param pVec [in] The vector to reinit.
 *
 *  @return Void.
 */
static inline void jf_datavec_reinit(jf_datavec_t * pVec)
{
    olint_t i = 0;
    jf_datavec_entry_t * entry = NULL;

    pVec->jd_u16NumOfEntry = 0;

    for (i = 0; i < pVec->jd_u16MaxEntry; i ++)
    {
        entry = &pVec->jd_jdeEntry[i];
        entry->jde_sOffset = 0;
    }
}

/** Set data vector to offset. It will traverse the entries and set the offset in each entry.
 *
 *  @param pVec [in] The vector to set.
 *  @param offset [in] The offset of data.
 *
 *  @return Void.
 */
static inline void jf_datavec_set(jf_datavec_t * pVec, olsize_t offset)
{
    olint_t i = 0;
    jf_datavec_entry_t * entry = NULL;

    pVec->jd_u16NumOfEntry = 0;

    for (i = 0; i < pVec->jd_u16MaxEntry; i ++)
    {
        entry = &pVec->jd_jdeEntry[i];
        if (offset >= entry->jde_sData)
        {
            entry->jde_sOffset = entry->jde_sData;
            offset -= entry->jde_sData;
        }
        else
        {
            entry->jde_sOffset = offset;
            break;
        }
    }

    pVec->jd_u16NumOfEntry = i;
}

/** Locate the entry with specified offset.
 *
 *  @param pVec [in] The vector to locate.
 *  @param offset [in] The offset of data.
 *  @param entryoffset [out] The offset in the entry.
 *
 *  @return The data entry.
 */
static inline jf_datavec_entry_t * jf_datavec_locate(
    jf_datavec_t * pVec, olsize_t offset, olsize_t * entryoffset)
{
    olint_t i = 0;
    jf_datavec_entry_t * entry = NULL;

    for (i = 0; i < pVec->jd_u16NumOfEntry; i ++)
    {
        entry = &pVec->jd_jdeEntry[i];
        if (offset >= entry->jde_sOffset)
        {
            offset -= entry->jde_sData;
            continue;
        }
        else
        {
            *entryoffset = offset;
            break;
        }
    }

    if (i == pVec->jd_u16NumOfEntry)
        entry = NULL;
 
    return entry;
}

/** Copy the data to vector.
 *
 *  @param pVec [in] The vector to set
 *  @param data [in] The data to be copied
 *  @param size [in] The size of the data
 *
 *  @return The size actually copied
 */
static inline olsize_t jf_datavec_copyData(jf_datavec_t * pVec, u8 * data, olsize_t size)
{
    jf_datavec_entry_t * entry = NULL;
    olsize_t copy = 0, offset = 0;

    if (pVec->jd_u16NumOfEntry == pVec->jd_u16MaxEntry)
        return 0;

    while (size > 0)
    {
        entry = &pVec->jd_jdeEntry[pVec->jd_u16NumOfEntry];

        copy = size;
        if (entry->jde_sOffset + copy > entry->jde_sData)
            copy = entry->jde_sData - entry->jde_sOffset;

        ol_memcpy(entry->jde_pu8Data + entry->jde_sOffset, data + offset, copy);
        offset += copy;
        entry->jde_sOffset += copy;
        size -= copy;
        if (entry->jde_sOffset == entry->jde_sData)
            pVec->jd_u16NumOfEntry ++;

        if (pVec->jd_u16NumOfEntry == pVec->jd_u16MaxEntry)
            break;
    }

    return offset;
}

/** Check if if the entry is the last entry containing data in vector.
 *
 *  @param pVec [in] The vector to check.
 *  @param entry [in] The entry.
 *
 *  @return The status.
 *  @retval TRUE If it's the last entry.
 *  @retval FALSE If it's not the last entry.
 */
static inline boolean_t jf_datavec_isLastEntry(jf_datavec_t * pVec, jf_datavec_entry_t * entry)
{
    if ((entry->jde_sOffset != entry->jde_sData) ||
        (entry - pVec->jd_jdeEntry == pVec->jd_u16MaxEntry - 1))
        return TRUE;

    return FALSE;
}

/** Base on the new buffer size, return the number of entry.
 *
 *  @param pVec [in] The vector to check.
 *  @param sBuf [in] Size of buffer.
 *
 *  @return Number of entry required for the buffer.
 */
static inline u16 jf_datavec_getMaxEntry(jf_datavec_t * pVec, olsize_t sBuf)
{
    u16 index = 0;
    jf_datavec_entry_t * entry = NULL;
    u16 u16Entry = 0;

    for (index = 0; index < pVec->jd_u16NumOfEntry; index ++)
    {
        entry = &pVec->jd_jdeEntry[index];

        u16Entry += (entry->jde_sOffset + sBuf - 1) / sBuf;
    }

    return u16Entry;
}

/** Convert the data vector to another new data vector.
 *
 *  @param pVec [in] The source data vector.
 *  @param sBuf [in] Size of buffer for the new data vector.
 *  @param pNewVec [in] The new data vector.
 *
 *  @return Void.
 */
static inline void jf_datavec_convert(
    jf_datavec_t * pVec, olsize_t sBuf, jf_datavec_t * pNewVec)
{
    u16 index = 0, index2 = 0;
    jf_datavec_entry_t * entry = NULL, * entry2 = NULL;
    olsize_t start = 0, size = 0;

    for (index = 0, index2 = 0; index < pVec->jd_u16NumOfEntry; index ++)
    {
        entry = &pVec->jd_jdeEntry[index];
        start = 0;
        while (start < entry->jde_sOffset)
        {
            size = entry->jde_sOffset - start;
            if (size > sBuf)
                size = sBuf;

            entry2 = &pNewVec->jd_jdeEntry[index2];
            entry2->jde_pu8Data = entry->jde_pu8Data + start;
            entry2->jde_sData = size;
            entry2->jde_sOffset = size;

            start += size;
            index2 ++;
        }
    }

    pNewVec->jd_u16NumOfEntry = index2;
}

#endif /*JIUTAI_DATAVEC_H*/

/*------------------------------------------------------------------------------------------------*/
