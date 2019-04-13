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

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** The data vector entry structure
 */
typedef struct
{
    u8 * jde_pu8Buf;   /**< buffer */
    olsize_t jde_sBuf; /**< the size of the buffer*/
    olsize_t jde_sOffset; /**< the size of the data in this buffer*/
} jf_datavec_entry_t;

/** The data vector structure
 */
typedef struct
{
    u16 jd_u16MaxEntry; /**< max number of entries allocated*/
    u16 jd_u16CurEntry; /**< tatal number of entryies containing data*/
    u32 jd_u32Reserved8[3];
    jf_datavec_entry_t * jd_pjdeEntry;
} jf_datavec_t;

/* --- functional routines ---------------------------------------------------------------------- */

static inline void jf_datavec_init(jf_datavec_t * vec)
{
    olint_t i;
    jf_datavec_entry_t * entry;

    vec->jd_u16CurEntry = 0;

    for (i = 0; i < vec->jd_u16MaxEntry; i ++)
    {
        entry = &vec->jd_pjdeEntry[i];
        entry->jde_sOffset = 0;
    }
}

static inline void jf_datavec_set(jf_datavec_t * vec, olsize_t offset)
{
    olint_t i;
    jf_datavec_entry_t * entry;

    vec->jd_u16CurEntry = 0;

    for (i = 0; i < vec->jd_u16MaxEntry; i ++)
    {
        entry = &vec->jd_pjdeEntry[i];
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

static inline jf_datavec_entry_t * jf_datavec_locate(
    jf_datavec_t * vec, olsize_t offset, olsize_t * entryoffset)
{
    olint_t i;
    jf_datavec_entry_t * entry = NULL;

    for (i = 0; i < vec->jd_u16CurEntry; i ++)
    {
        entry = &vec->jd_pjdeEntry[i];
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

static inline olsize_t jf_datavec_copyData(
    jf_datavec_t * vec, u8 * data, olsize_t size)
{
    jf_datavec_entry_t * entry;
    olsize_t copy, offset = 0;

    if (vec->jd_u16CurEntry == vec->jd_u16MaxEntry)
        return 0;

    while (size > 0)
    {
        entry = &vec->jd_pjdeEntry[vec->jd_u16CurEntry];

        copy = size;
        if (entry->jde_sOffset + copy > entry->jde_sBuf)
            copy = entry->jde_sBuf - entry->jde_sOffset;

        memcpy(entry->jde_pu8Buf + entry->jde_sOffset, data + offset, copy);
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

/*return true is this is the last entry containing data in vector*/
static inline boolean_t jf_datavec_isLastEntry(
    jf_datavec_t * vec, jf_datavec_entry_t * entry)
{
    if ((entry->jde_sOffset != entry->jde_sBuf) ||
        (entry - vec->jd_pjdeEntry == vec->jd_u16MaxEntry - 1))
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
        entry = &vec->jd_pjdeEntry[index];

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
        entry = &vec->jd_pjdeEntry[index];
        start = 0;
        while (start < entry->jde_sOffset)
        {
            size = entry->jde_sOffset - start;
            if (size > sBuf)
                size = sBuf;

            entry2 = &newvec->jd_pjdeEntry[index2];
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


