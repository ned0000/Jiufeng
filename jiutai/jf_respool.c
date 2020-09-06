/**
 *  @file jf_respool.c
 *
 *  @brief Resource pool implementation file.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_respool.h"
#include "jf_logger.h"
#include "jf_err.h"
#include "jf_array.h"
#include "jf_mutex.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the internal resource state data type.
 */
typedef enum
{
    /**The resource is free.*/
    IRS_FREE = 0,
    /**The resource is reserved.*/
    IRS_RESERVED,
    /**The resource is busy.*/
    IRS_BUSY,
} internal_resource_state_t;

struct internal_resource_pool;

/** Define the internal resource data type.
 */
typedef struct
{
    /**Full time resource if it's TRUE; otherwise part time resource.*/
    boolean_t ir_bFulltime;
    u8 ir_u8Reserved[7];
    /**Pointer to the resource pool.*/
    struct internal_resource_pool * ir_pirpPool;
    /**The resource state.*/
    internal_resource_state_t ir_irsResourceState;
    /**The data of the resource.*/
    jf_respool_resource_data_t * ir_pjrrdData;
} internal_resource_t;

/** Define the internal respource pool data type.
 */
typedef struct internal_resource_pool
{
    /**Null-terminated string. It should be no longer than 15 characters.*/
    olchar_t irp_strName[16];
    /**Minimum number of resources that should be allocated all the time.*/
    u32 irp_u32MinResources;
    /**Maximum number of resources that can co-exist at the same time.*/
    u32 irp_u32MaxResources;
    /**Release the parttime resource immediately after use.*/
    boolean_t irp_bImmediateRelease;
    u8 irp_u8Reserved[7];
    /**Synchronize the access to the resources.*/
    jf_mutex_t irp_jmLock;
    /**Array contains full time resources.*/
    jf_array_t * irp_pjaFulltimeResources;
    /**Array contains part time resources.*/    
    jf_array_t * irp_pjaParttimeResources;

    /**The callback function to create resource.*/
    jf_respool_fnCreateResource_t irp_fnCreateResource;
    /**The callback function to destroy resource.*/
    jf_respool_fnDestroyResource_t irp_fnDestroyResource;

} internal_resource_pool_t;

/* --- private routine section ------------------------------------------------------------------ */

/** Validate resource pool parameter.
 *
 *  @param pjrcp [in] The pointer to the parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_PARAM Invalid parameter.
 */
static u32 _validateParam(jf_respool_create_param_t * pjrcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pjrcp->jrcp_u32MinResources > pjrcp->jrcp_u32MaxResources)
    {
        u32Ret = JF_ERR_INVALID_PARAM;
    }
    else
    {
        if ((pjrcp->jrcp_fnCreateResource == NULL) || (pjrcp->jrcp_fnDestroyResource == NULL))
        {
            u32Ret = JF_ERR_INVALID_PARAM;
        }
    }

    return u32Ret;
}

/** Lock resource pool.
 *
 *  @param pirp [in] The pointer to the resource pool. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _lockResourcePool(internal_resource_pool_t * pirp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_mutex_acquire(&pirp->irp_jmLock);

    return u32Ret;
}

/** Unlock resource pool.
 *
 *  @param pirp [in] The pointer to the resource pool. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _unlockResourcePool(internal_resource_pool_t * pirp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = jf_mutex_release(&pirp->irp_jmLock);

    return u32Ret;
}

/** Destroy a resource.
 *
 *  @param pirp [in] The pointer to the resource pool. 
 *  @param ppir [in/out] The pointer to the resource to be destroyed. After destruction, it will be
 *   set to NULL.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _destroyResourceInPool(internal_resource_pool_t * pirp, internal_resource_t ** ppir)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_t * pir = (internal_resource_t *)*ppir;

    JF_LOGGER_DEBUG("destroy resource");

    u32Ret = pirp->irp_fnDestroyResource((jf_respool_resource_t *)pir, &pir->ir_pjrrdData);
    
    jf_jiukun_freeMemory((void **)ppir);

    return u32Ret;
}

static u32 _fnDestroyArrayResource(jf_array_element_t ** ppjae)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_t * pir = (internal_resource_t *) *ppjae;
    internal_resource_pool_t * pirp = pir->ir_pirpPool;

    u32Ret = _destroyResourceInPool(pirp, (internal_resource_t **)ppjae);

    return u32Ret;
}

/** Check whether the resource is free.
 *
 *  @param pir [in] The pointer to the resource. 
 *
 *  @return The free state of the resource.
 *  @retval TRUE the resource is free.
 *  @retval FALSE the resource is not free.
 */
