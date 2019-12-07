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
 *  if ((fnEventGuard == NULL) || fnEventGuard())
 *  {
 *      if (fnEventAction != NULL)
 *          fnEventAction()
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
#define JF_HSM_LAST_STATE_ID     (U32_MAX)

/** Last event ID.
 */
#define JF_HSM_LAST_EVENT_ID     (U32_MAX)

/* --- data structures -------------------------------------------------------------------------- */

/** The HSM state ID.
 */
typedef u32  jf_hsm_state_id_t;

/** The HSM event ID.
 */
typedef u32  jf_hsm_event_id_t;

/** The HSM event data structure.
 */
typedef struct jf_hsm_event
{
    /**The event id.*/
    jf_hsm_event_id_t jhe_jheiEventId;
    /**The data associated with the event id.*/
    void * jhe_pData;
    /**The second data associated with the event id.*/
    void * jhe_pDataEx;
} jf_hsm_event_t;

typedef boolean_t (* jf_hsm_fnEventGuard_t)(jf_hsm_event_t * pEvent);
typedef u32 (* jf_hsm_fnEventAction_t)(jf_hsm_event_t * pEvent);
typedef u32 (*jf_hsm_fnOnEntry)(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent);
typedef u32 (*jf_hsm_fnOnExit)(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent);

/** The state transition data structure. 
 */
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
    /**Next state id, if next state id is JF_HSM_LAST_EVENT_ID, no state transition and the
       callback function fnOnEntry and fnOnExit are not called*/
    jf_hsm_state_id_t jht_jhsiNextStateId;
} jf_hsm_transition_t;

typedef void  jf_hsm_t;

/* --- functional routines ---------------------------------------------------------------------- */

u32 jf_hsm_create(
    jf_hsm_t ** ppHsm, jf_hsm_transition_t * pTransition, jf_hsm_event_id_t initialStateId);

u32 jf_hsm_destroy(jf_hsm_t ** ppHsm);

/** Set the transition table for the specified state
 *
 *  @param pHsm [in] the pointer to hsm object
 *  @param stateId [in] the state id for the transition table
 *  @param pTransition [in] the state transition table
 *  @param initialStateId [in] the initial state id when entering the state
 *
 *  @return the error code
 *  @retval JF_ERR_NO_ERROR success
 *
 */
u32 jf_hsm_addStateTransition(
    jf_hsm_t * pHsm, jf_hsm_state_id_t stateId, jf_hsm_transition_t * pTransition,
    jf_hsm_event_id_t initialStateId);

u32 jf_hsm_addStateCallback(
    jf_hsm_t * pHsm, jf_hsm_state_id_t stateId, jf_hsm_fnOnEntry fnOnEntry,
    jf_hsm_fnOnExit fnOnExit);

jf_hsm_state_id_t jf_hsm_getCurrentStateId(jf_hsm_t * pjh);

u32 jf_hsm_processEvent(jf_hsm_t * pjh, jf_hsm_event_t * pEvent);

static inline void jf_hsm_initEvent(
    jf_hsm_event_t * pEvent, jf_hsm_event_id_t eventId, void * pData, void * pDataEx)
{
    pEvent->jhe_jheiEventId = eventId;
    pEvent->jhe_pData = pData;
    pEvent->jhe_pDataEx = pDataEx;
}


#endif /*JIUTAI_HSM_H*/

/*------------------------------------------------------------------------------------------------*/


