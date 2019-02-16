/**
 *  @file attask.h
 *
 *  @brief API for the attask common object
 *
 *  @author Min Zhang
 *
 *  @note The attask object is not thread safe, DO NOT use it in multi-thread
 *   envirionment
 *  @note link with xtime, xmalloc common object
 *
 */

#ifndef JIUTAI_ATTASK_H
#define JIUTAI_ATTASK_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef void  attask_t;

typedef u32 (* fnCallbackOfAttaskItem_t)(void * pData);
typedef u32 (* fnDestroyAttaskItem_t)(void ** ppData);

/* --- functional routines ------------------------------------------------- */

/** Creates an empty attask 
 *
 *  @param ppAttask [out] the attask
 *
 *  @return the error code
 */
u32 createAttask(attask_t ** ppAttask);

/** Destroy a attask
 *
 *  @param pAttask [in/out] the attask object 
 *
 *  @return the error code
 */
u32 destroyAttask(attask_t ** ppAttask);

/** Add a timed callback with millisecond granularity
 *
 *  @param pAttask [in] the attask
 *  @param pData [in] the data object to associate with the task 
 *  @param u32Millseconds [in] the number of milliseconds for the task 
 *  @param fnCallback [in] the callback function pointer 
 *  @param fnDestory [in] the abort function pointer, which triggers all 
 *   non-triggered timed callbacks, upon shutdown
 *
 *  @return the error code
 */
u32 addAttaskItem(
    attask_t * pAttask, void * pData, u32 u32Milliseconds,
    fnCallbackOfAttaskItem_t fnCallback, fnDestroyAttaskItem_t fnDestroy);

/** Removes item specified by pData from an attask
 *
 *  @note If there are multiple item pertaining to pData, all of them are
 *   removed
 *  @note Before destroying the attask item structure, (* destroy)() is called
 *
 *  @param pAttask [in] the attask object to remove the callback from 
 *  @param pData [in] the data object to remove 
 *
 *  @return the error code
 */
u32 removeAttaskItem(attask_t * pAttask, void * pData);

/** Checks the attask item and get the block time.
 *
 *  @param pAttask [in] the attask
 *  @param pu32Blocktime [out] max block time in millisecond
 *
 *  @return the error code
 */
u32 checkAttask(attask_t * pAttask, u32 * pu32Blocktime);

#endif  /*JIUTAI_ATTASK_H*/

/*---------------------------------------------------------------------------*/


