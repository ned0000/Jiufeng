/**
 *  @file conffile.h
 *
 *  @brief configuration file header file
 *
 *  @author Min Zhang
 *
 *  @note the implementation is very simple, reads the options in a
 *   configuration file.
 *  @note an example of the configuration file is as below:
 *  @note # this is an exmaple of the configuration file
 *  @note optiontagname1=value1 # this is an example of option
 *  @note optiontagname2=value2
 *  @note # the end of the configuration file
 *
 */

#ifndef JIUTAI_CONFFILE_H
#define JIUTAI_CONFFILE_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */

#define MAX_CONFFILE_LINE_LEN  256

/* --- data structures ----------------------------------------------------- */

typedef struct
{
    FILE * cf_pfConfFile;
    u8 cf_u8Reserved[8];
} conf_file_t;

/* --- functional routines ------------------------------------------------- */

/** Open a configuration file according to the file path.
 *
 *  @param pcf [in] the configuration file object to be created and returned.
 *  @param pstrFile [in] the path to the configuraiton file
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 openConfFile(conf_file_t * pcf, const olchar_t * pstrFile);

/** Close the configuration file object.
 *
 *  @param pcf [in] the configuration file object to be destroyed. After 
 *   destruction, it will be set to NULL.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 closeConfFile(conf_file_t * pcf);

/** Get an integer type option from the configuration file.
 *  If the pcf is set to NULL, or the option is not found, the default
 *  value will be returned.
 *
 *  @param pcf [in] the configuration file object.
 *  @param pstrTag [in] the option tag name.
 *  @param nDefault [in] the default value of the option.
 *  @param pnValue [in/out] the option value will be return to it.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 getConfFileInt(
    conf_file_t * pcf, const olchar_t * pstrTag, olint_t nDefault,
    olint_t * pnValue);

/** Get an string type option from the configuration file.
 *  If the pcf is set to NULL, or the option is not found, the default
 *  value will be returned.
 *
 *  @param pcf [in] the configuration file object.
 *  @param pstrTag [in] the option tag name.
 *  @param pstrDefault [in] the default value of the option.
 *  @param pstrValueBuf [in/out] the option value will be return to it.
 *  @param sBuf [in] the size of the pstrValueBuf.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 getConfFileString(
    conf_file_t * pcf, const olchar_t * pstrTag, const olchar_t * pstrDefault, 
    olchar_t * pstrValueBuf, olsize_t sBuf);

#endif /*JIUTAI_CONFFILE_H*/

/*---------------------------------------------------------------------------*/


