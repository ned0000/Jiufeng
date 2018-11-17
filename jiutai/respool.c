/**
 *  @file respool.c
 *
 *  @brief resource pool implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdlib.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "respool.h"
#include "logger.h"
#include "errcode.h"
#include "array.h"
#include "syncmutex.h"
#include "xmalloc.h"

/* --- private data/data structure section --------------------------------- */

/**
 *  Internal resource state
 */
typedef enum
{
    IRS_FREE = 0,  /**< the resource is free */
    IRS_RESERVED,  /**< the resource is reserved */
    IRS_BUSY,      /**< the resource is busy */
} internal_resource_state_t;

/**
 *  Internal resource data structure
 */
typedef struct
{
    /** fulltime resource or not */
    boolean_t ir_bFulltime;
    u8 ir_u8Reserved[7];
    /** the resource state */
    internal_resource_state_t ir_irsResourceState;
    /** the data of the resource */
    resource_data_t * ir_prdData;
} internal_resource_t;

typedef struct
{
    /** null-terminated string. It should be no longer than 15 characters */
    olchar_t irp_strName[16];
    /** minimum number of resources that should be allocated all the time */
    u32 irp_u32MinResources;
    /** maximum number of resources that can co-exist at the same time */
    u32 irp_u32MaxResources;
    /** release the parttime resource immediately after use */
    boolean_t irp_bImmediateRelease;
    /** synchronize the access to the resources */
    sync_mutex_t irp_smLock;
    /** basic array contains fulltime resources */
    basic_array_t * irp_pbaFulltimeResources;
    /** basic array contains parttime resources */    
    basic_array_t * irp_pbaParttimeResources;

    /** the callback function to create resource*/
    fnCreateResource_t irp_fnCreateResource;
    /** the callback function to destroy resource*/
    fnDestroyResource_t irp_fnDestroyResource;

} internal_resource_pool_t;

/* --- private routine section --------------------------------------------- */

/** Validate resource pool parameter
 *
 *  @param prpp [in] the pointer to the parameter
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_INVALID_PARAM invalid parameter
 */
static u32 _validateParam(resource_pool_param_t * prpp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    if (prpp->rpp_u32MinResources > prpp->rpp_u32MaxResources)
        u32Ret = OLERR_INVALID_PARAM;
    else
    {
        if ((prpp->rpp_fnCreateResource == NULL) ||
            (prpp->rpp_fnDestroyResource == NULL))
        {
            u32Ret = OLERR_INVALID_PARAM;
        }
    }

    return u32Ret;
}

/** Lock resource pool.
 *
 *  @param pirp [in] the pointer to the resource pool 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _lockResourcePool(internal_resource_pool_t * pirp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("lock resource pool");

    u32Ret = acquireSyncMutex(&(pirp->irp_smLock));

    return u32Ret;
}

/** Unlock resource pool.
 *
 *  @param pirp [in] the pointer to the resource pool 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _unlockResourcePool(internal_resource_pool_t * pirp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    logInfoMsg("unlock resource pool");
        
    u32Ret = releaseSyncMutex(&(pirp->irp_smLock));

    return u32Ret;
}

/** Destroy a resource.
 *
 *  @param pirp [in] the pointer to the resource pool 
 *  @param ppir [in/out] the pointer to the resource to be destroyed. 
 *   After destruction, it will be set to NULL.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _destroyResourceInPool(
    internal_resource_pool_t * pirp, internal_resource_t ** ppir)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_t * pir = NULL;

    pir = (internal_resource_t *)*ppir;

    logInfoMsg("destroy resource");

    *ppir = NULL;

    u32Ret = pirp->irp_fnDestroyResource((resource_t *)pir, &pir->ir_prdData);
    
    xfree((void **)&pir);

    return u32Ret;
}

/** Check whether the maximum resources are reached in array
 *
 *  @param pirp [in] the pointer to the resource pool
 *  @param bFulltime [in] specify which array should be checked
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_REACH_MAX_RESOURCES reach maximum resources
 */
static u32 _isMaxPoolResourcesReached(
    internal_resource_pool_t * pirp, boolean_t bFulltime)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 u32Size;

    if (bFulltime)
    {
        u32Size = getBasicArraySize(pirp->irp_pbaFulltimeResources);
        if (u32Size == pirp->irp_u32MinResources)
        {
            logInfoMsg("max fulltime resource is reached");
            u32Ret = OLERR_REACH_MAX_RESOURCES;
        }
    }
    else
    {
        u32Size = getBasicArraySize(pirp->irp_pbaParttimeResources);
        if (u32Size == pirp->irp_u32MaxResources - pirp->irp_u32MinResources)
        {
            logInfoMsg("max parttime resource is reached");
            u32Ret = OLERR_REACH_MAX_RESOURCES;
        }
    }

    return u32Ret;
}

