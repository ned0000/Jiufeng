/**
 *  @file filepersistency.h
 *
 *  @brief File persistency header file.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef PERSISTENCY_FILE_PERSISTENCY_H
#define PERSISTENCY_FILE_PERSISTENCY_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "persistencycommon.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/* --- functional routines ---------------------------------------------------------------------- */

u32 initFilePersistency(
    persistency_manager_t * pManager, jf_persistency_config_file_t * pConfig);

#endif /*PERSISTENCY_FILE_PERSISTENCY_H*/

/*------------------------------------------------------------------------------------------------*/


