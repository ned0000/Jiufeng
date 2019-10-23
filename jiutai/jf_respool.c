/**
 *  @file jf_respool.c
 *
 *  @brief resource pool implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_respool.h"
#include "jf_logger.h"
#include "jf_err.h"
#include "jf_array.h"
#include "jf_mutex.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

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
    jf_respool_resource_data_t * ir_pjrrdData;
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
    jf_mutex_t irp_jmLock;
    /** array contains fulltime resources */
    jf_array_t * irp_pjaFulltimeResources;
    /** array contains parttime resources */    
    jf_array_t * irp_pjaParttimeResources;

    /** the callback function to create resource*/
    jf_respool_fnCreateResource_t irp_fnCreateResource;
    /** the callback function to destroy resource*/
    jf_respool_fnDestroyResource_t irp_fnDestroyResource;

} internal_resource_pool_t;

/* --- private routine section ------------------------------------------------------------------ */

/** Validate resource pool parameter
 *
 *  @param pjrcp [in] the pointer to the parameter
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_INVALID_PARAM invalid parameter
 */
static u32 _validateParam(jf_respool_create_param_t * pjrcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pjrcp->jrcp_u32MinResources > pjrcp->jrcp_u32MaxResources)
        u32Ret = JF_ERR_INVALID_PARAM;
    else
    {
        if ((pjrcp->jrcp_fnCreateResource == NULL) ||
            (pjrcp->jrcp_fnDestroyResource == NULL))
        {
            u32Ret = JF_ERR_INVALID_PARAM;
        }
    }

    return u32Ret;
}

/** Lock resource pool.
 *
 *  @param pirp [in] the pointer to the resource pool 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _lockResourcePool(internal_resource_pool_t * pirp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("lock resource pool");

    u32Ret = jf_mutex_acquire(&(pirp->irp_jmLock));

    return u32Ret;
}

/** Unlock resource pool.
 *
 *  @param pirp [in] the pointer to the resource pool 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _unlockResourcePool(internal_resource_pool_t * pirp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_logger_logInfoMsg("unlock resource pool");
        
    u32Ret = jf_mutex_release(&(pirp->irp_jmLock));

    return u32Ret;
}

/** Destroy a resource.
 *
 *  @param pirp [in] the pointer to the resource pool 
 *  @param ppir [in/out] the pointer to the resource to be destroyed. 
 *   After destruction, it will be set to NULL.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _destroyResourceInPool(
    internal_resource_pool_t * pirp, internal_resource_t ** ppir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_t * pir = NULL;

    pir = (internal_resource_t *)*ppir;

    jf_logger_logInfoMsg("destroy resource");

    *ppir = NULL;

    u32Ret = pirp->irp_fnDestroyResource((jf_respool_resource_t *)pir, &pir->ir_pjrrdData);
    
    jf_jiukun_freeMemory((void **)&pir);

    return u32Ret;
}

/** Check whether the maximum resources are reached in array
 *
 *  @param pirp [in] the pointer to the resource pool
 *  @param bFulltime [in] specify which array should be checked
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_REACH_MAX_RESOURCES reach maximum resources
 */
