/**
 *  @file hsm-test.c
 *
 *  @brief Test file for hierarchical state machine defined in jf_hsm common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_logger.h"
#include "jf_err.h"
#include "jf_hsm.h"
#include "jf_jiukun.h"

/* --- private data/data structure section ------------------------------------------------------ */

enum rice_cooker_state_id
{
    RCS_START = 0,

    RCS_POWER_OFF = RCS_START,
    RCS_KEEP_WARM,
    RCS_COOK,

    RCS_FIREPOWER_LOW,  /**<Sub state for cook state.*/
    RCS_FIREPOWER_MID,  /**<Sub state for cook state.*/
    RCS_FIREPOWER_HIGH, /**<Sub state for cook state.*/

    RCS_END,
};

enum rice_cooker_event_id
{
    RCE_PLUG_IN = 0,
    RCE_PRESS_COOK_BUTTON, /**Cook <--> Keep warm.*/
    RCE_PLUG_OUT,
    RCE_PRESS_FIREPOWER_BUTTON, /**<Firepower low, mid, high.*/
};

/* --- private routine section ------------------------------------------------------------------ */

static boolean_t _guardPowerOffForPlugIn(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RCE_PLUG_IN)
        bRet = TRUE;

    return bRet;
}

static boolean_t _guardFirepowerLowForFirepower(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RCE_PRESS_FIREPOWER_BUTTON)
        bRet = TRUE;

    return bRet;
}

static u32 _actionPowerOn(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("Action for Power-On\n");

    return u32Ret;
}

static u32 _actionFirepower(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("Action for Firepower\n");

    return u32Ret;
}

static boolean_t _guardKeepWarmForPressCookButton(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RCE_PRESS_COOK_BUTTON)
        bRet = TRUE;

    return bRet;
}

static boolean_t _guardKeepWarmForPlugOut(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RCE_PLUG_OUT)
        bRet = TRUE;

    return bRet;
}

static u32 _actionCook(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("Action for Cook\n");

    return u32Ret;
}

static u32 _actionPlugOut(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("Action for Plug-Out\n");

    return u32Ret;
}

static boolean_t _guardCookForPressCookButton(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RCE_PRESS_COOK_BUTTON)
        bRet = TRUE;

    return bRet;
}

static boolean_t _guardCookForPlugOut(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RCE_PLUG_OUT)
        bRet = TRUE;

    return bRet;
}

static u32 _actionKeepWarm(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_printf("Action for Keep-Warm\n");

    return u32Ret;
}

static void _getCurrentRiceCookerStateString(jf_hsm_state_id_t current, olchar_t * pstr)
{
    switch (current)
    {
    case RCS_POWER_OFF:
        ol_strcpy(pstr, "Power-Off");
        break;
    case RCS_KEEP_WARM:
        ol_strcpy(pstr, "Keep-Warm");
        break;
    case RCS_COOK:
        ol_strcpy(pstr, "Cook");
        break;
    case RCS_FIREPOWER_LOW:
        ol_strcpy(pstr, "Cook : Low-Firepower");
        break;
    case RCS_FIREPOWER_MID:
        ol_strcpy(pstr, "Cook : Mid-Firepower");
        break;
    case RCS_FIREPOWER_HIGH:
        ol_strcpy(pstr, "Cook : High-Firepower");
        break;
    default:            
        ol_strcpy(pstr, "Unkwnown");
        break;
    }
}

static void _printCurrentRiceCookerState(jf_hsm_t * pjh)
{
    jf_hsm_state_id_t current;
    olchar_t buffer[64];

    current = jf_hsm_getCurrentStateId(pjh);
    _getCurrentRiceCookerStateString(current, buffer);

    ol_printf("Current state: %s\n\n", buffer);
}

static void _printEvent(jf_hsm_event_t * pEvent)
{
    switch (pEvent->jhe_jheiEventId)
    {
    case RCE_PLUG_IN:
        ol_printf("Event: Plug_In\n");
        break;
    case RCE_PRESS_COOK_BUTTON:
        ol_printf("Event: Press_Cook_Button\n");
        break;
    case RCE_PLUG_OUT:
        ol_printf("Event: Plug_Out\n");
        break;
    case RCE_PRESS_FIREPOWER_BUTTON:
        ol_printf("Event: Press_Firepower_Button\n");
        break;
    default:
        ol_printf("Event: Unknown\n");
        break;
    }
}

static u32 _hsmTestOnEntry(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t buffer[64];

    _getCurrentRiceCookerStateString(stateId, buffer);

    ol_printf("OnEntry, %s\n", buffer);

    return u32Ret;
}

