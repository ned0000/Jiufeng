/**
 *  @file sqlitepersistency.h
 *
 *  @brief sqlite persistency header file
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef PERSISTENCY_SQLITE_PERSISTENCY_H
#define PERSISTENCY_SQLITE_PERSISTENCY_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "errcode.h"
#include "persistencycommon.h"
#include "sqlite3.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */

/* --- functional routines ------------------------------------------------- */

u32 initSqlitePersistency(
    persistency_manager_t * pManager, jf_persistency_config_sqlite_t * pConfig);

#endif /*PERSISTENCY_SQLITE_PERSISTENCY_H*/

/*---------------------------------------------------------------------------*/