static u32 _isMaxPoolResourcesReached(
    internal_resource_pool_t * pirp, boolean_t bFulltime)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Size;

    if (bFulltime)
    {
        u32Size = jf_array_getSize(pirp->irp_pjaFulltimeResources);
        if (u32Size == pirp->irp_u32MinResources)
        {
            jf_logger_logInfoMsg("max fulltime resource is reached");
            u32Ret = JF_ERR_REACH_MAX_RESOURCES;
        }
    }
    else
    {
        u32Size = jf_array_getSize(pirp->irp_pjaParttimeResources);
        if (u32Size == pirp->irp_u32MaxResources - pirp->irp_u32MinResources)
        {
            jf_logger_logInfoMsg("max parttime resource is reached");
            u32Ret = JF_ERR_REACH_MAX_RESOURCES;
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
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _setPoolResourceState(
    internal_resource_t * pir, internal_resource_state_t irs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pir->ir_irsResourceState = irs;

    return u32Ret;
}

/** Create a resource and add it to the array
 *
 *  @param pirp [in] the pointer to the resource pool
 *  @param bFulltime [in] specify if the resource is fulltime or parttime
 *  @param state [in] the resource state
 *  @param ppjrr [in/out] the pointer to the resource to be created and returned.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _createResourceInPoolArray(
    internal_resource_pool_t * pirp, boolean_t bFulltime,
    internal_resource_state_t state, jf_respool_resource_t ** ppjrr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_t * pir = NULL;
    jf_array_t * pja;

    if (bFulltime)
        pja = pirp->irp_pjaFulltimeResources;
    else
        pja = pirp->irp_pjaParttimeResources;

    jf_logger_logDebugMsg(
        "create resource in %s array", (bFulltime ? "fulltime" : "parttime"));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _lockResourcePool(pirp);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _isMaxPoolResourcesReached(pirp, bFulltime);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_jiukun_allocMemory((void **)&pir, sizeof(internal_resource_t), 0);

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_memset(pir, 0, sizeof(internal_resource_t));

            pir->ir_bFulltime = bFulltime;
            _setPoolResourceState(pir, state);

            u32Ret = pirp->irp_fnCreateResource(pir, &pir->ir_pjrrdData);
        }
    
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            /* append the resource to the array */
            u32Ret = jf_array_appendElementTo(pja, (jf_array_element_t *)pir);
        }

        _unlockResourcePool(pirp);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppjrr = (jf_respool_resource_t *)pir;
    else if (pir != NULL)
        _destroyResourceInPool(pirp, &pir);

    return u32Ret;
}

/** Create resource
 *
 *  @param pirp [in] the pointer to the resource pool
 *  @param ppjrr [in/out] the pointer to the resource to be created
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_OUT_OF_MEMORY out of memeory
 *  @retval JF_ERR_INVALID_PARAM invalid parameter
 */
static u32 _createResourceInPool(
    internal_resource_pool_t * pirp, jf_respool_resource_t ** ppjrr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _createResourceInPoolArray(pirp, TRUE, IRS_BUSY, ppjrr);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        u32Ret = _createResourceInPoolArray(pirp, FALSE, IRS_BUSY, ppjrr);
    }

    return u32Ret;    
}

/** Destroy all resources in array
 *
 *  @param pirp [in] the pointer to the resource pool to be destroyed. 
 *  @param ppja [in/out] the pointer to the array to be destroyed. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _destroyResourceArray(
    internal_resource_pool_t * pirp, jf_array_t ** ppja)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Size, u32Index;
    jf_array_t * pja = *ppja;
    internal_resource_t * pir;

    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Size = jf_array_getSize(pja);
        u32Index = 0;
        while ((u32Index < u32Size) && (u32Ret == JF_ERR_NO_ERROR))
        {
            u32Ret = jf_array_getElementAt(pja, 0, (jf_array_element_t **)&pir);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                jf_array_removeElementAt(pja, 0);
                _destroyResourceInPool(pirp, &pir);
            }
            u32Index ++;
        }

        jf_array_destroy(ppja);

        _unlockResourcePool(pirp);
    }

    return u32Ret;
}

/** Destroy all resources in pool.
 *
 *  @param pirp [in] the pointer to the resource pool to be destroyed. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _destroyAllResources(internal_resource_pool_t * pirp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /* full time */
    jf_logger_logDebugMsg("destroy resource in fulltime array");
    u32Ret = _destroyResourceArray(pirp, &pirp->irp_pjaFulltimeResources);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_logger_logErrMsg(u32Ret,
            "failed to destroy full time resource array of resource pool %s",
            pirp->irp_strName);
    }

    /* part time */
    jf_logger_logDebugMsg("destroy resource in parttime array");
    u32Ret = _destroyResourceArray(pirp, &pirp->irp_pjaParttimeResources);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_logger_logErrMsg(u32Ret,
            "failed to destroy part time resource array of resource pool %s",
            pirp->irp_strName);
    }

    return u32Ret;
}

