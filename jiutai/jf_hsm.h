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
#include "jf_basic.h"

/* --- constant definitions ------------------------------------------------ */

/** Last event ID
 */
#define JF_HSM_LAST_EVENT_ID     (U32_MAX)

/** Maximum state name length
 */
#define JF_HSM_MAX_STATE_NAME_LEN    (32)

/* --- data structures ----------------------------------------------------- */

typedef u32  jf_hsm_state_id_t;
typedef u32  jf_hsm_event_id_t;

struct jf_hsm_state;

typedef struct jf_hsm_event
{
    jf_hsm_event_id_t jhe_jheiEventId;
    void * jhe_pData;
} jf_hsm_event_t;

typedef boolean_t (* jf_hsm_fnEventGuard_t)(jf_hsm_event_t * pEvent);
typedef u32 (* jf_hsm_fnEventAction_t)(jf_hsm_event_t * pEvent);

typedef struct jf_hsm_transition
{
    jf_hsm_event_id_t jht_jheiEventId;
    jf_hsm_fnEventGuard_t jht_fnGuard;
    jf_hsm_fnEventAction_t jht_fnAction;
    struct jf_hsm_state * jht_pjhtNext;
} jf_hsm_transition_t;

typedef struct jf_hsm_state
{
    jf_hsm_state_id_t jhs_jhsiStateId;
    olchar_t jhs_strName[JF_HSM_MAX_STATE_NAME_LEN];
    jf_hsm_transition_t jhs_jhtTransition[];
} jf_hsm_state_t;

typedef struct
{
    jf_hsm_state_t * jh_pjhsInitial;
    jf_hsm_state_t * jh_pjhsCurrent;
} jf_hsm_t;

/* --- functional routines ------------------------------------------------- */

u32 jf_hsm_init(jf_hsm_t * pjh, jf_hsm_state_t * pInitial);

u32 jf_hsm_fini(jf_hsm_t * pjh);

jf_hsm_state_id_t jf_hsm_getCurrentStateId(jf_hsm_t * pjh);

jf_hsm_state_t * jf_hsm_getCurrentState(jf_hsm_t * pjh);

u32 jf_hsm_handleEvent(jf_hsm_t * pjh, jf_hsm_event_t * pEvent);

#endif /*JIUTAI_HSM_H*/

/*---------------------------------------------------------------------------*/


