/**
 *  @file jf_dir.h
 *
 *  @brief Provide common routines to manipulate directory.
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_files library
 */

#ifndef JIUFENG_DIR_H
#define JIUFENG_DIR_H

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

#if defined(LINUX)
    #include <dirent.h>
    #include <sys/file.h>
#elif defined(WINDOWS)
    #include <io.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_datavec.h"
#include "jf_file.h"

/* --- constant definitions ------------------------------------------------ */

#if defined(LINUX)
    typedef DIR      jf_dir_t;
#elif defined(WINDOWS)
    typedef void     jf_dir_t;
#endif

#define JF_DIR_DEFAULT_CREATE_MODE     (0755)


/* --- data structures ----------------------------------------------------- */

typedef struct
{
    olchar_t jde_strName[JF_LIMIT_MAX_PATH_LEN];
    olsize_t jde_sName;
    u8 jde_u8Reserved[60];
} jf_dir_entry_t;

/* --- functional routines ------------------------------------------------- */

/*'mode' is for Linux only, it specifies the permission to use*/
FILESAPI u32 FILESCALL jf_dir_create(
    const olchar_t * pstrDirName, jf_file_mode_t mode);

FILESAPI u32 FILESCALL jf_dir_remove(const olchar_t * pstrDirName);

FILESAPI u32 FILESCALL jf_dir_open(
    const olchar_t * pstrDirName, jf_dir_t ** ppDir);

FILESAPI u32 FILESCALL jf_dir_close(jf_dir_t ** ppDir);

FILESAPI u32 FILESCALL jf_dir_getFirstDirEntry(
    jf_dir_t * pDir, jf_dir_entry_t * pEntry);

FILESAPI u32 FILESCALL jf_dir_getNextDirEntry(
    jf_dir_t * pDir, jf_dir_entry_t * pEntry);

typedef u32 (* jf_dir_fnHandleFile_t)(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, void * pArg);

FILESAPI u32 FILESCALL jf_dir_traversal(
    const olchar_t * pstrDirName, jf_dir_fnHandleFile_t fnHandleFile, void * pArg);

FILESAPI u32 FILESCALL jf_dir_parse(
    const olchar_t * pstrDirName, jf_dir_fnHandleFile_t fnHandleFile, void * pArg);

/*true, the entry is ignored*/
typedef boolean_t (* jf_dir_fnFilterDirEntry_t)(jf_dir_entry_t * entry);
typedef olint_t (* jf_dir_fnCompareDirEntry_t)(const void * a, const void * b);

FILESAPI u32 FILESCALL jf_dir_scan(
    const olchar_t * pstrDirName, jf_dir_entry_t * entry, olint_t * numofentry,
    jf_dir_fnFilterDirEntry_t fnFilter, jf_dir_fnCompareDirEntry_t fnCompare);

FILESAPI olint_t FILESCALL jf_dir_compareDirEntry(const void * a, const void * b);


#endif /*JIUFENG_DIR_H*/

/*---------------------------------------------------------------------------*/