static boolean_t _isPoolResourceFree(internal_resource_t * pir)
{
    boolean_t bFree = FALSE;

    if (pir->ir_irsResourceState == IRS_FREE)
        bFree = TRUE;

    return bFree;
}

/** Set the state of the resource.
 *
 *  @param pir [in] The pointer to the resource.
 *  @param irs [in] The new state to be set.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _setPoolResourceState(
    internal_resource_t * pir, internal_resource_state_t irs)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pir->ir_irsResourceState = irs;

    return u32Ret;
}

static boolean_t _fnFindArrayResource(jf_array_element_t * pjae, void * pKey)
{
    boolean_t bFound = FALSE, bFree = FALSE;
    internal_resource_t * pir = (internal_resource_t *) pjae;

    JF_LOGGER_DEBUG("find array resource");

    bFree = _isPoolResourceFree(pir);
    if (bFree)
    {
        JF_LOGGER_DEBUG("from %s array", (pir->ir_bFulltime ? "fulltime" : "parttime"));
        _setPoolResourceState(pir, IRS_BUSY);
        bFound = TRUE;
    }

    return bFound;
}

/** Check whether the maximum resources are reached in array.
 *
 *  @param pirp [in] The pointer to the resource pool.
 *  @param bFulltime [in] specify which array should be checked.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_REACH_MAX_RESOURCES Reach maximum resources.
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
            JF_LOGGER_DEBUG("max fulltime resource is reached");
            u32Ret = JF_ERR_REACH_MAX_RESOURCES;
        }
    }
    else
    {
        u32Size = jf_array_getSize(pirp->irp_pjaParttimeResources);
        if (u32Size == pirp->irp_u32MaxResources - pirp->irp_u32MinResources)
        {
            JF_LOGGER_DEBUG("max parttime resource is reached");
            u32Ret = JF_ERR_REACH_MAX_RESOURCES;
        }
    }

    return u32Ret;
}

/** Create a resource and add it to the array.
 *
 *  @param pirp [in] The pointer to the resource pool.
 *  @param bFulltime [in] specify if the resource is fulltime or parttime.
 *  @param state [in] The resource state.
 *  @param ppjrr [in/out] The pointer to the resource to be created and returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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

    JF_LOGGER_DEBUG("%s array", (bFulltime ? "fulltime" : "parttime"));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _lockResourcePool(pirp);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _isMaxPoolResourcesReached(pirp, bFulltime);

        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = jf_jiukun_allocMemory((void **)&pir, sizeof(internal_resource_t));

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_bzero(pir, sizeof(internal_resource_t));

            pir->ir_bFulltime = bFulltime;
            pir->ir_pirpPool = pirp;
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

/** Create resource.
 *
 *  @param pirp [in] The pointer to the resource pool.
 *  @param ppjrr [in/out] The pointer to the resource to be created.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 *  @retval JF_ERR_INVALID_PARAM Invalid parameter.
 */
static u32 _createResourceInPool(
    internal_resource_pool_t * pirp, jf_respool_resource_t ** ppjrr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Try to create a fulltime resource.*/
    u32Ret = _createResourceInPoolArray(pirp, TRUE, IRS_BUSY, ppjrr);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        /*Try to create a parttime resource if fulltime resource cannot be created.*/
        u32Ret = _createResourceInPoolArray(pirp, FALSE, IRS_BUSY, ppjrr);
    }

    return u32Ret;    
}

