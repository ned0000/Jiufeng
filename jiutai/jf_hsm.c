/**
 *  @file jf_hsm.c
 *
 *  @brief The hierarchical statemachine implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_hsm.h"
#include "jf_jiukun.h"
#include "jf_listhead.h"

/* --- private data/data structure section ------------------------------------------------------ */

typedef struct internal_hsm_state_transition_table
{
    /**The state id for the transition table, if the state id is JF_HSM_LAST_STATE_ID, the state 
     transition table is the top table, otherwise it's the table for the state*/
    jf_hsm_state_id_t ihstt_jhsiStateId;
    /**The initial state id, current state is set to initial state id when entering this state*/
    jf_hsm_state_id_t ihstt_jhsiInitialStateId;
    /**linked list for state definition*/
    jf_listhead_t ihstt_jlList;
    /**Current state id*/
    jf_hsm_state_id_t ihstt_jhsiCurrentStateId;
    /**State transition table*/
    jf_hsm_transition_t ihstt_jhtTransition[];
} internal_hsm_state_transition_table_t;

typedef struct internal_hsm_state_callback
{
    jf_hsm_state_id_t ihsc_jhsiStateId;

    jf_hsm_fnOnEntry ihsc_fnOnEntry;
    jf_hsm_fnOnExit ihsc_fnOnExit;

    jf_listhead_t ihsc_jlList;
} internal_hsm_state_callback_t;

typedef struct
{

    jf_listhead_t ih_jlStateTransitionTable;

    jf_listhead_t ih_jlStateCallback;

} internal_hsm_t;

/* --- private routine section ------------------------------------------------------------------ */

static olsize_t _getJfHsmTransitionTableSize(jf_hsm_transition_t * pTransition)
{
    olsize_t tsize = 0;
    u32 u32Count = 0;
    jf_hsm_transition_t * pStart = pTransition;

    while (pStart->jht_jhsiCurrentStateId != JF_HSM_LAST_STATE_ID)
    {
        u32Count ++;
        pStart ++;
    }

    tsize = (u32Count + 1) * sizeof(jf_hsm_transition_t);

    return tsize;
}

static u32 _destroyInternalHsmStateCallback(internal_hsm_state_callback_t ** ppCallback)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_jiukun_freeMemory((void **)ppCallback);

    return u32Ret;
}

static u32 _destroyInternalHsmStateTransitionTable(internal_hsm_state_transition_table_t ** ppState)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    jf_jiukun_freeMemory((void **)ppState);

    return u32Ret;
}

static u32 _createInternalHsmStateTransitionTable(
    internal_hsm_state_transition_table_t ** ppState, jf_hsm_state_id_t stateId,
    jf_hsm_transition_t * pTransition, jf_hsm_event_id_t initialStateId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hsm_state_transition_table_t * pihstt = NULL;
    olsize_t tsize = 0;

    tsize = _getJfHsmTransitionTableSize(pTransition);

    u32Ret = jf_jiukun_allocMemory((void **)&pihstt, sizeof(*pihstt) + tsize);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pihstt->ihstt_jhsiStateId = stateId;
        pihstt->ihstt_jhsiInitialStateId = initialStateId;
        pihstt->ihstt_jhsiCurrentStateId = initialStateId;
        ol_memcpy(pihstt->ihstt_jhtTransition, pTransition, tsize);
        jf_listhead_init(&pihstt->ihstt_jlList);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppState = pihstt;
    else if (pihstt != NULL)
        _destroyInternalHsmStateTransitionTable(&pihstt);
    
    return u32Ret;
}

static u32 _getInternalHsmStateTransitionTable(
    internal_hsm_t * pih, jf_hsm_state_id_t stateId, internal_hsm_state_transition_table_t ** ppState)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos = NULL;
    internal_hsm_state_transition_table_t * pihstt = NULL;

    *ppState = NULL;

    if (stateId == JF_HSM_LAST_STATE_ID)
        u32Ret = JF_ERR_INVALID_PARAM;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = JF_ERR_HSM_STATE_NOT_FOUND;

        jf_listhead_forEach(&pih->ih_jlStateTransitionTable, pos)
        {
            pihstt = jf_listhead_getEntry(pos, internal_hsm_state_transition_table_t, ihstt_jlList);

            if (pihstt->ihstt_jhsiStateId == stateId)
            {
                u32Ret = JF_ERR_NO_ERROR;
                *ppState = pihstt;
                break;
            }
        }
    }

    return u32Ret;
}

