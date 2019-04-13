/**
 *  @file host-test.c
 *
 *  @brief test file for host common object
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_host.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_host_info_t jhi;
    olchar_t strErrMsg[300];
    u16 u16Index;

    u32Ret = jf_host_getInfo(&jhi);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("host name: %s\n", jhi.jhi_strHostName);
        ol_printf("OS name: %s\n", jhi.jhi_strOSName);
        ol_printf("%u network interface found\n", jhi.jhi_u16NetCount);

        for (u16Index = 0; u16Index < jhi.jhi_u16NetCount; u16Index ++)
        {
            ol_printf(
                "    %s, %s\n", jhi.jhi_jhniNet[u16Index].jhni_strName,
                jhi.jhi_jhniNet[u16Index].jhni_strIpAddr);
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/

