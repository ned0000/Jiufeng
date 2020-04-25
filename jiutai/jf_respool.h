/**
 *  @file jf_respool.h
 *
 *  @brief Resource pool header file. Resource pool contains some homogeneous resources, which can
 *  be statically or dynamically allocated and released.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_respool object.
 *  -# Link with jf_mutex and jf_array common object.
 *  -# Link with jiukun library for memory allocation.
 *  -# The object is thread safe.
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

/** Define the resource pool data type.
 */
typedef void  jf_respool_t;

/** Define The resource data type.
 */
typedef void  jf_respool_resource_t;

/** Define resource data when creating the resource.
 */
typedef void  jf_respool_resource_data_t;

/** Create a resource according to the parameters.
 *
 *  @note
 *  -# The function cannot be blocked, otherwise it blocks others who are getting resource from
 *     pool.
 *
 *  @param pjrr [in] The pointer to the resource to be created and returned.
 *  @param ppData [out] The pointer to the resource data.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_INVALID_PARAM Invalid parameter.
 */
typedef u32 (* jf_respool_fnCreateResource_t)(
    jf_respool_resource_t * pjrr, jf_respool_resource_data_t ** ppData);

/** Destroy the resource.
 *  
 *  @param pjrr [in] The pointer to the resource to be destroyed.
 *  @param ppData [in/out] The pointer to the resource data.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_respool_fnDestroyResource_t)(
    jf_respool_resource_t * pjrr, jf_respool_resource_data_t ** ppData);

/** The parameter for creating the resource.
 *
 *  @note
 *  -# The minimum number of resources is full time resources which are not released after use.
 *   Other resources are part time resource which are released after use.
 */
typedef struct
{
    /**Resource pool name, it should be no longer than 15 characters.*/
    olchar_t * jrcp_pstrName;
    /**Minimum number of resources that should be allocated all the time.*/
    u32 jrcp_u32MinResources;
    /**Maximum number of resources that can co-exist at the same time.*/
    u32 jrcp_u32MaxResources;
    u8 jrcp_u8Reserved[8];
    /**The callback function is to creat resource.*/
    jf_respool_fnCreateResource_t jrcp_fnCreateResource;
    /**The callback function is to destroy resource.*/
    jf_respool_fnDestroyResource_t jrcp_fnDestroyResource;

    u32 jrcp_u32Reserved2[4];
} jf_respool_create_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create a resource pool according to the parameters.
 *
 *  @note
 *  -# No resources are created in this function.
 *
 *  @param ppjr [in/out] The pointer to the resource pool to be created and returned.
 *  @param pjrcp [in] The parameters for creating the resource pool.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_OUT_OF_MEMORY Out of memeory.
 *  @retval JF_ERR_INVALID_PARAM Invalid parameter.
 */
u32 jf_respool_create(jf_respool_t ** ppjr, jf_respool_create_param_t * pjrcp);

/** Destroy the resource pool.
 *
 *  @param ppjr [in/out] The pointer to the resource pool to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_respool_destroy(jf_respool_t ** ppjr);

/** Get resource from resource pool.
 *
 *  @param pjr [in] The pointer to resource pool.
 *  @param ppRes [in/out] The pointer to the resource. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_respool_getResource(jf_respool_t * pjr, jf_respool_resource_t ** ppRes);

/** Put resource in resource pool.
 *
 *  @note
 *  -# jf_respool_getResource() must be called successfully before this func is called.
 *
 *  @param pjr [in] The pointer to resource pool.
 *  @param ppRes [in/out] The pointer to the resource. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_respool_putResource(jf_respool_t * pjr, jf_respool_resource_t ** ppRes);

#endif /*JIUTAI_RESPOOL_H*/

/*------------------------------------------------------------------------------------------------*/