static u32 _createInternalHsmStateCallback(
    internal_hsm_state_callback_t ** ppCallback, jf_hsm_state_id_t stateId,
    jf_hsm_fnOnEntry fnOnEntry, jf_hsm_fnOnExit fnOnExit)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hsm_state_callback_t * pihsc = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pihsc, sizeof(*pihsc));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pihsc->ihsc_jhsiStateId = stateId;
        pihsc->ihsc_fnOnEntry = fnOnEntry;
        pihsc->ihsc_fnOnExit = fnOnExit;

        jf_listhead_init(&pihsc->ihsc_jlList);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppCallback = pihsc;
    else if (pihsc != NULL)
        _destroyInternalHsmStateCallback(&pihsc);
    
    return u32Ret;
}

static u32 _getInternalHsmStateCallback(
    internal_hsm_t * pih, jf_hsm_state_id_t stateId, internal_hsm_state_callback_t ** ppCallback)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_listhead_t * pos = NULL;
    internal_hsm_state_callback_t * pihsc = NULL;

    *ppCallback = NULL;

    if (stateId == JF_HSM_LAST_STATE_ID)
        u32Ret = JF_ERR_INVALID_PARAM;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = JF_ERR_HSM_STATE_NOT_FOUND;
        jf_listhead_forEach(&pih->ih_jlStateCallback, pos)
        {
            pihsc = jf_listhead_getEntry(pos, internal_hsm_state_callback_t, ihsc_jlList);

            if (pihsc->ihsc_jhsiStateId == stateId)
            {
                u32Ret = JF_ERR_NO_ERROR;
                *ppCallback = pihsc;
                break;
            }
        }
    }

    return u32Ret;
}

static u32 _processHsmEvent(
    internal_hsm_t * pih, internal_hsm_state_transition_table_t * pihstt,
    jf_hsm_event_t * pEvent, boolean_t * pbHit)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hsm_transition_t * pjht = NULL;
    internal_hsm_state_callback_t * pihsc = NULL;
    internal_hsm_state_transition_table_t * pihsttNew = NULL;

    pjht = pihstt->ihstt_jhtTransition;
    while (pjht->jht_jheiEventId != JF_HSM_LAST_EVENT_ID)
    {
        if ((pjht->jht_jhsiCurrentStateId == pihstt->ihstt_jhsiCurrentStateId) &&
            (pjht->jht_jheiEventId == pEvent->jhe_jheiEventId))
        {
            *pbHit = TRUE;

            if ((pjht->jht_fnGuard == NULL) || pjht->jht_fnGuard(pEvent))
            {
                /*execute the callback action function*/
                u32Ret = pjht->jht_fnAction(pEvent);

                /*execute the callback function for exiting the old state*/
                _getInternalHsmStateCallback(pih, pihstt->ihstt_jhsiCurrentStateId, &pihsc);
                if (pihsc != NULL)
                    pihsc->ihsc_fnOnExit(pihsc->ihsc_jhsiStateId);

                /*transit to next state*/
                pihstt->ihstt_jhsiCurrentStateId = pjht->jht_jhsiNextStateId;

                /*execute the callback function for entering the new state*/
                _getInternalHsmStateCallback(pih, pihstt->ihstt_jhsiCurrentStateId, &pihsc);
                if (pihsc != NULL)
                    pihsc->ihsc_fnOnEntry(pihsc->ihsc_jhsiStateId);
                /*set current state to initial state if the new state has lower level state
                  transition table*/
                _getInternalHsmStateTransitionTable(pih, pihstt->ihstt_jhsiCurrentStateId, &pihsttNew);
                if (pihsttNew != NULL)
                    pihsttNew->ihstt_jhsiCurrentStateId = pihsttNew->ihstt_jhsiInitialStateId;

                break;
            }
        }

        pjht ++;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_hsm_destroy(jf_hsm_t ** ppHsm)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hsm_t * pih = (internal_hsm_t *)*ppHsm;
    jf_listhead_t * pos = NULL, * next = NULL;
    internal_hsm_state_transition_table_t * pihstt = NULL;
    internal_hsm_state_callback_t * pihsc = NULL;

    jf_listhead_forEachSafe(&pih->ih_jlStateTransitionTable, pos, next)
    {
        pihstt = jf_listhead_getEntry(pos, internal_hsm_state_transition_table_t, ihstt_jlList);

        _destroyInternalHsmStateTransitionTable(&pihstt);
    }

    jf_listhead_forEachSafe(&pih->ih_jlStateCallback, pos, next)
    {
        pihsc = jf_listhead_getEntry(pos, internal_hsm_state_callback_t, ihsc_jlList);

        _destroyInternalHsmStateCallback(&pihsc);
    }

    jf_jiukun_freeMemory(ppHsm);

    return u32Ret;
}

