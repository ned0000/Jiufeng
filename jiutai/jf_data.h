/**
 *  @file jf_datavec.h
 *
 *  @brief Header file defines the data vector.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUTAI_DATA_H
#define JIUTAI_DATA_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */


/* --- functional routines ---------------------------------------------------------------------- */

/** Copy the data into buffer.
 *
 *  @param pBuf [in] The buffer to hold data.
 *  @param sBuf [in] Size of buffer.
 *  @param sOffset [in] Offset the buffer for copying data from.
 *  @param pData [in] The data to be copied.
 *  @param sData [in] Size of data.
 *
 *  @return Number of data copied.
 */
static inline olsize_t jf_data_copyToBuffer(
    void * pBuf, olsize_t sBuf, olsize_t sOffset, void * pData, olsize_t sData)
{
    olsize_t copy = 0;

    if (sOffset < sBuf)
    {
        copy = sBuf - sOffset;
        if (copy > sData)
            copy = sData;

        ol_memcpy(pBuf + sOffset, pData, copy);
    }

    return copy;
}

#endif /*JIUTAI_DATA_H*/

/*------------------------------------------------------------------------------------------------*/
