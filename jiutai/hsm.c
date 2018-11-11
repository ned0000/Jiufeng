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

u32 initHsmStateMachine(hsm_statemachine_t * phs, hsm_state_t * pInitial)
{
    u32 u32Ret = OLERR_NO_ERROR;

    phs->hs_phsInitial = pInitial;
    phs->hs_phsCurrent = phs->hs_phsInitial;

    return u32Ret;
}

hsm_state_id_t getHsmCurrentStateId(hsm_statemachine_t * phs)
{
    return phs->hs_phsCurrent->hs_hsiStateId;
}

hsm_state_t * getHsmCurrentState(hsm_statemachine_t * phs)
{
    return phs->hs_phsCurrent;
}

u32 handleHsmEvent(hsm_statemachine_t * phs, hsm_event_t * pEvent)
{
    u32 u32Ret = OLERR_NO_ERROR;
    hsm_transition_t * pht;
    boolean_t bRet;

    pht = phs->hs_phsCurrent->hs_hrTransition;
    while (pht->ht_heiEventId != HSM_LAST_EVENT_ID)
    {
        if (pht->ht_heiEventId == pEvent->he_hsiEventId)
        {
            bRet = pht->ht_fnGuard(pEvent);
            if (bRet == TRUE)
            {
                u32Ret = pht->ht_fnAction(pEvent);
                phs->hs_phsCurrent = pht->ht_phsNext;
                break;
            }
        }

        pht ++;
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/

