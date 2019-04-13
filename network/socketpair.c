/**
 *  @file socketpair.c
 *
 *  @brief The implementation of socketpair
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdio.h>
#include <string.h>

#if defined(LINUX)

#elif defined(WINDOWS)
    #include <winsock2.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_network.h"
#include "internalsocket.h"

/* --- private data/data structure section ------------------------------------------------------ */

/* --- private routine section ------------------------------------------------------------------ */
#if defined(LINUX)
static u32 _createUnixSocketPair(olint_t type, jf_network_socket_t * psPair[2])
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet;
    isocket_t isPair[2];

    nRet = socketpair(AF_UNIX, type, 0, isPair);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET_PAIR;

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = newIsocketWithSocket(
            (internal_socket_t **)&(psPair[0]), isPair[0]);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = newIsocketWithSocket(
            (internal_socket_t **)&(psPair[1]), isPair[1]);
    }

    return u32Ret;
}
#endif
static u32 _createInetSocketPair(olint_t domain, olint_t type, jf_network_socket_t * psPair[2])
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    jf_ipaddr_t ipaddr, clientaddr;
    u16 u16Port = 0, u16ClientPort = 0;
    jf_network_socket_t * pListener = NULL;

    jf_ipaddr_setIpV4Addr(&ipaddr, htonl(INADDR_LOOPBACK));

    u32Ret = jf_network_createStreamSocket(&ipaddr, &u16Port, &pListener);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_listen(pListener, 5);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_createSocket(domain, type, 0, &(psPair[1]));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_setSocketNonblock(psPair[1]);

        u32Ret = jf_network_connect(psPair[1], &ipaddr, u16Port);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_accept(pListener, &clientaddr,
            &u16ClientPort, &(psPair[0]));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_setSocketBlock(psPair[1]);
    }

    if (pListener != NULL)
        jf_network_destroySocket(&pListener);

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_network_destroySocketPair(psPair);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 jf_network_createSocketPair(
    olint_t domain, olint_t type, jf_network_socket_t * psPair[2])
{
    u32 u32Ret = JF_ERR_NO_ERROR;

#if defined(LINUX)
    assert((domain == AF_UNIX) ||
           (domain == AF_INET && type == SOCK_STREAM) ||
           (domain == AF_INET6 && type == SOCK_STREAM));

    psPair[0] = NULL;
    psPair[1] = NULL;

    if (domain == AF_UNIX)
        u32Ret = _createUnixSocketPair(type, psPair);
    else if (domain == AF_INET)
        u32Ret = _createInetSocketPair(domain, type, psPair);
    else
        u32Ret = JF_ERR_NOT_IMPLEMENTED;

#elif defined(WINDOWS)

    assert((domain == AF_INET && type == SOCK_STREAM) ||
           (domain == AF_INET6 && type == SOCK_STREAM));

    psPair[0] = NULL;
    psPair[1] = NULL;

    if (domain == AF_INET)
        u32Ret = _createInetSocketPair(domain, type, psPair);
    else
        u32Ret = JF_ERR_NOT_IMPLEMENTED;
#endif

    return u32Ret;
}

u32 jf_network_destroySocketPair(jf_network_socket_t * psPair[2])
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (psPair[0] != NULL)
        jf_network_destroySocket(&(psPair[0]));

    if (psPair[1] != NULL)
        jf_network_destroySocket(&(psPair[1]));

    return u32Ret;
}


/*------------------------------------------------------------------------------------------------*/


