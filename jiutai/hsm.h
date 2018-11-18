/**
 *  @file hsm.h
 *
 *  @brief HSM header file. Interfaces for hierarchical state machine
 *
 *  @author Min Zhang
 *
 *  @note
 */

#ifndef JIUTAI_HSM_H
#define JIUTAI_HSM_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */

typedef u32  hsm_state_id_t;
typedef u32  hsm_event_id_t;

struct hsm_state;

typedef struct hsm_event
{
    hsm_event_id_t he_hsiEventId;
    void * he_pData;
} hsm_event_t;

typedef boolean_t (* fnHsmEventGuard_t)(hsm_event_t * pEvent);
typedef u32 (* fnHsmEventAction_t)(hsm_event_t * pEvent);

typedef struct hsm_transition
{
#define HSM_LAST_EVENT_ID     (U32_MAX)
    hsm_event_id_t ht_heiEventId;
    fnHsmEventGuard_t ht_fnGuard;
    fnHsmEventAction_t ht_fnAction;
    struct hsm_state * ht_phsNext;
} hsm_transition_t;

typedef struct hsm_state
{
    hsm_state_id_t hs_hsiStateId;
#define MAX_HSM_STATE_NAME_LEN    (32)
    olchar_t hs_strName[MAX_HSM_STATE_NAME_LEN];
    hsm_transition_t hs_hrTransition[];
} hsm_state_t;

typedef struct
{
    hsm_state_t * hs_phsInitial;
    hsm_state_t * hs_phsCurrent;
} hsm_statemachine_t;

/* --- functional routines ------------------------------------------------- */

u32 initHsmStateMachine(hsm_statemachine_t * phs, hsm_state_t * pInitial);

hsm_state_id_t getHsmCurrentStateId(hsm_statemachine_t * phs);

hsm_state_t * getHsmCurrentState(hsm_statemachine_t * phs);

u32 handleHsmEvent(hsm_statemachine_t * phs, hsm_event_t * pEvent);

#endif /*JIUTAI_HSM_H*/

/*---------------------------------------------------------------------------*/


