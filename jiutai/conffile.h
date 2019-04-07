/**
 *  @file conffile.h
 *
 *  @brief configuration file header file
 *
 *  @author Min Zhang
 *
 *  @note The implementation is very simple, reads the options in a
 *   configuration file.
 *  @note An example of the configuration file is as below:    \n
 *   # this is an exmaple of the configuration file            \n
 *   optiontagname1=value1 # this is an example of option      \n
 *   optiontagname2=value2                                     \n
 *   # the end of the configuration file
 *  @note Link with olfiles library
 */

#ifndef JIUTAI_CONFFILE_H
#define JIUTAI_CONFFILE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "files.h"

/* --- constant definitions ------------------------------------------------ */

#define JF_CONFFILE_MAX_LINE_LEN    (256)

/* --- data structures ----------------------------------------------------- */

typedef void  jf_conffile_t;

typedef struct
{
    olchar_t * jcop_pstrFile;
    u32 jcop_u32Reserved[8];
} jf_conffile_open_param_t;

/* --- functional routines ------------------------------------------------- */

/** Open a configuration file according to the file path.
 *
 *  @param pParam [in] the parameter for opening conf file
 *  @param ppConffile [out] the configuration file object to be created and returned.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
FILESAPI u32 FILESCALL jf_conffile_open(
    jf_conffile_open_param_t * pParam, jf_conffile_t ** ppConffile);

/** Close the configuration file object.
 *
 *  @param ppConffile [in/out] the configuration file object to be destroyed.
 *   After destruction, it will be set to NULL.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
FILESAPI u32 FILESCALL jf_conffile_close(jf_conffile_t ** ppConffile);

/** Get an integer type option from the configuration file.
 *  If the pjc is set to NULL, or the option is not found, the default
 *  value will be returned.
 *
 *  @param pConffile [in] the configuration file object.
 *  @param pstrTag [in] the option tag name.
 *  @param nDefault [in] the default value of the option.
 *  @param pnValue [in/out] the option value will be return to it.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
FILESAPI u32 FILESCALL jf_conffile_getInt(
    jf_conffile_t * pConffile, const olchar_t * pstrTag, olint_t nDefault,
    olint_t * pnValue);

/** Get an string type option from the configuration file.
 *  If the pjc is set to NULL, or the option is not found, the default
 *  value will be returned.
 *
 *  @param pConffile [in] the configuration file object.
 *  @param pstrTag [in] the option tag name.
 *  @param pstrDefault [in] the default value of the option.
 *  @param pstrValueBuf [in/out] the option value will be return to it.
 *  @param sBuf [in] the size of the pstrValueBuf.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
FILESAPI u32 FILESCALL jf_conffile_getString(
    jf_conffile_t * pConffile, const olchar_t * pstrTag,
    const olchar_t * pstrDefault, olchar_t * pstrValueBuf, olsize_t sBuf);

#endif /*JIUTAI_CONFFILE_H*/

/*---------------------------------------------------------------------------*/


