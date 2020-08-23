/**
 *  @file jf_conffile.h
 *
 *  @brief Header file defines the interface for configuration file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_files library.
 *  -# The file contains lines with configuration in "tag=value" pair format.
 *  -# Comment is supported. '#' is the identification of comment.
 *
 *  <HR>
 *
 *  @par Example
 *  @code
 *   # this is an exmaple of the configuration file
 *   config-tag1=value1 # this is an example of configuration
 *   config-tag2=value2
 *   # the end of the configuration file
 *  @endcode
 *
 *  <HR>
 */

#ifndef JIUFEN_CONFFILE_H
#define JIUFEN_CONFFILE_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_file.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Maximum line length.
 */
#define JF_CONFFILE_MAX_LINE_LEN                   (1024)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the configuration file data type.
 */
typedef void  jf_conffile_t;

/** Define the parameter for opening configuration file.
 */
typedef struct
{
    /**The configuration file.*/
    olchar_t * jcop_pstrFile;
    /**Open the file for write.*/
    boolean_t jcop_bWrite;
    u8 jcop_u8Reserved[7];
    u32 jcop_u32Reserved[6];
} jf_conffile_open_param_t;

/** Define the function data type which is used to handle configuration. The function is used in
 *  traversal.
 *
 *  @note
 *  -# The traversal will stop if the return code is not JF_ERR_NO_ERROR.
 *
 *  @param pstrTag [in] The config tag name.
 *  @param pstrValue [in] The config value.
 *  @param pArg [in] The argument for the callback function. It's passed by jf_conffile_traverse().
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_conffile_fnHandleConfig_t)(olchar_t * pstrTag, olchar_t * pstrValue, void * pArg);

/* --- functional routines ---------------------------------------------------------------------- */

/** Open a configuration file according to the file path.
 *
 *  @param pParam [in] The parameter for opening conf file.
 *  @param ppConffile [out] The configuration file object to be created and returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI u32 FILESCALL jf_conffile_open(
    jf_conffile_open_param_t * pParam, jf_conffile_t ** ppConffile);

/** Close the configuration file object.
 *
 *  @param ppConffile [in/out] The configuration file object to be destroyed. After destruction, it
 *   will be set to NULL.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI u32 FILESCALL jf_conffile_close(jf_conffile_t ** ppConffile);

/** Write configuration to file.
 *
 *  @param pConffile [in] The configuration file object to write.
 *  @param pstrData [in] The data to write.
 *  @param sData [in] Size of the data.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI u32 FILESCALL jf_conffile_write(
    jf_conffile_t * pConffile, const olchar_t * pstrData, olsize_t sData);


/** Get an integer type config from the configuration file.
 *
 *  @note
 *  -# If the tag name is set to NULL, or the config is not found, the default value will be
 *    returned.
 *
 *  @param pConffile [in] The configuration file object.
 *  @param pstrTag [in] The config tag name.
 *  @param nDefault [in] The default value of the config.
 *  @param pnValue [in/out] The config value returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI u32 FILESCALL jf_conffile_getInt(
    jf_conffile_t * pConffile, const olchar_t * pstrTag, olint_t nDefault, olint_t * pnValue);

/** Get an string type config from the configuration file.
 *
 *  @note
 *  -# If the tag name is set to NULL, or the config is not found, the default value will be
 *   returned.
 *
 *  @param pConffile [in] The configuration file object.
 *  @param pstrTag [in] The config tag name.
 *  @param pstrDefault [in] The default value of the config.
 *  @param pstrValueBuf [out] The config value buffer.
 *  @param sBuf [in] The size of the value buffer.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI u32 FILESCALL jf_conffile_getString(
    jf_conffile_t * pConffile, const olchar_t * pstrTag, const olchar_t * pstrDefault,
    olchar_t * pstrValueBuf, olsize_t sBuf);

/** Traverse configuration file.
 *
 *  @note
 *  -# The function traverses the line in configuration file.
 *  -# The traversal will stop if return code of callback function is not JF_ERR_NO_ERROR.
 *
 *  @param pConffile [in] The configuration file object.
 *  @param fnHandleConfig [in] The callback function for each config in file.
 *  @param pArg [in] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
FILESAPI u32 FILESCALL jf_conffile_traverse(
    jf_conffile_t * pConffile, jf_conffile_fnHandleConfig_t fnHandleConfig, void * pArg);

#endif /*JIUFEN_CONFFILE_H*/

/*------------------------------------------------------------------------------------------------*/
