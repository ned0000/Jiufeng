/**
 *  @file jf_attask.h
 *
 *  @brief Header file declares functions for the attask object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_attask object.
 *  -# The attask object is NOT thread safe.
 *  -# Link with jf_time common object for time function.
 *  -# Link with jiukun library for memory allocation.
 */

#ifndef JIUTAI_ATTASK_H
#define JIUTAI_ATTASK_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** Define the attask data type.
 */
typedef void  jf_attask_t;

/** Define the function data type which is a callback function of attask item.
 */
typedef u32 (* jf_attask_fnCallbackOfItem_t)(void * pData);

/** Define the function data type which is used to destroy attask item.
 */
typedef u32 (* jf_attask_fnDestroyItem_t)(void ** ppData);

/* --- functional routines ---------------------------------------------------------------------- */

/** Creates an attask object.
 *
 *  @param ppAttask [out] the pointer to attask object
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_attask_create(jf_attask_t ** ppAttask);

/** Destroy an attask object
 *
 *  @param ppAttask [in/out] the pointer to attask object 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_attask_destroy(jf_attask_t ** ppAttask);

/** Add a timed callback task to attask object with millisecond granularity.
 *
 *  @param pAttask [in] the pointer to attask object
 *  @param pData [in] the data object to associate with the task
 *  @param u32Milliseconds [in] the number of milliseconds for the task 
 *  @param fnCallback [in] the callback function when the task is triggerred
 *  @param fnDestroy [in] the callback function to destroy the task
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_JIUKUN_OUT_OF_MEMORY out of memory
 */
u32 jf_attask_addItem(
    jf_attask_t * pAttask, void * pData, u32 u32Milliseconds,
    jf_attask_fnCallbackOfItem_t fnCallback, jf_attask_fnDestroyItem_t fnDestroy);

/** Removes tasks specified by the parameter from an attask object.
 *
 *  @note If there are multiple items pertaining to task, all of them are removed.
 *  @note Before destroying the task, fnDestroy() is called.
 *
 *  @param pAttask [in] the attask object to remove the task from 
 *  @param pData [in] the task to remove 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_attask_removeItem(jf_attask_t * pAttask, void * pData);

/** Checks the attask item and get the block time.
 *
 *  @param pAttask [in] the pointer to attask object
 *  @param pu32Blocktime [out] max block time in millisecond
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_attask_check(jf_attask_t * pAttask, u32 * pu32Blocktime);

#endif  /*JIUTAI_ATTASK_H*/

/*------------------------------------------------------------------------------------------------*/