/** Set the state of the resource
 *
 *  @param pir [in] the pointer to the resource.
 *  @param irs [in] the new state to be set.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _setPoolResourceState(
    internal_resource_t * pir, internal_resource_state_t irs)
{
    u32 u32Ret = OLERR_NO_ERROR;

    pir->ir_irsResourceState = irs;

    return u32Ret;
}

/** Create a resource and add it to the array
 *
 *  @param pirp [in] the pointer to the resource pool
 *  @param bFulltime [in] specify if the resource is fulltime or parttime
 *  @param state [in] the resource state
 *  @param ppr [in/out] the pointer to the resource to be created and returned.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _createResourceInPoolArray(
    internal_resource_pool_t * pirp, boolean_t bFulltime,
    internal_resource_state_t state, resource_t ** ppr)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_t * pir = NULL;
    basic_array_t * pba;

    if (bFulltime)
        pba = pirp->irp_pbaFulltimeResources;
    else
        pba = pirp->irp_pbaParttimeResources;

    logDebugMsg(
        "create resource in %s array", (bFulltime ? "fulltime" : "parttime"));

    if (u32Ret == OLERR_NO_ERROR)
    {
        _lockResourcePool(pirp);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = _isMaxPoolResourcesReached(pirp, bFulltime);

        if (u32Ret == OLERR_NO_ERROR)
            u32Ret = xmalloc((void **)&pir, sizeof(internal_resource_t));

        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_memset(pir, 0, sizeof(internal_resource_t));

            pir->ir_bFulltime = bFulltime;
            _setPoolResourceState(pir, state);

            u32Ret = pirp->irp_fnCreateResource(pir, &pir->ir_prdData);
        }
    
        if (u32Ret == OLERR_NO_ERROR)
        {
            /* append the resource to the array */
            u32Ret = appendToBasicArray(pba, (basic_array_element_t *)pir);
        }

        _unlockResourcePool(pirp);
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppr = (resource_t *)pir;
    else if (pir != NULL)
        _destroyResourceInPool(pirp, &pir);

    return u32Ret;
}

/** Create resource
 *
 *  @param pirp [in] the pointer to the resource pool
 *  @param prpp [in/out] the pointer to the resource to be created
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_OUT_OF_MEMORY out of memeory
 *  @retval OLERR_INVALID_PARAM invalid parameter
 */
static u32 _createResourceInPool(
    internal_resource_pool_t * pirp, resource_t ** ppr)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _createResourceInPoolArray(pirp, TRUE, IRS_BUSY, ppr);
    if (u32Ret != OLERR_NO_ERROR)
    {
        u32Ret = _createResourceInPoolArray(pirp, FALSE, IRS_BUSY, ppr);
    }

    return u32Ret;    
}

/** Destroy all resources in array
 *
 *  @param pirp [in] the pointer to the resource pool to be destroyed. 
 *  @param ppba [in/out] the pointer to the array to be destroyed. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _destroyResourceArray(
    internal_resource_pool_t * pirp, basic_array_t ** ppba)
{
    u32 u32Ret = OLERR_NO_ERROR;
    u32 u32Size, u32Index;
    basic_array_t * pba = *ppba;
    internal_resource_t * pir;

    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Size = getBasicArraySize(pba);
        u32Index = 0;
        while ((u32Index < u32Size) && (u32Ret == OLERR_NO_ERROR))
        {
            u32Ret = getAtBasicArray(pba, 0, (basic_array_element_t **)&pir);
            if (u32Ret == OLERR_NO_ERROR)
            {
                removeAtBasicArray(pba, 0);
                _destroyResourceInPool(pirp, &pir);
            }
            u32Index ++;
        }

        destroyBasicArray(ppba);

        _unlockResourcePool(pirp);
    }

    return u32Ret;
}

/** Destroy all resources in pool.
 *
 *  @param pirp [in] the pointer to the resource pool to be destroyed. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _destroyAllResources(internal_resource_pool_t * pirp)
{
    u32 u32Ret = OLERR_NO_ERROR;

    /* full time */
    logDebugMsg("destroy resource in fulltime array");
    u32Ret = _destroyResourceArray(pirp, &pirp->irp_pbaFulltimeResources);
    if (u32Ret != OLERR_NO_ERROR)
    {
        logErrMsg(u32Ret,
            "failed to destroy full time resource array of resource pool %s",
            pirp->irp_strName);
    }

    /* part time */
    logDebugMsg("destroy resource in parttime array");
    u32Ret = _destroyResourceArray(pirp, &pirp->irp_pbaParttimeResources);
    if (u32Ret != OLERR_NO_ERROR)
    {
        logErrMsg(u32Ret,
            "failed to destroy part time resource array of resource pool %s",
            pirp->irp_strName);
    }

    return u32Ret;
}