static u32 _hsmTestOnExit(jf_hsm_state_id_t stateId, jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t buffer[64];

    _getCurrentRiceCookerStateString(stateId, buffer);

    ol_printf("OnExit, %s\n", buffer);

    return u32Ret;
}

static u32 _testHsm(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_hsm_t * pjh = NULL;
    jf_hsm_event_t eventPlugIn = {RCE_PLUG_IN, NULL};
    jf_hsm_event_t eventPressCookButton = {RCE_PRESS_COOK_BUTTON, NULL};
    jf_hsm_event_t eventPlugOut = {RCE_PLUG_OUT, NULL};
    jf_hsm_event_t eventPressFirepowerButtion = {RCE_PRESS_FIREPOWER_BUTTON, NULL};
    jf_hsm_event_t * pEvent;
    jf_hsm_transition_t riceCookerTransitionTable[] = {
        {RCS_POWER_OFF, RCE_PLUG_IN, _guardPowerOffForPlugIn, _actionPowerOn, RCS_KEEP_WARM},
        {RCS_KEEP_WARM, RCE_PRESS_COOK_BUTTON, _guardKeepWarmForPressCookButton, _actionCook, RCS_COOK},
        {RCS_KEEP_WARM, RCE_PLUG_OUT, _guardKeepWarmForPlugOut, _actionPlugOut, RCS_POWER_OFF},
        {RCS_COOK, RCE_PRESS_COOK_BUTTON, _guardCookForPressCookButton, _actionKeepWarm, RCS_KEEP_WARM},
        {RCS_COOK, RCE_PLUG_OUT, _guardCookForPlugOut, _actionPlugOut, RCS_POWER_OFF},
        {JF_HSM_LAST_STATE_ID, JF_HSM_LAST_EVENT_ID, NULL, NULL, JF_HSM_LAST_STATE_ID},            
    };
    jf_hsm_transition_t cookTransitionTable[] = {
        {RCS_FIREPOWER_LOW, RCE_PRESS_FIREPOWER_BUTTON, _guardFirepowerLowForFirepower, _actionFirepower, RCS_FIREPOWER_MID},
        {RCS_FIREPOWER_MID, RCE_PRESS_FIREPOWER_BUTTON, NULL, _actionFirepower, RCS_FIREPOWER_HIGH},
        {RCS_FIREPOWER_HIGH, RCE_PRESS_FIREPOWER_BUTTON, NULL, _actionFirepower, RCS_FIREPOWER_LOW},
        {JF_HSM_LAST_STATE_ID, JF_HSM_LAST_EVENT_ID, NULL, NULL, JF_HSM_LAST_STATE_ID},            
    };
    olint_t index = 0;
    jf_hsm_event_t * pEventArray[] = {
        &eventPlugOut,
        &eventPlugIn,
        &eventPressFirepowerButtion,
        &eventPressCookButton,
        &eventPressFirepowerButtion,
        &eventPressCookButton,
        &eventPressCookButton,
        &eventPressFirepowerButtion,
        &eventPressFirepowerButtion,
        &eventPressFirepowerButtion,
        &eventPlugOut,
    };

    u32Ret = jf_hsm_create(&pjh, riceCookerTransitionTable, RCS_POWER_OFF);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_hsm_addStateTransition(
            pjh, RCS_COOK, cookTransitionTable, RCS_FIREPOWER_LOW);
    }
        
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (index = RCS_START;
             (index < RCS_END) && (u32Ret == JF_ERR_NO_ERROR); index ++)
        {
            u32Ret = jf_hsm_addStateCallback(
                pjh, (jf_hsm_state_id_t)index, _hsmTestOnEntry, _hsmTestOnExit);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        _printCurrentRiceCookerState(pjh);

        for (index = 0; index < ARRAY_SIZE(pEventArray); ++ index)
        {
            pEvent = pEventArray[index];
            _printEvent(pEvent);
            jf_hsm_processEvent(pjh, pEvent);
            _printCurrentRiceCookerState(pjh);
        }
    }

    if (pjh != NULL)
        jf_hsm_destroy(&pjh);

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;
    jf_jiukun_init_param_t jjip;

    ol_bzero(&jlipParam, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "HSM-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_LEVEL_DATA;

    ol_bzero(&jjip, sizeof(jjip));
    jjip.jjip_sPool = JF_JIUKUN_MAX_POOL_SIZE;

    jf_logger_init(&jlipParam);

    u32Ret = jf_jiukun_init(&jjip);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = _testHsm();

        jf_jiukun_fini();
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    jf_logger_fini();

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
