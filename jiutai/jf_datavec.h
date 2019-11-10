/**
 *  @file jf_datavec.h
 *
 *  @brief data vector definition
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUTAI_DATAVEC_H
#define JIUTAI_DATAVEC_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_jiukun.h"

/* --- constant definitions --------------------------------------------------------------------- */

#define JF_DATAVEC_MAX_ENTRY    (8)

/* --- data structures -------------------------------------------------------------------------- */

/** The data vector entry structure
 */
typedef struct
{
    /** Buffer */
    u8 * jde_pu8Buf;
    /** The size of the buffer*/
    olsize_t jde_sBuf;
    /** The size of the data in this buffer*/
    olsize_t jde_sOffset;
} jf_datavec_entry_t;

/** The data vector structure
 */
typedef struct
{
    /** max number of entries allocated*/
    u16 jd_u16MaxEntry;
    /** tatal number of entryies containing data*/
    u16 jd_u16CurEntry;
    u32 jd_u32Reserved8[3];
    jf_datavec_entry_t jd_jdeEntry[JF_DATAVEC_MAX_ENTRY];
} jf_datavec_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Clone the data into data vector.
 *
 *  @param vec [in/out] the vector to clone
 *  @param ppu8Data [in] the data array to set to vector entries
 *  @param psData [in] the data size array to set to vector entries
 *  @param u16NumOfData [in] number of buffer in array
 *
 *  @return the error code
 */
static inline u32 jf_datavec_cloneData(
    jf_datavec_t * vec, u8 ** ppu8Data, olsize_t * psData, u16 u16NumOfData)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index = 0;
    jf_datavec_entry_t * entry = NULL;

    assert(u16NumOfData < JF_DATAVEC_MAX_ENTRY);

    ol_bzero(vec, sizeof(*vec));
    vec->jd_u16MaxEntry = JF_DATAVEC_MAX_ENTRY;
    vec->jd_u16CurEntry = u16NumOfData;

    for (u16Index = 0; (u16Index < u16NumOfData) && (u32Ret == JF_ERR_NO_ERROR); ++ u16Index)
    {
        entry = &vec->jd_jdeEntry[u16Index];
        entry->jde_sBuf = psData[u16Index];
        entry->jde_sOffset = entry->jde_sBuf;

        u32Ret = jf_jiukun_cloneMemory(
            (void **)&entry->jde_pu8Buf, ppu8Data[u16Index], psData[u16Index]);
    }

    return u32Ret;
}

/** Free the data in data vector.
 *
 *  @param vec [in] the vector to free
 *
 *  @return the error code
 */
static inline u32 jf_datavec_freeData(jf_datavec_t * vec)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u16 u16Index = 0;
    jf_datavec_entry_t * entry = NULL;

    for (u16Index = 0; u16Index < vec->jd_u16CurEntry; u16Index ++)
    {
        entry = &vec->jd_jdeEntry[u16Index];

        jf_jiukun_freeMemory((void **)&entry->jde_pu8Buf);
    }

    ol_bzero(vec, sizeof(*vec));

    return u32Ret;
}

/** Reinit the data vector, the current entry is set to 0 and offset in each entry is set to 0 also
 *
 *  @param vec [in] the vector to reinit
 *
 *  @return void 
 */
static inline void jf_datavec_reinit(jf_datavec_t * vec)
{
    olint_t i;
    jf_datavec_entry_t * entry;

    vec->jd_u16CurEntry = 0;

    for (i = 0; i < vec->jd_u16MaxEntry; i ++)
    {
        entry = &vec->jd_jdeEntry[i];
        entry->jde_sOffset = 0;
    }
}

/** Set data vector to offset. It will traverse the entries and set the offset in each entry
 *
 *  @param vec [in] the vector to set
 *  @param offset [in] the offset of data
 *
 *  @return void 
 */
static inline void jf_datavec_set(jf_datavec_t * vec, olsize_t offset)
{
    olint_t i;
    jf_datavec_entry_t * entry;

    vec->jd_u16CurEntry = 0;

    for (i = 0; i < vec->jd_u16MaxEntry; i ++)
    {
        entry = &vec->jd_jdeEntry[i];
        if (offset >= entry->jde_sBuf)
        {
            entry->jde_sOffset = entry->jde_sBuf;
            offset -= entry->jde_sBuf;
        }
        else
        {
            entry->jde_sOffset = offset;
            break;
        }
    }

    vec->jd_u16CurEntry = i;
}

