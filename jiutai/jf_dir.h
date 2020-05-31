/**
 *  @file jf_dir.h
 *
 *  @brief Header file declares common routines to manipulate directory.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_files library.
 */

#ifndef JIUFENG_DIR_H
#define JIUFENG_DIR_H

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

#if defined(LINUX)
    #include <dirent.h>
    #include <sys/file.h>
#elif defined(WINDOWS)
    #include <io.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_file.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Define the directory data type.
 */
#if defined(LINUX)
    typedef DIR      jf_dir_t;
#elif defined(WINDOWS)
    typedef void     jf_dir_t;
#endif

/** Define the default create mode when creating directory with jf_dir_create().
 */
#define JF_DIR_DEFAULT_CREATE_MODE     (0755)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the directory entry data type.
 */
typedef struct
{
    /**The directory entry name.*/
    olchar_t jde_strName[JF_LIMIT_MAX_PATH_LEN];
    /**Length of the directory entry name.*/
    olsize_t jde_sName;
    u8 jde_u8Reserved[60];
} jf_dir_entry_t;

/** Define the function data type which is used to handle files in directory. The function is
 *  used in directory parse.
 */
typedef u32 (* jf_dir_fnHandleFile_t)(
    const olchar_t * pstrFullpath, jf_file_stat_t * pStat, void * pArg);

/** Define the function data type which is used to filter files in directory. The function is
 *  used in directory parse. When the function returns TRUE, the entry is ignored.
 */
typedef boolean_t (* jf_dir_fnFilterDirEntry_t)(jf_dir_entry_t * entry);

/** Define the function data type which is used to compare entry of directory. The function is
 *  used in directory parse. When the function returns TRUE, the entry is ignored.
 */
typedef olint_t (* jf_dir_fnCompareDirEntry_t)(const void * a, const void * b);

/* --- functional routines ---------------------------------------------------------------------- */

/** Create directory.
 *
 *  @note
 *  -# Mode is for Linux only, it specifies the permission to use.
 *
 *  @param pstrDirName [in] The directory name.
 *  @param mode [in] The file mode for the directory.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_DIR_ALREADY_EXIST Directory already exists.
 *  @retval JF_ERR_FAIL_CREATE_DIR Failed to create directory.
 */
FILESAPI u32 FILESCALL jf_dir_create(const olchar_t * pstrDirName, jf_file_mode_t mode);

/** Remove directory.
 *
 *  @param pstrDirName [in] The directory name to be removed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_REMOVE_DIR Failed to remove directory.
 */
FILESAPI u32 FILESCALL jf_dir_remove(const olchar_t * pstrDirName);

/** Open directory.
 *
 *  @param pstrDirName [in] The directory name.
 *  @param ppDir [out] The handler to the directory.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_OPEN_DIR Failed to open directory.
 */
FILESAPI u32 FILESCALL jf_dir_open(const olchar_t * pstrDirName, jf_dir_t ** ppDir);

/** Close directory.
 *
 *  @param ppDir [in/out] The handler to the directory.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI u32 FILESCALL jf_dir_close(jf_dir_t ** ppDir);

/** Get the first entry in directory.
 *
 *  @param pDir [in] The handler to the directory.
 *  @param pEntry [out] The pointer to the entry.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_DIR_ENTRY_NOT_FOUND No entry in directory.
 */
FILESAPI u32 FILESCALL jf_dir_getFirstDirEntry(jf_dir_t * pDir, jf_dir_entry_t * pEntry);

/** Get the next entry in directory.
 *
 *  @param pDir [in] The handler to the directory.
 *  @param pEntry [out] The pointer to the entry.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_DIR_ENTRY_NOT_FOUND No entry in directory.
 */
FILESAPI u32 FILESCALL jf_dir_getNextDirEntry(jf_dir_t * pDir, jf_dir_entry_t * pEntry);

/** Traversal directory.
 *
 *  @note
 *  -# The function traverses the directory recursively, the sub-directory are traversed also.
 *
 *  @param pstrDirName [in] The pointer to the directory name.
 *  @param fnHandleFile [in] The callback function for each entry in directory.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_OPEN_DIR Failed to open directory.
 */
FILESAPI u32 FILESCALL jf_dir_traversal(
    const olchar_t * pstrDirName, jf_dir_fnHandleFile_t fnHandleFile, void * pArg);

/** Parse directory.
 *
 *  @note
 *  -# The function parse the directory, the sub-directory are not parsed.
 *  -# If the callback function returns error, the parse will stop and the error is returned.
 *
 *  @param pstrDirName [in] The directory name.
 *  @param fnHandleFile [in] The callback function for each entry in directory.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_OPEN_DIR Failed to open directory.
 */
FILESAPI u32 FILESCALL jf_dir_parse(
    const olchar_t * pstrDirName, jf_dir_fnHandleFile_t fnHandleFile, void * pArg);

/** Scan the directory and return the entry.
 *
 *  @note
 *  -# The entries returned in entry list are sorted if fnCompare() is not NULL.
 *  -# The callback function fnCompare() can be NULL if sort is not required.
 *
 *  @param pstrDirName [in] The directory name.
 *  @param entry [out] The entry list.
 *  @param numofentry [in/out] Number of entry in the list before scan. After scan, the parameter
 *   specify number of available entry in list.
 *  @param fnFilter [in] The callback function to filter the entry.
 *  @param fnCompare [in] The callback function for sorting the entry.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_OPEN_DIR Failed to open directory.
 */
FILESAPI u32 FILESCALL jf_dir_scan(
    const olchar_t * pstrDirName, jf_dir_entry_t * entry, olint_t * numofentry,
    jf_dir_fnFilterDirEntry_t fnFilter, jf_dir_fnCompareDirEntry_t fnCompare);

/** The helper function for comparing entry.
 *
 *  @note The function can be used as fnCompare in jf_dir_scan().
 *
 *  @param a [in] The first entry.
 *  @param b [in] The second entry.
 *
 *  @return The compare result.
 *  @retval >0 a is greater than b.
 *  @retval =0 a is equal to b.
 *  @retval <0 a is less than b.
 */
FILESAPI olint_t FILESCALL jf_dir_compareDirEntry(const void * a, const void * b);


#endif /*JIUFENG_DIR_H*/

/*------------------------------------------------------------------------------------------------*/