/** Destroy all resources in array
 *
 *  @param pirp [in] The pointer to the resource pool to be destroyed. 
 *  @param ppja [in/out] The pointer to the array to be destroyed. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _destroyResourceArray(internal_resource_pool_t * pirp, jf_array_t ** ppja)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_array_destroyArrayAndElements(ppja, _fnDestroyArrayResource);

        _unlockResourcePool(pirp);
    }

    return u32Ret;
}

/** Destroy all resources in pool.
 *
 *  @param pirp [in] The pointer to the resource pool to be destroyed. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _destroyAllResources(internal_resource_pool_t * pirp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    JF_LOGGER_INFO("destroy all resources");

    /*Destroy the full time resource.*/
    u32Ret = _destroyResourceArray(pirp, &pirp->irp_pjaFulltimeResources);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        JF_LOGGER_ERR(u32Ret, "failed to destroy full time resource %s", pirp->irp_strName);
    }

    /*Destroy the part time resource.*/
    u32Ret = _destroyResourceArray(pirp, &pirp->irp_pjaParttimeResources);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        JF_LOGGER_ERR(u32Ret, "failed to destroy part time resource %s", pirp->irp_strName);
    }

    return u32Ret;
}

/** Create a resource pool according to the parameters.
 *
 *  @param ppirp [in/out] The pointer to the resource pool to be created and returned.
 *  @param pjrcp [in] The parameters for creating the resource pool.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 *  @retval JF_ERR_INVALID_PARAM Invalid parameter.
 *
 */
static u32 _createResourcePool(
    internal_resource_pool_t ** ppirp, jf_respool_create_param_t * pjrcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = NULL;

    /*Allocate memory for resource pool.*/
    u32Ret = jf_jiukun_allocMemory((void **)&pirp, sizeof(internal_resource_pool_t));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(pirp, sizeof(internal_resource_pool_t));

        u32Ret = jf_mutex_init(&pirp->irp_jmLock);
    }

    /*Create full time resource array.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_array_create(&pirp->irp_pjaFulltimeResources);

    /*Create part time resource array.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_array_create(&pirp->irp_pjaParttimeResources);
    
    /*Set the resource pool.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pirp->irp_fnCreateResource = pjrcp->jrcp_fnCreateResource;
        pirp->irp_fnDestroyResource = pjrcp->jrcp_fnDestroyResource;

        pirp->irp_u32MaxResources = pjrcp->jrcp_u32MaxResources;
        pirp->irp_u32MinResources = pjrcp->jrcp_u32MinResources;

        /*Set to TRUE so part time resource is released after use.*/
        pirp->irp_bImmediateRelease = TRUE;
        
        ol_strncpy(pirp->irp_strName, pjrcp->jrcp_pstrName, sizeof(pirp->irp_strName) - 1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppirp = pirp;
    else if (pirp != NULL)
        jf_respool_destroy((jf_respool_t **)&pirp);

    return u32Ret;
}

/** Get resource from array in resource pool.
 *
 *  @param pirp [in] The pointer to internal resource pool.
 *  @param pja [in] The array contains resources.
 *  @param ppRes [in/out] The pointer to the resource. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _getResourceFromPoolArray(
    internal_resource_pool_t * pirp, jf_array_t * pja, jf_respool_resource_t ** ppRes)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    *ppRes = NULL;

    u32Ret = _lockResourcePool(pirp);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_array_findElement(
            pja, (jf_array_element_t **)ppRes, _fnFindArrayResource, NULL);

        _unlockResourcePool(pirp);
    }

    return u32Ret;
}

/** Get resource from resource pool.
 *
 *  @param pirp [in] The pointer to internal resource pool.
 *  @param ppRes [in/out] The pointer to the resource. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _getResourceFromPool(
    internal_resource_pool_t * pirp, jf_respool_resource_t ** ppRes)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Get resource from full time array.*/
    u32Ret = _getResourceFromPoolArray(pirp, pirp->irp_pjaFulltimeResources, ppRes);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        /*Get resource from partime time array.*/
        u32Ret = _getResourceFromPoolArray(pirp, pirp->irp_pjaParttimeResources, ppRes);
    }

    /*Create resource if no resource is available.*/
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        u32Ret = _createResourceInPool(pirp, ppRes);
    }
    
    return u32Ret;
}

