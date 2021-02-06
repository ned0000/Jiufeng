/**
 *  @file socketpair.c
 *
 *  @brief Implementation file of socket pair.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#if defined(LINUX)

#elif defined(WINDOWS)
    #include <winsock2.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_network.h"

#include "internalsocket.h"

/* --- private data/data structure section ------------------------------------------------------ */


/* --- private routine section ------------------------------------------------------------------ */

#if defined(LINUX)
static u32 _createUnixSocketPair(olint_t type, jf_network_socket_t * psPair[2])
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet = 0;
    isocket_t isPair[2] = {INVALID_ISOCKET, INVALID_ISOCKET};

    /*create a pair of connected sockets.*/
    nRet = socketpair(AF_UNIX, type, 0, isPair);
    if (nRet != 0)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET_PAIR;

    /*Create the first network socket.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        JF_LOGGER_DEBUG("unix sock pair: %d, %d", isPair[0], isPair[1]);

        u32Ret = newIsocketWithSocket((internal_socket_t **)&(psPair[0]), isPair[0]);
    }

    /*Create the second network socket.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = newIsocketWithSocket((internal_socket_t **)&(psPair[1]), isPair[1]);
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

    /*Use loopback as server address.*/
    jf_ipaddr_setIpV4Addr(&ipaddr, htonl(INADDR_LOOPBACK));

    /*Create listen stream socket.*/
    u32Ret = jf_network_createStreamSocket(&ipaddr, &u16Port, &pListener);

    /*Start listening.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_listen(pListener, 5);
    }

    /*Create the second socket in socket pair.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_createSocket(domain, type, 0, &(psPair[1]));
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Set the second socket in non-block mode, it's necessary.*/
        jf_network_setSocketNonblock(psPair[1]);

        /*Use the second socket to connect to server.*/
        u32Ret = jf_network_connect(psPair[1], &ipaddr, u16Port);
    }

    /*Accept the connecting request with the first socket in socket pair.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_network_accept(pListener, &clientaddr, &u16ClientPort, &(psPair[0]));
    }

    /*Restore the second socket to block mode.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        jf_network_setSocketBlock(psPair[1]);
    }

    /*Destroy the listen socket.*/
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

    /*Create socket pair based on the domain.*/
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

    /*Create socket pair based on the domain.*/
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

    /*Destroy the first socket.*/
    if (psPair[0] != NULL)
        jf_network_destroySocket(&(psPair[0]));

    /*Destroy the second socket.*/
    if (psPair[1] != NULL)
        jf_network_destroySocket(&(psPair[1]));

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