/** Create a resource pool according to the parameters.
 *
 *  @param ppirp [in/out] the pointer to the resource pool to be created and returned.
 *  @param prpp [in] the parameters for creating the resource pool.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_OUT_OF_MEMORY out of memeory
 *  @retval OLERR_INVALID_PARAM invalid parameter
 *
 */
static u32 _createResourcePool(
    internal_resource_pool_t ** ppirp, resource_pool_param_t * prpp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_pool_t * pirp;

    u32Ret = xmalloc((void **)&pirp, sizeof(internal_resource_pool_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_memset(pirp, 0, sizeof(internal_resource_pool_t));

        u32Ret = initSyncMutex(&(pirp->irp_smLock));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = createBasicArray(&(pirp->irp_pbaFulltimeResources));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = createBasicArray(&(pirp->irp_pbaParttimeResources));
    }
    
    if (u32Ret == OLERR_NO_ERROR)
    {
        pirp->irp_fnCreateResource = prpp->rpp_fnCreateResource;
        pirp->irp_fnDestroyResource = prpp->rpp_fnDestroyResource;

        pirp->irp_u32MaxResources = prpp->rpp_u32MaxResources;
        pirp->irp_u32MinResources = prpp->rpp_u32MinResources;

        pirp->irp_bImmediateRelease = prpp->rpp_bImmediateRelease;
        
        ol_strcpy(pirp->irp_strName, prpp->rpp_pstrName);
    }

    if (u32Ret == OLERR_NO_ERROR)
        *ppirp = pirp;
    else
        destroyResourcePool((resource_pool_t **)&pirp);

    return u32Ret;
}

/** Check whether the resource is free
 *
 *  @param pir [in] the pointer to the resource. 
 *
 *  @return the free state of the resource
 *  @retval TRUE the resource is free
 *  @retval FALSE the resource is not free
 */
static boolean_t _isPoolResourceFree(internal_resource_t * pir)
{
    boolean_t bFree = FALSE;

    if (pir->ir_irsResourceState == IRS_FREE)
        bFree = TRUE;

    return bFree;
}

/** Get resource from array in resource pool.
 *
 *  @param pirp [in] the pointer to internal resource pool
 *  @param pba [in] the array contains resources
 *  @param ppRes [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _getResourceFromPoolArray(
    internal_resource_pool_t * pirp, basic_array_t * pba, resource_t ** ppRes)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_t * pir;
    u32 u32Index, u32Size;
    boolean_t bFree;

    *ppRes = NULL;

    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Size = getBasicArraySize(pba);
        u32Index = 0;
        while ((u32Index < u32Size) && (u32Ret == OLERR_NO_ERROR))
        {
            u32Ret = getAtBasicArray(
                pba, u32Index, (basic_array_element_t **)&pir);
            if (u32Ret == OLERR_NO_ERROR)
            {
                bFree = _isPoolResourceFree(pir);
                if (bFree)
                {
                    logDebugMsg(
                        "find free resource from %s array",
                        (pir->ir_bFulltime ? "fulltime" : "parttime"));
                    _setPoolResourceState(pir, IRS_BUSY);
                    *ppRes = pir;
                    u32Ret = OLERR_NO_ERROR;
                    break;
                }
            }
            u32Index ++;
        }

        _unlockResourcePool(pirp);
    }

    if (*ppRes == NULL)
        u32Ret = OLERR_NOT_FOUND;
    
    return u32Ret;
}

/** Get resource from resource pool.
 *
 *  @param pirp [in] the pointer to internal resource pool
 *  @param ppRes [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _getResourceFromPool(
    internal_resource_pool_t * pirp, resource_t ** ppRes)
{
    u32 u32Ret = OLERR_NO_ERROR;

    /* get resource from fulltime array */
    logDebugMsg("get resource from fulltime pool array");
    u32Ret = _getResourceFromPoolArray(
        pirp, pirp->irp_pbaFulltimeResources, ppRes);
    if (u32Ret != OLERR_NO_ERROR)
    {
        logDebugMsg("get resource from parttime pool array");
        u32Ret = _getResourceFromPoolArray(
            pirp, pirp->irp_pbaParttimeResources, ppRes);
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        u32Ret = _createResourceInPool(pirp, ppRes);
    }
    
    return u32Ret;
}

