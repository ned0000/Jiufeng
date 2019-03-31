/**
 *  @file comminit.h
 *
 *  @brief Including common initialization and finalization. Initialize and
 *   finalize net-related library, including ifmgmt, network library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUTAI_COMMINIT_H
#define JIUTAI_COMMINIT_H

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if defined(LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
#elif defined(WINDOWS)
    #include <Winsock2.h>
    #include <Iphlpapi.h>
#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"


/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */


/* --- functional routines ------------------------------------------------- */


/*init net-related library*/
static inline u32 jf_network_initLib(void)
{
    u32 u32Ret = OLERR_NO_ERROR;

#if defined(LINUX)

#elif defined(WINDOWS)
    WORD wVersionRequested;
    WSADATA wsaData;
    olint_t err;

    srand(time(NULL));

    wVersionRequested = MAKEWORD(2, 0);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        u32Ret = OLERR_FAIL_INIT_NET_LIB;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        /* Confirm that the WinSock DLL supports 2.0.*/
        /* Note that if the DLL supports versions greater    */
        /* than 2.0 in addition to 2.0, it will still return */
        /* 2.0 in wVersion since that is the version we      */
        /* requested.                                        */
        if (LOBYTE(wsaData.wVersion ) != 2 ||
            HIBYTE(wsaData.wVersion ) != 0)
        {
            /* Tell the user that we could not find a usable WinSock DLL.*/
            WSACleanup();
            u32Ret = OLERR_FAIL_INIT_NET_LIB;
        }
    }

#endif

    return u32Ret;
}

static inline u32 jf_network_finiLib(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
#if defined(LINUX)

#elif defined(WINDOWS)
    WSACleanup();
#endif

    return u32Ret;
}

#endif /*JIUTAI_COMMINIT_H*/

/*---------------------------------------------------------------------------*/


