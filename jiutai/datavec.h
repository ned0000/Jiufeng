/**
 *  @file datavec.h
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

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
typedef struct
{
    u8 * dve_pu8Buf; /*buffer*/
    olsize_t dve_sBuf; /*the size of the buffer*/
    olsize_t dve_sOffset; /*the size of the data in this buffer*/
} data_vec_entry_t;

typedef struct
{
    u16 dv_u16MaxEntry; /*max number of entries allocated*/
    u16 dv_u16CurEntry; /*tatal number of entryies containing data*/
    u32 dv_u32Reserved8[3];
    data_vec_entry_t * dv_pdveEntry;
} data_vec_t;

/* --- functional routines ------------------------------------------------- */
static inline void initDataVec(data_vec_t * vec)
{
    olint_t i;
    data_vec_entry_t * entry;

    vec->dv_u16CurEntry = 0;

    for (i = 0; i < vec->dv_u16MaxEntry; i ++)
    {
        entry = &vec->dv_pdveEntry[i];
        entry->dve_sOffset = 0;
    }
}

static inline void setDataVec(data_vec_t * vec, olsize_t offset)
{
    olint_t i;
    data_vec_entry_t * entry;

    vec->dv_u16CurEntry = 0;

    for (i = 0; i < vec->dv_u16MaxEntry; i ++)
    {
        entry = &vec->dv_pdveEntry[i];
        if (offset >= entry->dve_sBuf)
        {
            entry->dve_sOffset = entry->dve_sBuf;
            offset -= entry->dve_sBuf;
        }
        else
        {
            entry->dve_sOffset = offset;
            break;
        }
    }

    vec->dv_u16CurEntry = i;
}

static inline data_vec_entry_t * locateDataVec(
    data_vec_t * vec, olsize_t offset, olsize_t * entryoffset)
{
    olint_t i;
    data_vec_entry_t * entry = NULL;

    for (i = 0; i < vec->dv_u16CurEntry; i ++)
    {
        entry = &vec->dv_pdveEntry[i];
        if (offset >= entry->dve_sOffset)
        {
            offset -= entry->dve_sBuf;
            continue;
        }
        else
        {
            *entryoffset = offset;
            break;
        }
    }

    if (i == vec->dv_u16CurEntry)
        entry = NULL;
 
    return entry;
}

static inline olsize_t copyDataToVec(data_vec_t * vec, u8 * data, olsize_t size)
{
    data_vec_entry_t * entry;
    olsize_t copy, offset = 0;

    if (vec->dv_u16CurEntry == vec->dv_u16MaxEntry)
        return 0;

    while (size > 0)
    {
        entry = &vec->dv_pdveEntry[vec->dv_u16CurEntry];

        copy = size;
        if (entry->dve_sOffset + copy > entry->dve_sBuf)
            copy = entry->dve_sBuf - entry->dve_sOffset;

        memcpy(entry->dve_pu8Buf + entry->dve_sOffset, data + offset, copy);
        offset += copy;
        entry->dve_sOffset += copy;
        size -= copy;
        if (entry->dve_sOffset == entry->dve_sBuf)
            vec->dv_u16CurEntry ++;

        if (vec->dv_u16CurEntry == vec->dv_u16MaxEntry)
            break;
    }

    return offset;
}

/*return true is this is the last entry containing data in vector*/
static inline boolean_t isLastVecEntry(data_vec_t * vec,
    data_vec_entry_t * entry)
{
    if ((entry->dve_sOffset != entry->dve_sBuf) ||
        (entry - vec->dv_pdveEntry == vec->dv_u16MaxEntry - 1))
        return 1;

    return 0;
}

/*base on the new buffer size, return the new number of entry*/
static inline u16 newDataVecMaxEntry(data_vec_t * vec, olsize_t sBuf)
{
    u16 index;
    data_vec_entry_t * entry;
    u16 u16Entry = 0;

    for (index = 0; index < vec->dv_u16CurEntry; index ++)
    {
        entry = &vec->dv_pdveEntry[index];

        u16Entry += (entry->dve_sOffset + sBuf - 1) / sBuf;
    }

    return u16Entry;
}

static inline void convertDataVec(
    data_vec_t * vec, olsize_t sBuf, data_vec_t * newvec)
{
    u16 index, index2;
    data_vec_entry_t * entry, *entry2;
    olsize_t start, size;

    for (index = 0, index2 = 0; index < vec->dv_u16CurEntry; index ++)
    {
        entry = &vec->dv_pdveEntry[index];
        start = 0;
        while (start < entry->dve_sOffset)
        {
            size = entry->dve_sOffset - start;
            if (size > sBuf)
                size = sBuf;

            entry2 = &newvec->dv_pdveEntry[index2];
            entry2->dve_pu8Buf = entry->dve_pu8Buf + start;
            entry2->dve_sBuf = size;
            entry2->dve_sOffset = size;

            start += size;
            index2 ++;
        }
    }

    newvec->dv_u16CurEntry = index2;
}

#endif /*JIUTAI_DATAVEC_H*/

/*---------------------------------------------------------------------------*/


