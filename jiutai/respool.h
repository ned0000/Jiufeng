/**
 *  @file respool.h
 *
 *  @brief Resource pool header file. Resource pool contains some homogeneous
 *  resources, which can be statically or dynamically allocated and released.
 *
 *  @author Min Zhang
 *  
 *  @note link with common object: syncmutex, xmalloc, array
 *  @note the object is thread safe
 *
 */

#ifndef JIUTAI_RESPOOL_H
#define JIUTAI_RESPOOL_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "logger.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
/**
 *  The resource type definition
 */
typedef void  resource_t;
/**
 *  The resource pool type definition
 */
typedef void  resource_pool_t;
/**
 *  The resource data when creating the resource
 */
typedef void  resource_data_t;


/** Create a resource according to the parameters.
 *
 *  @param pr [in] the pointer to the resource to be created and returned
 *  @param ppData [in/out] the pointer to the resource data
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_INVALID_PARAM invalid parameter
 *
 *  @note the function cannot be blocked, otherwise it blocks others who are
 *   getting resource from pool
 */
typedef u32 (* fnCreateResource_t)(resource_t * pr, resource_data_t ** ppData);

/** Destroy the resource.
 *  
 *  @param pr [in] the pointer to the resource to be destroyed. 
 *   After destruction, it will be set to NULL.
 *  @param ppData [in/out] the pointer to the resource data
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *
 *  @note fnCreateResource_t must be called successfully before this func is
 *   called.
 */
typedef u32 (* fnDestroyResource_t)(resource_t * pr, resource_data_t ** ppData);

/**
 *  The parameter for creating the resource 
 */
typedef struct
{
    /** resource pool name, it should be no longer than 15 characters */
    olchar_t * rpp_pstrName;
    /** minimum number of resources that should be allocated all the time */
    u32 rpp_u32MinResources;
    /** maximum number of resources that can co-exist at the same time */
    u32 rpp_u32MaxResources;
    /** if immediate release is set to true, resource is released after use*/
    boolean_t rpp_bImmediateRelease;
    u8 rpp_u8Reserved[7];
    /** the callback function is to creat resource */
    fnCreateResource_t rpp_fnCreateResource;
    /** the callback function is to destroy resource */
    fnDestroyResource_t rpp_fnDestroyResource;

    u32 rpp_u32Reserved2[4];
} resource_pool_param_t;

/* --- functional routines ------------------------------------------------- */

/** Create a resource pool according to the parameters.
 *
 *  @param pprp [in/out] the pointer to the resource pool to be created and returned.
 *  @param prpp [in] the parameters for creating the resource pool.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *  @retval OLERR_OUT_OF_MEMORY out of memeory
 *  @retval OLERR_INVALID_PARAM invalid parameter
 *
 *  @note no resources are created in this function
 */
u32 createResourcePool(resource_pool_t ** pprp, resource_pool_param_t * prpp);

/** Destroy the resource pool.
 *
 *  @param pprp [in/out]  the pointer to the resource pool to be destroyed. 
 *   After destruction, it will be set to NULL.
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 destroyResourcePool(resource_pool_t ** pprp);

/** Get resource from resource pool.
 *
 *  @param prp [in] the pointer to resource pool
 *  @param ppRes [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 getResourceFromPool(resource_pool_t * prp, resource_t ** ppRes);

/** Put resource in resource pool.
 *
 *  @param prp [in] the pointer to resource pool
 *  @param ppRes [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 *
 *  @note getResourceFromPool must be called successfully before this func is
 *   called.
 */
u32 putResourceInPool(resource_pool_t * prp, resource_t ** ppRes);

/** Find the free parttime resources, and release them.
 *
 *  @param prp [in] the pointer to the resource pool. 
 *
 *  @return the error code
 *  @retval OLERR_NO_ERROR success
 */
u32 reapResourceInPool(resource_pool_t * prp);

#endif /*JIUTAI_RESPOOL_H*/

/*---------------------------------------------------------------------------*/

