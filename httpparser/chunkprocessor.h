/**
 *  @file chunkprocessor.h
 *
 *  @brief Header file defines the interface of chunk processor.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_HTTPPARSER_CHUNK_PROCESSOR_H
#define JIUFENG_HTTPPARSER_CHUNK_PROCESSOR_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_httpparser.h"

/* --- constant definitions --------------------------------------------------------------------- */



/* --- data structures -------------------------------------------------------------------------- */

/** Define the http chunk processor data type.
 */
typedef void  jf_httpparser_chunk_processor_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Destroy the chunk processor.
 *
 *  @param ppProcessor [in/out] The chunk processor to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_httpparser_destroyChunkProcessor(
    jf_httpparser_chunk_processor_t ** ppProcessor);

/** Create the chunk processor for chunked data in http body.
 *
 *  @param ppProcessor [out] The chunk processor to be created.
 *  @param u32MallocSize [in] The memory size to be allocated for the chunked data.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_httpparser_createChunkProcessor(
    jf_httpparser_chunk_processor_t ** ppProcessor, u32 u32MallocSize);

/** Process the chunked data.
 *
 *  @note
 *  -# Check the body pointer in the packet header, all chunk data are processed if it's not NULL,
 *   otherwise there are pending data.
 *
 *  @param pProcessor [in] The chunk processor.
 *  @param pjhph [in] The http packet header.
 *  @param buffer [in] The receive buffer.
 *  @param psBeginPointer [out] The buffer start pointer.
 *  @param endPointer [in] The length of the buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_httpparser_processChunk(
    jf_httpparser_chunk_processor_t * pProcessor, jf_httpparser_packet_header_t * pjhph,
    u8 * buffer, olsize_t * psBeginPointer, olsize_t endPointer);

#endif /*JIUFENG_HTTPPARSER_CHUNK_PROCESSOR_H*/

/*------------------------------------------------------------------------------------------------*/
