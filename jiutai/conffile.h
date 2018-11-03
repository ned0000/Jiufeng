/**
 *  @file conffile.h
 *
 *  @brief configuration file header file
 *
 *  @author Min Zhang
 *
 *  @note
 *    the implementation is very simple, no memory allocation, no
 *    dependancy on other libraries
 *    reads the options in a configuration file an example of the
 *    configuration file
 *        --------------------------------------------------------
 *        |# this is an exmaple of the configuration file        |
 *        |optiontagname1=value1 # this is an example of option  |
 *        |optiontagname2=value2                                 |
 *        |                                                      |
 *        |# the end of the configuration file                   |
 *        --------------------------------------------------------
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

/** open a configuration file according to the file path.
 *
 *  @param pcf : conf_file_t <BR>
 *    @b [in] the configuration file object to be created and returned.
 *  @param pstrFile : const olchar_t * <BR>
 *    @b [in] the path to the configuraiton file
 *
 *  @return: return OLERR_NO_ERROR otherwise the error code 
 */
u32 openConfFile(conf_file_t * pcf, const olchar_t * pstrFile);

/** close the configuration file object.
 *
 *  @Param pcf : conf_file_t <BR>
 *    @b [in] the configuration file object to be destroyed. After 
 *           destruction, it will be set to NULL.
 *
 *  @return: return OLERR_NO_ERROR otherwise the error code 
 */
u32 closeConfFile(conf_file_t * pcf);

/** get an integer type option from the configuration file.
 *  If the pcf is set to NULL, or the option is not found, the default
 *  value will be returned.
 *
 *  @param pcf : conf_file_t <BR>
 *    @b [in] the configuration file object.
 *  @param pstrTag : const olchar_t * <BR>
 *    @b [in] the option tag name.
 *  @param nDefault : olint_t <BR>
 *    @b [in] the default value of the option.
 *  @param pnValue : olint_t * <BR>
 *       [out] the option value will be return to it.
 *
 *  @return: return OLERR_NO_ERROR otherwise the error code 
 */
u32 getConfFileInt(
    conf_file_t * pcf, const olchar_t * pstrTag, olint_t nDefault,
    olint_t * pnValue);

/** get an string type option from the configuration file.
 *       If the pcf is set to NULL, or the option is not found, the default
 *       value will be returned.
 *
 *  @param pcf : conf_file_t <BR>
 *    @b [in] the configuration file object.
 *  @param pstrTag : const olchar_t * <BR>
 *    @b [in] the option tag name.
 *  @param pstrDefault : const olchar_t * <BR>
 *    @b [in] the default value of the option.
 *  @param pstrValueBuf : olchar_t * <BR>
 *    @b [out] the option value will be return to it.
 *  @param sBuf : olsize_t <BR>
 *    @b [in] the size of the pstrValueBuf.
 *
 *  @return: return OLERR_NO_ERROR otherwise the error code 
 */
u32 getConfFileString(
    conf_file_t * pcf, const olchar_t * pstrTag, const olchar_t * pstrDefault, 
    olchar_t * pstrValueBuf, olsize_t sBuf);

#endif /*JIUTAI_CONFFILE_H*/

/*---------------------------------------------------------------------------*/