/** Create a resource pool according to the parameters.
 *
 *  @param ppirp [in/out] the pointer to the resource pool to be created and returned.
 *  @param pjrcp [in] the parameters for creating the resource pool.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_OUT_OF_MEMORY out of memeory
 *  @retval JF_ERR_INVALID_PARAM invalid parameter
 *
 */
static u32 _createResourcePool(
    internal_resource_pool_t ** ppirp, jf_respool_create_param_t * pjrcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp;

    u32Ret = jf_jiukun_allocMemory((void **)&pirp, sizeof(internal_resource_pool_t), 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_memset(pirp, 0, sizeof(internal_resource_pool_t));

        u32Ret = jf_mutex_init(&(pirp->irp_jmLock));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_array_create(&(pirp->irp_pjaFulltimeResources));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_array_create(&(pirp->irp_pjaParttimeResources));
    }
    
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pirp->irp_fnCreateResource = pjrcp->jrcp_fnCreateResource;
        pirp->irp_fnDestroyResource = pjrcp->jrcp_fnDestroyResource;

        pirp->irp_u32MaxResources = pjrcp->jrcp_u32MaxResources;
        pirp->irp_u32MinResources = pjrcp->jrcp_u32MinResources;

        pirp->irp_bImmediateRelease = pjrcp->jrcp_bImmediateRelease;
        
        ol_strcpy(pirp->irp_strName, pjrcp->jrcp_pstrName);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppirp = pirp;
    else
        jf_respool_destroy((jf_respool_t **)&pirp);

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
 *  @param pja [in] the array contains resources
 *  @param ppRes [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _getResourceFromPoolArray(
    internal_resource_pool_t * pirp, jf_array_t * pja,
    jf_respool_resource_t ** ppRes)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_t * pir;
    u32 u32Index, u32Size;
    boolean_t bFree;

    *ppRes = NULL;

    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Size = jf_array_getSize(pja);
        u32Index = 0;
        while ((u32Index < u32Size) && (u32Ret == JF_ERR_NO_ERROR))
        {
            u32Ret = jf_array_getElementAt(
                pja, u32Index, (jf_array_element_t **)&pir);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                bFree = _isPoolResourceFree(pir);
                if (bFree)
                {
                    jf_logger_logDebugMsg(
                        "find free resource from %s array",
                        (pir->ir_bFulltime ? "fulltime" : "parttime"));
                    _setPoolResourceState(pir, IRS_BUSY);
                    *ppRes = pir;
                    u32Ret = JF_ERR_NO_ERROR;
                    break;
                }
            }
            u32Index ++;
        }

        _unlockResourcePool(pirp);
    }

    if (*ppRes == NULL)
        u32Ret = JF_ERR_NOT_FOUND;
    
    return u32Ret;
}

/** Get resource from resource pool.
 *
 *  @param pirp [in] the pointer to internal resource pool
 *  @param ppRes [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _getResourceFromPool(
    internal_resource_pool_t * pirp, jf_respool_resource_t ** ppRes)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /* get resource from fulltime array */
    jf_logger_logDebugMsg("get resource from fulltime pool array");
    u32Ret = _getResourceFromPoolArray(
        pirp, pirp->irp_pjaFulltimeResources, ppRes);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_logger_logDebugMsg("get resource from parttime pool array");
        u32Ret = _getResourceFromPoolArray(
            pirp, pirp->irp_pjaParttimeResources, ppRes);
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        u32Ret = _createResourceInPool(pirp, ppRes);
    }
    
    return u32Ret;
}

