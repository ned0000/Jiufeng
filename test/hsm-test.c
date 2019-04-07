/**
 *  @file hsm-test.c
 *
 *  @brief test file for testing hsm common object
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "logger.h"
#include "errcode.h"
#include "hsm.h"

/* --- private data/data structure section --------------------------------- */
enum rice_cooker_state_id
{
    RICE_COOKER_STATE_POWER_OFF = 0,
    RICE_COOKER_STATE_KEEP_WARM,
    RICE_COOKER_STATE_COOK,
};

enum rice_cooker_event_id
{
    RICE_COOKER_EVENT_PLUG_IN = 0,
    RICE_COOKER_EVENT_PRESS_BUTTON,
    RICE_COOKER_EVENT_PLUG_OUT,
};

static boolean_t _guardPowerOffForPlugIn(jf_hsm_event_t * pEvent);
static u32 _actionPowerOn(jf_hsm_event_t * pEvent);

static boolean_t _guardKeepWarmForPressButton(jf_hsm_event_t * pEvent);
static boolean_t _guardKeepWarmForPlugOut(jf_hsm_event_t * pEvent);
static u32 _actionCook(jf_hsm_event_t * pEvent);
static u32 _actionPlugOut(jf_hsm_event_t * pEvent);

static boolean_t _guardCookForPressButton(jf_hsm_event_t * pEvent);
static boolean_t _guardCookForPlugOut(jf_hsm_event_t * pEvent);
static u32 _actionKeepWarm(jf_hsm_event_t * pEvent);



static jf_hsm_state_t ls_hsRiceCookerStatePowerOff;
static jf_hsm_state_t ls_hsRiceCookerStateKeepWarm;
static jf_hsm_state_t ls_hsRiceCookerStateCook;

static jf_hsm_state_t ls_hsRiceCookerStatePowerOff =
{
    RICE_COOKER_STATE_POWER_OFF,
    "power off",
    {
        {RICE_COOKER_EVENT_PLUG_IN, _guardPowerOffForPlugIn, _actionPowerOn, &ls_hsRiceCookerStateKeepWarm},
        {HSM_LAST_EVENT_ID, NULL, NULL, NULL},
    },
};

static jf_hsm_state_t ls_hsRiceCookerStateKeepWarm =
{
    RICE_COOKER_STATE_KEEP_WARM,
    "keep warm",
    {
        {RICE_COOKER_EVENT_PRESS_BUTTON, _guardKeepWarmForPressButton, _actionCook,    &ls_hsRiceCookerStateCook},
        {RICE_COOKER_EVENT_PLUG_OUT,     _guardKeepWarmForPlugOut,     _actionPlugOut, &ls_hsRiceCookerStatePowerOff},
        {HSM_LAST_EVENT_ID, NULL, NULL, NULL},
    },
};

static jf_hsm_state_t ls_hsRiceCookerStateCook =
{
    RICE_COOKER_STATE_COOK,
    "cook",
    {
        {RICE_COOKER_EVENT_PRESS_BUTTON, _guardCookForPressButton, _actionKeepWarm, &ls_hsRiceCookerStateKeepWarm},
        {RICE_COOKER_EVENT_PLUG_OUT,     _guardCookForPlugOut,     _actionPlugOut,  &ls_hsRiceCookerStatePowerOff},
        {HSM_LAST_EVENT_ID, NULL, NULL, NULL},
    },
};

static jf_hsm_t ls_hsRiceCookerStateMachine;

/* --- private routine section---------------------------------------------- */

static boolean_t _guardPowerOffForPlugIn(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RICE_COOKER_EVENT_PLUG_IN)
        bRet = TRUE;

    return bRet;
}

static u32 _actionPowerOn(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

static boolean_t _guardKeepWarmForPressButton(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RICE_COOKER_EVENT_PRESS_BUTTON)
        bRet = TRUE;

    return bRet;
}

static boolean_t _guardKeepWarmForPlugOut(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RICE_COOKER_EVENT_PLUG_OUT)
        bRet = TRUE;

    return bRet;
}

static u32 _actionCook(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

static u32 _actionPlugOut(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

static boolean_t _guardCookForPressButton(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RICE_COOKER_EVENT_PRESS_BUTTON)
        bRet = TRUE;

    return bRet;
}

static boolean_t _guardCookForPlugOut(jf_hsm_event_t * pEvent)
{
    boolean_t bRet = FALSE;

    if (pEvent->jhe_jheiEventId == RICE_COOKER_EVENT_PLUG_OUT)
        bRet = TRUE;

    return bRet;
}

static u32 _actionKeepWarm(jf_hsm_event_t * pEvent)
{
    u32 u32Ret = JF_ERR_NO_ERROR;


    return u32Ret;
}

static void _printCurrentState(jf_hsm_t * pjh)
{
    jf_hsm_state_t * current;

    current = jf_hsm_getCurrentState(pjh);
    ol_printf("Current state: %s\n\n", current->jhs_strName);
}

static void _printEvent(jf_hsm_event_t * pEvent)
{
    switch (pEvent->jhe_jheiEventId)
    {
    case RICE_COOKER_EVENT_PLUG_IN:
        ol_printf("Event: Plug_In\n");
        break;
    case RICE_COOKER_EVENT_PRESS_BUTTON:
        ol_printf("Event: Press_Button\n");
        break;
    case RICE_COOKER_EVENT_PLUG_OUT:
        ol_printf("Event: Plug_Out\n");
        break;
    default:
        ol_printf("Event: Unknown\n");
        break;
    }
}

static void _testHsm(void)
{
    jf_hsm_t * phs = &ls_hsRiceCookerStateMachine;
    jf_hsm_event_t eventPlugIn = {RICE_COOKER_EVENT_PLUG_IN, NULL};
    jf_hsm_event_t eventPressButton = {RICE_COOKER_EVENT_PRESS_BUTTON, NULL};
    jf_hsm_event_t eventPlugOut = {RICE_COOKER_EVENT_PLUG_OUT, NULL};
    jf_hsm_event_t * pEvent;

    jf_hsm_init(phs, &ls_hsRiceCookerStatePowerOff);
    _printCurrentState(phs);

    pEvent = &eventPlugOut;
    _printEvent(pEvent);
    jf_hsm_handleEvent(phs, pEvent);
    _printCurrentState(phs);

    pEvent = &eventPlugIn;
    _printEvent(pEvent);
    jf_hsm_handleEvent(phs, pEvent);
    _printCurrentState(phs);

    pEvent = &eventPressButton;
    _printEvent(pEvent);
    jf_hsm_handleEvent(phs, pEvent);
    _printCurrentState(phs);

    pEvent = &eventPressButton;
    _printEvent(pEvent);
    jf_hsm_handleEvent(phs, pEvent);
    _printCurrentState(phs);

    pEvent = &eventPlugOut;
    _printEvent(pEvent);
    jf_hsm_handleEvent(phs, pEvent);
    _printCurrentState(phs);

    jf_hsm_fini(phs);
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t jlipParam;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_pstrCallerName = "HSM-TEST";
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = JF_LOGGER_TRACE_DATA;

    jf_logger_init(&jlipParam);

    _testHsm();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    jf_logger_fini();

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

