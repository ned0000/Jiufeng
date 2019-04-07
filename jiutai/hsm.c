/**
 *  @file hsm.c
 *
 *  @brief The hierarchical statemachine implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"
#include "hsm.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

u32 jf_hsm_init(jf_hsm_t * pjh, jf_hsm_state_t * pInitial)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pjh->jh_pjhsInitial = pInitial;
    pjh->jh_pjhsCurrent = pjh->jh_pjhsInitial;

    return u32Ret;
}

u32 jf_hsm_fini(jf_hsm_t * pjh)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    pjh->jh_pjhsInitial = NULL;
    pjh->jh_pjhsCurrent = NULL;

    return u32Ret;
}

jf_hsm_state_id_t jf_hsm_getCurrentStateId(jf_hsm_t * pjh)
{
    return pjh->jh_pjhsCurrent->jhs_jhsiStateId;
}

jf_hsm_state_t * jf_hsm_getCurrentState(jf_hsm_t * pjh)
{
    return pjh->jh_pjhsCurrent;
}

u32 jf_hsm_handleEvent(jf_hsm_t * pjh, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hsm_transition_t * pjht;
    boolean_t bRet;

    pjht = pjh->jh_pjhsCurrent->jhs_jhtTransition;
    while (pjht->jht_jheiEventId != HSM_LAST_EVENT_ID)
    {
        if (pjht->jht_jheiEventId == pEvent->jhe_jheiEventId)
        {
            bRet = pjht->jht_fnGuard(pEvent);
            if (bRet == TRUE)
            {
                u32Ret = pjht->jht_fnAction(pEvent);
                pjh->jh_pjhsCurrent = pjht->jht_pjhtNext;
                break;
            }
        }

        pjht ++;
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

