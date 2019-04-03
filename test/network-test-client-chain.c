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
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_network_asocket_t * pAsocket = NULL;
    jf_network_asocket_create_param_t jnacp;
    jf_logger_init_param_t jlipParam;

    memset(&jlipParam, 0, sizeof(jf_logger_init_param_t));
    jlipParam.jlip_bLogToStdout = TRUE;
    jlipParam.jlip_u8TraceLevel = 2;

    jf_logger_init(&jlipParam);

    u32Ret = jf_network_initLib();
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = registerSignalHandlers(_terminate);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_network_createChain(&ls_pjncChain);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {
            u32Ret = jf_network_createAsocket(ls_pjncChain, &pAsocket, &jnacp);
        }

        if (u32Ret == JF_ERR_NO_ERROR)
        {

        }

        jf_network_finiLib();
    }

    jf_logger_fini();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

