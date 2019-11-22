/**
 *  @file jf_hsm.h
 *
 *  @brief HSM header file. Interfaces for hierarchical state machine
 *
 *  @author Min Zhang
 *
 *  @note Link with jf_jiukun library for memory allocation
 *  @note This object is not thread safe
 *  @note The procedure for state transition \n
 *   if ((fnEventGuard == NULL) || fnEventGuard())         \n
 *   {                           \n
 *       fnEventAction()         \n
 *       fnOnExit(old-state)     \n
 *       state = new-state       \n
 *       fnOnEntry(new-state)    \n
 *   }                           \n
 */

#ifndef JIUTAI_HSM_H
#define JIUTAI_HSM_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Last state ID
 */
#define JF_HSM_LAST_STATE_ID     (U32_MAX)

/** Last event ID
 */
#define JF_HSM_LAST_EVENT_ID     (U32_MAX)

/** Maximum state name length
 */
#define JF_HSM_MAX_STATE_NAME_LEN    (32)

/* --- data structures -------------------------------------------------------------------------- */

typedef u32  jf_hsm_state_id_t;
typedef u32  jf_hsm_event_id_t;

typedef struct jf_hsm_event
{
    jf_hsm_event_id_t jhe_jheiEventId;
    void * jhe_pData;
} jf_hsm_event_t;

typedef boolean_t (* jf_hsm_fnEventGuard_t)(jf_hsm_event_t * pEvent);
typedef u32 (* jf_hsm_fnEventAction_t)(jf_hsm_event_t * pEvent);
typedef u32 (*jf_hsm_fnOnEntry)(jf_hsm_state_id_t stateId);
typedef u32 (*jf_hsm_fnOnExit)(jf_hsm_state_id_t stateId);

typedef struct jf_hsm_transition
{
    /**Current state id*/
    jf_hsm_state_id_t jht_jhsiCurrentStateId;
    /**The event triggers the transition*/
    jf_hsm_event_id_t jht_jheiEventId;
    /**The guard function, if TRUE is returned, fnAction is executed and transit to next state*/
    jf_hsm_fnEventGuard_t jht_fnGuard;
    /**Action function*/
    jf_hsm_fnEventAction_t jht_fnAction;
    /**Next state id*/
    jf_hsm_state_id_t jht_jhsiNextStateId;
} jf_hsm_transition_t;

typedef void  jf_hsm_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 jf_hsm_create(
    jf_hsm_t ** ppHsm, jf_hsm_transition_t * pTransition, jf_hsm_event_id_t initialStateId);

u32 jf_hsm_destroy(jf_hsm_t ** ppHsm);

/** Set the transition table for the state
 *
 *  @param pHsm [in] the pointer to hsm object
 *  @param stateId [in] the state id for the transition table
 *  @param pTransition [in] the state transition table
 *  @param initialStateId [in] the initial state id when entering the state
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *
 *  @note getResourceFromPool must be called successfully before this func is called.
 */
u32 jf_hsm_addStateTransition(
    jf_hsm_t * pHsm, jf_hsm_state_id_t stateId, jf_hsm_transition_t * pTransition,
    jf_hsm_event_id_t initialStateId);

u32 jf_hsm_addStateCallback(
    jf_hsm_t * pHsm, jf_hsm_state_id_t stateId, jf_hsm_fnOnEntry fnOnEntry,
    jf_hsm_fnOnExit fnOnExit);

jf_hsm_state_id_t jf_hsm_getCurrentStateId(jf_hsm_t * pjh);

u32 jf_hsm_processEvent(jf_hsm_t * pjh, jf_hsm_event_t * pEvent);

#endif /*JIUTAI_HSM_H*/

/*------------------------------------------------------------------------------------------------*/


