/**
 *  @file hostinfo-test.c
 *
 *  @brief test file for hostinfo common object file
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
#include "hostinfo.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    host_info_t hi;
    olchar_t strErrMsg[300];
    u16 u16Index;

    u32Ret = getHostInfo(&hi);
    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("host name: %s\n", hi.hi_strHostName);
        ol_printf("OS name: %s\n", hi.hi_strOSName);
        ol_printf("%u network interface found\n", hi.hi_u16NetCount);

        for (u16Index = 0; u16Index < hi.hi_u16NetCount; u16Index ++)
        {
            ol_printf("    %s, %s\n", hi.hi_niNet[u16Index].ni_strName,
                  hi.hi_niNet[u16Index].ni_strIpAddr);
        }
    }

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, 300);
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*--------------------------------------------------------------------------*/

