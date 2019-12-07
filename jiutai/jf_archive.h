/**
 *  @file jf_archive.h
 *
 *  @brief Archive library header file, provides routines to create and extract archive.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_archive library.
 *  
 */

#ifndef JIUFENG_ARCHIVE_H
#define JIUFENG_ARCHIVE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_linklist.h"

#undef ARCHIVEAPI
#undef ARCHIVECALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_ARCHIVE_DLL)
        #define ARCHIVEAPI  __declspec(dllexport)
        #define ARCHIVECALL
    #else
        #define ARCHIVEAPI
        #define ARCHIVECALL __cdecl
    #endif
#else
    #define ARCHIVEAPI
    #define ARCHIVECALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

/** Parameters for creating archive with files.
 */
typedef struct
{
    /**Compress the data if it's TRUE.*/
    boolean_t jacp_bCompress;
    /**Use fastest compression method if it's TRUE.*/
    boolean_t jacp_bFastest;
    /**Use slowest compression and best compression method if it's TRUE.*/
    boolean_t jacp_bBestCompress;
    /**Print verbose debug information if it's TRUE.*/
    boolean_t jacp_bVerbose;
    u8 jacp_u8Reserved[4];
    u8 jacp_u8Reserved2[28];
} jf_archive_create_param_t;

/** Parameters for extracting files from archive.
 */
typedef struct
{
    /**List contents of archive and files are not created if it's TRUE.*/
    boolean_t jaep_bListArchive;
    /**Print verbose debug information if it's TRUE.*/
    boolean_t jaep_bVerbose;
    u8 jaep_u8Reserved[6];
    u8 jaep_u8Reserved2[28];
} jf_archive_extract_param_t;

/** Create an archive with member files.
 *
 *  @param pMemberFile [in] The linked list containing member files.
 *  @param pstrArchiveName [in] The archive name.
 *  @param pParam [in] Parameters for creating archive.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY Out of memory.
 *  @retval JF_ERR_FAIL_STAT_FILE cannot get file status.
 *
 */
ARCHIVEAPI u32 ARCHIVECALL jf_archive_create(
    jf_linklist_t * pMemberFile, olchar_t * pstrArchiveName, jf_archive_create_param_t * pParam);

/** Extract member files from an archive.
 *
 *  @param pstrArchiveName [in] The archive name.
 *  @param pParam [in] Parameters for extracting archive.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY Out of memory.
 *  @retval JF_ERR_FAIL_STAT_FILE cannot get file status.
 *
 */
ARCHIVEAPI u32 ARCHIVECALL jf_archive_extract(
    olchar_t * pstrArchiveName, jf_archive_extract_param_t * pParam);

#endif /*JIUFENG_ARCHIVE_H*/

/*------------------------------------------------------------------------------------------------*/


