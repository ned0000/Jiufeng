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
#include "olbasic.h"
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
    boolean_t cap_bCompress;
    boolean_t cap_bFastest;
    boolean_t cap_bBestCompress;
    boolean_t cap_bVerbose;
    u8 cap_u8Reserved[4];
    u8 cap_u8Reserved2[28];
} create_archive_param_t;

typedef struct
{
    boolean_t eap_bListArchive;
    boolean_t eap_bVerbose;
    u8 eap_u8Reserved[6];
    u8 eap_u8Reserved2[28];
} extract_archive_param_t;

ARCHIVEAPI u32 ARCHIVECALL createArchive(
    link_list_t * pMemberFile,
    olchar_t * pstrArchiveName, create_archive_param_t * pParam);

ARCHIVEAPI u32 ARCHIVECALL extractArchive(
    olchar_t * pstrArchiveName, extract_archive_param_t * pParam);

#endif /*JIUFENG_ARCHIVE_H*/

/*---------------------------------------------------------------------------*/


