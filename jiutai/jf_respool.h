/**
 *  @file jf_respool.h
 *
 *  @brief Resource pool header file. Resource pool contains some homogeneous
 *  resources, which can be statically or dynamically allocated and released.
 *
 *  @author Min Zhang
 *
 *  @note Routines declared in this file are included in jf_respool object
 *  @note link with common object: jf_mutex, jf_array
 *  @note link with jiukun library for memory allocation
 *  @note the object is thread safe
 *
 */

#ifndef JIUTAI_RESPOOL_H
#define JIUTAI_RESPOOL_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_logger.h"

/* --- constant definitions --------------------------------------------------------------------- */

/* --- data structures -------------------------------------------------------------------------- */

/** The resource pool type definition
 */
typedef void  jf_respool_t;

/** The resource type definition
 */
typedef void  jf_respool_resource_t;

/** The resource data when creating the resource
 */
typedef void  jf_respool_resource_data_t;

/** Create a resource according to the parameters.
 *
 *  @param pjrr [in] the pointer to the resource to be created and returned
 *  @param ppData [in/out] the pointer to the resource data
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_INVALID_PARAM invalid parameter
 *
 *  @note the function cannot be blocked, otherwise it blocks others who are getting resource from
 *   pool
 */
typedef u32 (* jf_respool_fnCreateResource_t)(
    jf_respool_resource_t * pjrr, jf_respool_resource_data_t ** ppData);

/** Destroy the resource.
 *  
 *  @param pjrr [in] the pointer to the resource to be destroyed. After destruction, it will be set
 *   to NULL.
 *  @param ppData [in/out] the pointer to the resource data
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *
 *  @note fnCreateResource_t must be called successfully before this func is called.
 */
typedef u32 (* jf_respool_fnDestroyResource_t)(
    jf_respool_resource_t * pjrr, jf_respool_resource_data_t ** ppData);

/**
 *  The parameter for creating the resource 
 */
typedef struct
{
    /** resource pool name, it should be no longer than 15 characters */
    olchar_t * jrcp_pstrName;
    /** minimum number of resources that should be allocated all the time */
    u32 jrcp_u32MinResources;
    /** maximum number of resources that can co-exist at the same time */
    u32 jrcp_u32MaxResources;
    /** if immediate release is set to true, resource is released after use*/
    boolean_t jrcp_bImmediateRelease;
    u8 jrcp_u8Reserved[7];
    /** the callback function is to creat resource */
    jf_respool_fnCreateResource_t jrcp_fnCreateResource;
    /** the callback function is to destroy resource */
    jf_respool_fnDestroyResource_t jrcp_fnDestroyResource;

    u32 jrcp_u32Reserved2[4];
} jf_respool_create_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create a resource pool according to the parameters.
 *
 *  @param ppjr [in/out] the pointer to the resource pool to be created and returned.
 *  @param pjrcp [in] the parameters for creating the resource pool.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *  @retval JF_ERR_OUT_OF_MEMORY out of memeory
 *  @retval JF_ERR_INVALID_PARAM invalid parameter
 *
 *  @note no resources are created in this function
 */
u32 jf_respool_create(jf_respool_t ** ppjr, jf_respool_create_param_t * pjrcp);

/** Destroy the resource pool.
 *
 *  @param ppjr [in/out]  the pointer to the resource pool to be destroyed. 
 *   After destruction, it will be set to NULL.
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_respool_destroy(jf_respool_t ** ppjr);

/** Get resource from resource pool.
 *
 *  @param pjr [in] the pointer to resource pool
 *  @param ppRes [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_respool_getResource(jf_respool_t * pjr, jf_respool_resource_t ** ppRes);

/** Put resource in resource pool.
 *
 *  @param pjr [in] the pointer to resource pool
 *  @param ppRes [in/out] the pointer to the resource. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *
 *  @note getResourceFromPool must be called successfully before this func is called.
 */
u32 jf_respool_putResource(jf_respool_t * pjr, jf_respool_resource_t ** ppRes);

/** Find the free parttime resources, and release them.
 *
 *  @param pjr [in] the pointer to the resource pool. 
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 */
u32 jf_respool_reapResource(jf_respool_t * pjr);

#endif /*JIUTAI_RESPOOL_H*/

/*------------------------------------------------------------------------------------------------*/

