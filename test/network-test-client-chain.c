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

/* --- private data/data structure section --------------------------------- */
static basic_chain_t * ls_pbcChain;

/* --- private routine section---------------------------------------------- */


static void _terminate(olint_t signal)
{
    ol_printf("get signal\n");

    if (ls_pbcChain != NULL)
        stopBasicChain(ls_pbcChain);
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    asocket_t * pAsocket = NULL;
    asocket_param_t ap;
    logger_param_t lp;

    memset(&lp, 0, sizeof(logger_param_t));
    lp.lp_bLogToStdout = TRUE;
    lp.lp_u8TraceLevel = 2;

    initLogger(&lp);

    u32Ret = initNetworkLib();
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = registerSignalHandlers(_terminate);
        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = createBasicChain(&ls_pbcChain);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {
            u32Ret = createAsocket(ls_pbcChain, &pAsocket, &ap);
        }

        if (u32Ret == OLERR_NO_ERROR)
        {

        }

        finiNetworkLib();
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

