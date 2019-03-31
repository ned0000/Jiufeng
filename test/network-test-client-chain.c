/**
 *  @file network-test-client-chain.c
 *
 *  @brief Test file for network library. Network client with chain mode.
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
#include "errcode.h"
#include "network.h"
#include "stringparse.h"
#include "process.h"
#include "comminit.h"

/* --- private data/data structure section --------------------------------- */
static jf_network_chain_t * ls_pjncChain;

/* --- private routine section---------------------------------------------- */


static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    if (ls_pjncChain != NULL)
        jf_network_stopChain(ls_pjncChain);
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_network_asocket_t * pAsocket = NULL;
    jf_network_asocket_create_param_t jnacp;
    logger_param_t lp;

    memset(&lp, 0, sizeof(logger_param_t));
    lp.lp_bLogToStdout = TRUE;
    lp.lp_u8TraceLevel = 2;

    initLogger(&lp);

    u32Ret = jf_network_initLib();
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = registerSignalHandlers(_terminate);
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = jf_network_createChain(&ls_pjncChain);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = jf_network_createAsocket(ls_pjncChain, &pAsocket, &jnacp);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {

        }

        jf_network_finiLib();
    }

    finiLogger();

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