u32 jf_hsm_create(
    jf_hsm_t ** ppHsm, jf_hsm_transition_t * pTransition, jf_hsm_state_id_t initialStateId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hsm_t * pih = NULL;
    internal_hsm_state_transition_table_t * pihstt = NULL;

    u32Ret = jf_jiukun_allocMemory((void **)&pih, sizeof(*pih));
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_listhead_init(&pih->ih_jlStateTransitionTable);
        jf_listhead_init(&pih->ih_jlStateCallback);

        u32Ret = _createInternalHsmStateTransitionTable(
            &pihstt, JF_HSM_LAST_STATE_ID, pTransition, initialStateId);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_listhead_add(&pih->ih_jlStateTransitionTable, &pihstt->ihstt_jlList);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppHsm = pih;
    else if (pih != NULL)
        jf_hsm_destroy((jf_hsm_t **)&pih);

    return u32Ret;
}

jf_hsm_state_id_t jf_hsm_getCurrentStateId(jf_hsm_t * pjh)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hsm_t * pih = (internal_hsm_t *)pjh;
    internal_hsm_state_transition_table_t * pihstt = NULL;
    jf_hsm_state_id_t stateId = JF_HSM_LAST_STATE_ID;

    assert(! jf_listhead_isEmpty(&pih->ih_jlStateTransitionTable));

    /*get current state in top level state transition table */
    pihstt = jf_listhead_getEntry(
        pih->ih_jlStateTransitionTable.jl_pjlNext, internal_hsm_state_transition_table_t, ihstt_jlList);
    stateId = pihstt->ihstt_jhsiCurrentStateId;

    /*if the state has transition table, return the current state in lower level transiton table */
    u32Ret = _getInternalHsmStateTransitionTable(pih, stateId, &pihstt);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        stateId = pihstt->ihstt_jhsiCurrentStateId;
    }

    return stateId;
}

u32 jf_hsm_processEvent(jf_hsm_t * pjh, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hsm_t * pih = (internal_hsm_t *)pjh;
    boolean_t bHit = FALSE;
    jf_listhead_t * pos = NULL;
    internal_hsm_state_transition_table_t * pihstt = NULL;
    jf_hsm_state_id_t stateId = JF_HSM_LAST_STATE_ID;

    jf_listhead_forEach(&pih->ih_jlStateTransitionTable, pos)
    {
        pihstt = jf_listhead_getEntry(pos, internal_hsm_state_transition_table_t, ihstt_jlList);

        if (pihstt->ihstt_jhsiStateId == JF_HSM_LAST_STATE_ID)
        {
            /*top level state transition table*/
            stateId = pihstt->ihstt_jhsiCurrentStateId;
            _processHsmEvent(pih, pihstt, pEvent, &bHit);
            /*hit, we can quit*/
            if (bHit)
                break;
        }
        else
        {
            /*lower level state transition table*/
            if (pihstt->ihstt_jhsiStateId == stateId)
            {
                _processHsmEvent(pih, pihstt, pEvent, &bHit);
                /*hit, we can quit*/
                if (bHit)
                    break;
            }
        }
    }

    return u32Ret;
}

u32 jf_hsm_addStateTransition(
    jf_hsm_t * pHsm, jf_hsm_state_id_t stateId, jf_hsm_transition_t * pTransition,
    jf_hsm_event_id_t initialStateId)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hsm_t * pih = (internal_hsm_t *)pHsm;
    internal_hsm_state_transition_table_t * pihstt = NULL;

    assert(stateId != JF_HSM_LAST_STATE_ID);
    
    u32Ret = _getInternalHsmStateTransitionTable(pih, stateId, &pihstt);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_listhead_del(&pihstt->ihstt_jlList);
        _destroyInternalHsmStateTransitionTable(&pihstt);
    }

    if ((u32Ret == JF_ERR_NO_ERROR) || (u32Ret == JF_ERR_HSM_STATE_NOT_FOUND))
    {
        u32Ret = _createInternalHsmStateTransitionTable(
            &pihstt, stateId, pTransition, initialStateId);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_listhead_addTail(&pih->ih_jlStateTransitionTable, &pihstt->ihstt_jlList);
        }
    }

    return u32Ret;
}

u32 jf_hsm_addStateCallback(
    jf_hsm_t * pHsm, jf_hsm_state_id_t stateId, jf_hsm_fnOnEntry fnOnEntry,
    jf_hsm_fnOnExit fnOnExit)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_hsm_t * pih = (internal_hsm_t *)pHsm;
    internal_hsm_state_callback_t * pihsc = NULL;

    u32Ret = _getInternalHsmStateCallback(pih, stateId, &pihsc);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        pihsc->ihsc_fnOnEntry = fnOnEntry;
        pihsc->ihsc_fnOnExit = fnOnExit;
    }
    else if (u32Ret == JF_ERR_HSM_STATE_NOT_FOUND)
    {
        u32Ret = _createInternalHsmStateCallback(&pihsc, stateId, fnOnEntry, fnOnExit);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_listhead_addTail(&pih->ih_jlStateCallback, &pihsc->ihsc_jlList);
        }
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