/** Locate the entry with specified offset
 *
 *  @param vec [in] the vector to locate
 *  @param offset [in] the offset of data
 *  @param entryoffset [out] the offset in the entry
 *
 *  @return the data entry
 */
static inline jf_datavec_entry_t * jf_datavec_locate(
    jf_datavec_t * vec, olsize_t offset, olsize_t * entryoffset)
{
    olint_t i;
    jf_datavec_entry_t * entry = NULL;

    for (i = 0; i < vec->jd_u16CurEntry; i ++)
    {
        entry = &vec->jd_jdeEntry[i];
        if (offset >= entry->jde_sOffset)
        {
            offset -= entry->jde_sBuf;
            continue;
        }
        else
        {
            *entryoffset = offset;
            break;
        }
    }

    if (i == vec->jd_u16CurEntry)
        entry = NULL;
 
    return entry;
}

/** Copy the data to vector.
 *
 *  @param vec [in] the vector to set
 *  @param data [in] the data to be copied
 *  @param size [in] the size of the data
 *
 *  @return the size actually copied
 */
static inline olsize_t jf_datavec_copyData(jf_datavec_t * vec, u8 * data, olsize_t size)
{
    jf_datavec_entry_t * entry;
    olsize_t copy, offset = 0;

    if (vec->jd_u16CurEntry == vec->jd_u16MaxEntry)
        return 0;

    while (size > 0)
    {
        entry = &vec->jd_jdeEntry[vec->jd_u16CurEntry];

        copy = size;
        if (entry->jde_sOffset + copy > entry->jde_sBuf)
            copy = entry->jde_sBuf - entry->jde_sOffset;

        ol_memcpy(entry->jde_pu8Buf + entry->jde_sOffset, data + offset, copy);
        offset += copy;
        entry->jde_sOffset += copy;
        size -= copy;
        if (entry->jde_sOffset == entry->jde_sBuf)
            vec->jd_u16CurEntry ++;

        if (vec->jd_u16CurEntry == vec->jd_u16MaxEntry)
            break;
    }

    return offset;
}

/** Check if if the entry is the last entry containing data in vector
 *
 *  @param vec [in] the vector to check
 *  @param entry [in] the entry
 *
 *  @return the status
 *  @retval TRUE if it's the last entry
 *  @retval FALSE if it's not the last entry
 */
static inline boolean_t jf_datavec_isLastEntry(jf_datavec_t * vec, jf_datavec_entry_t * entry)
{
    if ((entry->jde_sOffset != entry->jde_sBuf) ||
        (entry - vec->jd_jdeEntry == vec->jd_u16MaxEntry - 1))
        return TRUE;

    return FALSE;
}

/*base on the new buffer size, return the number of entry*/
static inline u16 jf_datavec_getMaxEntry(jf_datavec_t * vec, olsize_t sBuf)
{
    u16 index;
    jf_datavec_entry_t * entry;
    u16 u16Entry = 0;

    for (index = 0; index < vec->jd_u16CurEntry; index ++)
    {
        entry = &vec->jd_jdeEntry[index];

        u16Entry += (entry->jde_sOffset + sBuf - 1) / sBuf;
    }

    return u16Entry;
}

static inline void jf_datavec_convert(
    jf_datavec_t * vec, olsize_t sBuf, jf_datavec_t * newvec)
{
    u16 index, index2;
    jf_datavec_entry_t * entry, *entry2;
    olsize_t start, size;

    for (index = 0, index2 = 0; index < vec->jd_u16CurEntry; index ++)
    {
        entry = &vec->jd_jdeEntry[index];
        start = 0;
        while (start < entry->jde_sOffset)
        {
            size = entry->jde_sOffset - start;
            if (size > sBuf)
                size = sBuf;

            entry2 = &newvec->jd_jdeEntry[index2];
            entry2->jde_pu8Buf = entry->jde_pu8Buf + start;
            entry2->jde_sBuf = size;
            entry2->jde_sOffset = size;

            start += size;
            index2 ++;
        }
    }

    newvec->jd_u16CurEntry = index2;
}

#endif /*JIUTAI_DATAVEC_H*/

/*------------------------------------------------------------------------------------------------*/