/** Put resource in resource pool.
 *
 *  @param pirp [in] the pointer to internal resource pool
 *  @param ppjrr [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *
 */
static u32 _putResourceInPool(
    internal_resource_pool_t * pirp, jf_respool_resource_t ** ppjrr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_t * pir = (internal_resource_t *)*ppjrr;

    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (pir->ir_bFulltime)
        {
            _setPoolResourceState(pir, IRS_FREE);
        }
        else if (pirp->irp_bImmediateRelease)
        {
            jf_array_removeElement(
                pirp->irp_pjaParttimeResources, (jf_array_element_t *)pir);
            _destroyResourceInPool(pirp, &pir);
        }

        _unlockResourcePool(pirp);
    }

    *ppjrr = NULL;

    return u32Ret;
}

/** Find the free parttime resources, and release them.
 *
 *  @param pirp [in] the pointer to the internal resource pool. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
static u32 _reapResourceInPool(internal_resource_pool_t * pirp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_array_t * pja;
    internal_resource_t * pir;
    boolean_t bFree;
    u32 u32Index = 0, u32Size = 0;
    
    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /* the resource can be timed out */
        pja = pirp->irp_pjaParttimeResources;
        u32Index = 0;
        u32Size = jf_array_getSize(pja);
        while ((u32Ret == JF_ERR_NO_ERROR) && (u32Index < u32Size))
        {
            u32Ret = jf_array_getElementAt(pja, u32Index, (jf_array_element_t **)&pir);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /* try to destroy it if it free */
                bFree = _isPoolResourceFree(pir);
                if (bFree)
                {
                    pirp->irp_fnDestroyResource(pir, &pir->ir_pjrrdData);
                    jf_array_removeElementAt(pja, u32Index);
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

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_respool_create(
    jf_respool_t ** ppjr, jf_respool_create_param_t * pjrcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = NULL;

    assert((ppjr != NULL) && (pjrcp != NULL));

    jf_logger_logInfoMsg("create resource pool");

    u32Ret = _validateParam(pjrcp);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _createResourcePool(&pirp, pjrcp);

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppjr = pirp;
    else if (pirp != NULL)
        jf_respool_destroy((jf_respool_t **)&pirp);

    return u32Ret;
}

u32 jf_respool_destroy(jf_respool_t ** ppjr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = NULL;
    
    assert(ppjr != NULL);

    pirp = (internal_resource_pool_t *)*ppjr;
    *ppjr = NULL;

    jf_logger_logInfoMsg("destroy resource pool");

    /* destroy the resource list */
    _destroyAllResources(pirp);

    /* destroy the mutex */
    u32Ret = jf_mutex_fini(&(pirp->irp_jmLock));

    /* free the resource pool */
    jf_jiukun_freeMemory((void **)&pirp);

    return u32Ret;
}

u32 jf_respool_getResource(jf_respool_t * pjr, jf_respool_resource_t ** ppRes)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = (internal_resource_pool_t *)pjr;

    assert((pjr != NULL) && (ppRes != NULL));

    jf_logger_logDebugMsg("get resource from pool");
    
    /* get resource from pool */
    u32Ret = _getResourceFromPool(pirp, ppRes);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        u32Ret = _createResourceInPool(pirp, ppRes);
    }
    
    return u32Ret;
}

u32 jf_respool_putResource(jf_respool_t * pjr, jf_respool_resource_t ** ppRes)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = (internal_resource_pool_t *)pjr;;

    assert((pjr != NULL) && (ppRes != NULL));

    jf_logger_logDebugMsg("put resource in pool");
    
    u32Ret = _putResourceInPool(pirp, ppRes);

    return u32Ret;
}

u32 jf_respool_reapResource(jf_respool_t * pjr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp;
    
    assert(pjr != NULL);

    pirp = (internal_resource_pool_t *)pjr;
    u32Ret = _reapResourceInPool(pirp);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

