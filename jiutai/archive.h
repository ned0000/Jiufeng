/**
 *  @file archive.h
 *
 *  @brief archive library header file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_ARCHIVE_H
#define JIUFENG_ARCHIVE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "errcode.h"
#include "bases.h"

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

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

typedef struct
{
    boolean_t jacp_bCompress;
    boolean_t jacp_bFastest;
    boolean_t jacp_bBestCompress;
    boolean_t jacp_bVerbose;
    u8 jacp_u8Reserved[4];
    u8 jacp_u8Reserved2[28];
} jf_archive_create_param_t;

typedef struct
{
    boolean_t jaep_bListArchive;
    boolean_t jaep_bVerbose;
    u8 jaep_u8Reserved[6];
    u8 jaep_u8Reserved2[28];
} jf_archive_extract_param_t;

ARCHIVEAPI u32 ARCHIVECALL jf_archive_create(
    jf_linklist_t * pMemberFile,
    olchar_t * pstrArchiveName, jf_archive_create_param_t * pParam);

ARCHIVEAPI u32 ARCHIVECALL jf_archive_extract(
    olchar_t * pstrArchiveName, jf_archive_extract_param_t * pParam);

#endif /*JIUFENG_ARCHIVE_H*/

/*---------------------------------------------------------------------------*/