/** Put resource in resource pool.
 *
 *  @param pirp [in] the pointer to internal resource pool
 *  @param ppr [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *
 */
static u32 _putResourceInPool(
    internal_resource_pool_t * pirp, resource_t ** ppr)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_t * pir = (internal_resource_t *)*ppr;

    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pir->ir_bFulltime)
        {
            _setPoolResourceState(pir, IRS_FREE);
        }
        else if (pirp->irp_bImmediateRelease)
        {
            removeBasicArrayElement(
                pirp->irp_pbaParttimeResources, (basic_array_element_t *)pir);
            _destroyResourceInPool(pirp, &pir);
        }

        _unlockResourcePool(pirp);
    }

    *ppr = NULL;

    return u32Ret;
}

/** Find the free parttime resources, and release them.
 *
 *  @param pirp [in] the pointer to the internal resource pool. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
static u32 _reapResourceInPool(internal_resource_pool_t * pirp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    basic_array_t * pba;
    internal_resource_t * pir;
    boolean_t bFree;
    u32 u32Index = 0, u32Size = 0;
    
    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == OLERR_NO_ERROR)
    {
        /* the resource can be timed out */
        pba = pirp->irp_pbaParttimeResources;
        u32Index = 0;
        u32Size = getBasicArraySize(pba);
        while ((u32Ret == OLERR_NO_ERROR) && (u32Index < u32Size))
        {
            u32Ret = getAtBasicArray(pba, u32Index, (basic_array_element_t **)&pir);
            if (u32Ret == OLERR_NO_ERROR)
            {
                /* try to destroy it if it free */
                bFree = _isPoolResourceFree(pir);
                if (bFree)
                {
                    pirp->irp_fnDestroyResource(pir, &pir->ir_prdData);
                    removeAtBasicArray(pba, u32Index);
                    u32Size --;
                }
                else
                {
                    u32Index ++;
                }
            }
        }

        _unlockResourcePool(pirp);
    }

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

u32 createResourcePool(resource_pool_t ** pprp, resource_pool_param_t * prpp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_pool_t * pirp = NULL;

    assert((pprp != NULL) && (prpp != NULL));

    logInfoMsg("create resource pool");

    u32Ret = _validateParam(prpp);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _createResourcePool(&pirp, prpp);

    if (u32Ret == OLERR_NO_ERROR)
        *pprp = pirp;
    else if (pirp != NULL)
        destroyResourcePool((resource_pool_t **)&pirp);

    return u32Ret;
}

u32 destroyResourcePool(resource_pool_t ** pprp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_pool_t * pirp = NULL;
    
    assert(pprp != NULL);

    pirp = (internal_resource_pool_t *)*pprp;
    *pprp = NULL;

    logInfoMsg("destroy resource pool");

    /* destroy the resource list */
    _destroyAllResources(pirp);

    /* destroy the mutex */
    u32Ret = finiSyncMutex(&(pirp->irp_smLock));

    /* free the resource pool */
    xfree((void **)&pirp);

    return u32Ret;
}

u32 getResourceFromPool(resource_pool_t * prp, resource_t ** ppRes)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_pool_t * pirp = (internal_resource_pool_t *)prp;

    assert((prp != NULL) && (ppRes != NULL));

    logDebugMsg("get resource from pool");
    
    /* get resource from pool */
    u32Ret = _getResourceFromPool(pirp, ppRes);
    if (u32Ret != OLERR_NO_ERROR)
    {
        u32Ret = _createResourceInPool(pirp, ppRes);
    }
    
    return u32Ret;
}

u32 putResourceInPool(resource_pool_t * prp, resource_t ** ppRes)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_pool_t * pirp = (internal_resource_pool_t *)prp;;

    assert((prp != NULL) && (ppRes != NULL));

    logDebugMsg("put resource in pool");
    
    u32Ret = _putResourceInPool(pirp, ppRes);

    return u32Ret;
}

u32 reapResourceInPool(resource_pool_t * prp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_resource_pool_t * pirp;
    
    assert(prp != NULL);

    pirp = (internal_resource_pool_t *)prp;
    u32Ret = _reapResourceInPool(pirp);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