/** Put resource in resource pool.
 *
 *  @param pirp [in] The pointer to internal resource pool.
 *  @param ppjrr [in/out] The pointer to the resource. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
            /*Change the state to free for full time resource.*/
            _setPoolResourceState(pir, IRS_FREE);
        }
        else if (pirp->irp_bImmediateRelease)
        {
            /*Destroy the resouce if it's part time.*/
            u32Ret = jf_array_removeElement(
                pirp->irp_pjaParttimeResources, (jf_array_element_t *)pir);
            if (u32Ret == JF_ERR_NO_ERROR)
                _destroyResourceInPool(pirp, &pir);
        }

        _unlockResourcePool(pirp);
    }

    *ppjrr = NULL;

    return u32Ret;
}

/** Find the free part time resources, and release them.
 *
 *  @param pirp [in] The pointer to the internal resource pool. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
        /*The resource can be timed out.*/
        pja = pirp->irp_pjaParttimeResources;
        u32Index = 0;
        u32Size = jf_array_getSize(pja);
        while ((u32Ret == JF_ERR_NO_ERROR) && (u32Index < u32Size))
        {
            u32Ret = jf_array_getElementAt(pja, u32Index, (jf_array_element_t **)&pir);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                /*Try to destroy it if it free.*/
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

u32 jf_respool_create(jf_respool_t ** ppjr, jf_respool_create_param_t * pjrcp)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = NULL;

    assert((ppjr != NULL) && (pjrcp != NULL));

    JF_LOGGER_INFO(
        "pool: %s, min: %u, max: %u", pjrcp->jrcp_pstrName, pjrcp->jrcp_u32MinResources,
        pjrcp->jrcp_u32MinResources);

    /*Validate the parameter.*/
    u32Ret = _validateParam(pjrcp);

    /*Create the resource pool.*/
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

    JF_LOGGER_INFO("pool: %s", pirp->irp_strName);

    /*Destroy all resources.*/
    _destroyAllResources(pirp);

    /*Destroy the mutex.*/
    jf_mutex_fini(&(pirp->irp_jmLock));

    /*Free the resource pool.*/
    jf_jiukun_freeMemory((void **)&pirp);

    return u32Ret;
}

u32 jf_respool_getResource(jf_respool_t * pjr, jf_respool_resource_t ** ppRes)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = (internal_resource_pool_t *)pjr;

    assert((pjr != NULL) && (ppRes != NULL));

    JF_LOGGER_DEBUG("pool: %s", pirp->irp_strName);
    
    /*Get resource from pool.*/
    u32Ret = _getResourceFromPool(pirp, ppRes);
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        /*Create resource if no resouce is available.*/
        u32Ret = _createResourceInPool(pirp, ppRes);
    }
    
    return u32Ret;
}

u32 jf_respool_putResource(jf_respool_t * pjr, jf_respool_resource_t ** ppRes)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = (internal_resource_pool_t *)pjr;

    assert((pjr != NULL) && (ppRes != NULL));

    JF_LOGGER_DEBUG("pool: %s", pirp->irp_strName);
    
    u32Ret = _putResourceInPool(pirp, ppRes);

    return u32Ret;
}

u32 jf_respool_reapResource(jf_respool_t * pjr)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_resource_pool_t * pirp = (internal_resource_pool_t *)pjr;
    
    assert(pjr != NULL);

    JF_LOGGER_DEBUG("pool: %s", pirp->irp_strName);

    u32Ret = _reapResourceInPool(pirp);

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
