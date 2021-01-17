/**
 *  @file jf_hsm.h
 *
 *  @brief HSM header file which defines interfaces of hierarchical state machine common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Link with jf_jiukun library for memory allocation.
 *  -# This object is NOT thread safe.
 *  -# Do not call jf_hsm_processEvent() to transit state in fnEventAction() as jf_hsm_processEvent()
 *     cannot be called recursively.
 *
 *  <HR>
 *
 *  @par Procedure for State Transition
 *  -# The procedure when state transition happens:
 *  @code
 *  if ((fnGuard == NULL) || fnGuard())
 *  {
 *      if (fnAction != NULL)
 *          fnAction()
 *      if ((new-state != old-state) &&
 *          (new-state != JF_HSM_LAST_STATE_ID))
 *      {
 *          fnOnExit(old-state)
 *          state = new-state
 *          fnOnEntry(new-state)
 *      }
 *  }
 *  @endcode
 *
 *  @par Rules for State Transition Table
 *  -# State may have a sub-state transition table.
 *  -# The event ID and state ID must be unique wherever it's in top level table or sub-table.
 *  -# The transition table must be ended by following line:
 *  @code
 *  {JF_HSM_LAST_STATE_ID, JF_HSM_LAST_EVENT_ID, NULL, NULL, JF_HSM_LAST_STATE_ID},
 *  @endcode
 *
 *  <HR>
 *
 */

#ifndef JIUTAI_HSM_H
#define JIUTAI_HSM_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"

/* --- constant definitions --------------------------------------------------------------------- */

/** Last state ID.
 */
#define JF_HSM_LAST_STATE_ID           (U32_MAX)

/** Last event ID.
 */
#define JF_HSM_LAST_EVENT_ID           (U32_MAX)

/* --- data structures -------------------------------------------------------------------------- */

/** Define the HSM state ID data type.
 */
typedef u32  jf_hsm_state_id_t;

/** Define the HSM event ID data type.
 */
typedef u32  jf_hsm_event_id_t;

/** Define the HSM event data type.
 */
typedef struct jf_hsm_event
{
    /**The event id.*/
    jf_hsm_event_id_t jhe_jheiEventId;
    /**The data associated with the event id.*/
    void * jhe_pData;
    /**The second parameter associated with the event id.*/
    s64 jhe_s64Param;
} jf_hsm_event_t;

/** The callback function to guard the state.
 */
typedef boolean_t (* jf_hsm_fnGuard_t)(jf_hsm_event_t * pEvent);

/** The callback function is executed when the guard function returns TRUE.
 */
typedef u32 (* jf_hsm_fnAction_t)(jf_hsm_event_t * pEvent);

/** The callback function when entering the state.
 */
typedef u32 (*jf_hsm_fnOnEntry)(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent);

/** The callback function when existing the state.
 */
typedef u32 (*jf_hsm_fnOnExit)(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent);

/** Define the state transition data type. 
 */
typedef struct jf_hsm_transition
{
    /**Current state id.*/
    jf_hsm_state_id_t jht_jhsiCurrentStateId;
    /**The event triggers the transition.*/
    jf_hsm_event_id_t jht_jheiEventId;
    /**The guard function, if TRUE is returned, fnAction is executed and transit to next state.*/
    jf_hsm_fnGuard_t jht_fnGuard;
    /**The action function.*/
    jf_hsm_fnAction_t jht_fnAction;
    /**Next state id, if next state id is JF_HSM_LAST_EVENT_ID, no state transition and the
       callback function fnOnEntry and fnOnExit are not called.*/
    jf_hsm_state_id_t jht_jhsiNextStateId;
} jf_hsm_transition_t;

/** Define the hsm data type. 
 */
typedef void  jf_hsm_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create the state machine.
 *
 *  @param ppHsm [out] The state machine to be created and returned.
 *  @param pTransition [out] The transition table.
 *  @param initialStateId [out] The initial state id for the transition table.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hsm_create(
    jf_hsm_t ** ppHsm, jf_hsm_transition_t * pTransition, jf_hsm_event_id_t initialStateId);

/** Destroy the state machine.
 *
 *  @param ppHsm [in/out] The state machine to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hsm_destroy(jf_hsm_t ** ppHsm);

/** Set the transition table for the specified state.
 *
 *  @param pHsm [in] The pointer to hsm object.
 *  @param stateId [in] The state id for the transition table.
 *  @param pTransition [in] The state transition table.
 *  @param initialStateId [in] The initial state id when entering the state.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hsm_addStateTransition(
    jf_hsm_t * pHsm, jf_hsm_state_id_t stateId, jf_hsm_transition_t * pTransition,
    jf_hsm_event_id_t initialStateId);

/** Add callback function for state.
 *
 *  @note
 *  -# The callback function is called when entering and exiting the state.
 *
 *  @param pHsm [in] The pointer to hsm object.
 *  @param stateId [in] The state id for the transition table.
 *  @param fnOnEntry [in] Callback function when entering the state.
 *  @param fnOnExit [in] Callback function when exiting the state.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hsm_addStateCallback(
    jf_hsm_t * pHsm, jf_hsm_state_id_t stateId, jf_hsm_fnOnEntry fnOnEntry,
    jf_hsm_fnOnExit fnOnExit);

/** Get current state id.
 *
 *  @param pHsm [in] The pointer to hsm object.
 *
 *  @return The current state id.
 */
jf_hsm_state_id_t jf_hsm_getCurrentStateId(jf_hsm_t * pHsm);

/** Process event according to the state transition table.
 *
 *  @param pHsm [in] The pointer to hsm object.
 *  @param pEvent [in] The event to be processed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 jf_hsm_processEvent(jf_hsm_t * pHsm, jf_hsm_event_t * pEvent);

/** Initialize the event data type.
 *
 *  @param pEvent [out] The event to be processed.
 *  @param eventId [in] The event id.
 *  @param pData [in] The data for the event.
 *  @param s64Param [in] The second parameter for the event.
 *
 *  @return Void.
 */
static inline void jf_hsm_initEvent(
    jf_hsm_event_t * pEvent, jf_hsm_event_id_t eventId, void * pData, s64 s64Param)
{
    pEvent->jhe_jheiEventId = eventId;
    pEvent->jhe_pData = pData;
    pEvent->jhe_s64Param = s64Param;
}

#endif /*JIUTAI_HSM_H*/

/*------------------------------------------------------------------------------------------------*/
