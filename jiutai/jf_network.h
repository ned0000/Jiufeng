/**
 *  @file jf_network.h
 *
 *  @brief Header file of network library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_network library.
 *  -# Timeout is in second if not specified.
 *
 */

/*------------------------------------------------------------------------------------------------*/

#ifndef JIUFENG_NETWORK_H
#define JIUFENG_NETWORK_H

/* --- standard C lib header files -------------------------------------------------------------- */
#if defined(LINUX)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/ip.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <net/if.h>
    #include <sys/ioctl.h>
#elif defined(WINDOWS)
    #include <Iphlpapi.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_ifmgmt.h"
#include "jf_mutex.h"

#undef NETWORKAPI
#undef NETWORKCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_NETWORK_DLL)
        #define NETWORKAPI  __declspec(dllexport)
        #define NETWORKCALL
    #else
        #define NETWORKAPI
        #define NETWORKCALL __cdecl
    #endif
#else
    #define NETWORKAPI
    #define NETWORKCALL
#endif

/* --- constant definitions --------------------------------------------------------------------- */

/** Maximum name length for socket and utimer.
 */
#define JF_NETWORK_MAX_NAME_LEN     (32)

/* --- data structures -------------------------------------------------------------------------- */
#if defined(LINUX)

#elif defined(WINDOWS)

#endif

/** Define the network socket data type.
 */
typedef void  jf_network_socket_t;

/** Define the network async socket data type.
 */
typedef void  jf_network_asocket_t;

/** Define the network async server socket data type.
 */
typedef void  jf_network_assocket_t;

/** Define the network async client socket data type.
 */
typedef void  jf_network_acsocket_t;

/** Define the network async datagram socket data type.
 */
typedef void  jf_network_adgram_t;

/** Define the network chain data type.
 */
typedef void  jf_network_chain_t;

/** Define the network chain object data type.
 */
typedef void  jf_network_chain_object_t;

/** Callback function before select().
 *
 *  @param pObject [in] Chain object.
 *  @param readset [in/out] Read fd set.
 *  @param writeset [in/out] Write fd set.
 *  @param errorset [in/out] Error fd set.
 *  @param pu32BlockTime [in/out] Timeout in millisecond for select().
 *
 *  @return The error code.
 */
typedef u32 (* jf_network_fnPreSelectChainObject_t)(
    jf_network_chain_object_t * pObject, fd_set * readset, fd_set * writeset, fd_set * errorset,
    u32 * pu32BlockTime);

/** Callback function after select()
 *
 *  @param pObject [in] chain object.
 *  @param nReady [in] number of ready fd.
 *  @param readset [in] read fd set.
 *  @param writeset [in] write fd set.
 *  @param errorset [in] error fd set.
 *
 *  @return The error code.
 */
typedef u32 (* jf_network_fnPostSelectChainObject_t)(
    jf_network_chain_object_t * pObject, olint_t nReady, fd_set * readset, fd_set * writeset,
    fd_set * errorset);

/** Header of chain object, MUST be placed at the beginning of the object.
 */
typedef struct
{
    jf_network_fnPreSelectChainObject_t jncoh_fnPreSelect;
    jf_network_fnPostSelectChainObject_t jncoh_fnPostSelect;
} jf_network_chain_object_header_t;

/** Define the network utimer data type.
 */
typedef void  jf_network_utimer_t;

/** The callback function is called when the timer is triggerred.
 */
typedef u32 (* jf_network_fnCallbackOfUtimerItem_t)(void * pData);

/** The callback function is called when the timer item is destroyed.
 */
typedef u32 (* jf_network_fnDestroyUtimerItemData_t)(void ** ppData);

/*  Async server socket.
 */

/** The function is to notify upper layer there are incoming data.
 *
 *  @note
 *  -# The pUser is set by fnAssocketOnConnect_t when a new incoming connection is accepted.
 */
typedef u32 (* jf_network_fnAssocketOnData_t)(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser);

/** The function is to notify upper layer there are new connection.
 */
typedef u32 (* jf_network_fnAssocketOnConnect_t)(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser);

/** The function is to notify upper layer a connection is closed.
 *
 *  @note
 *  -# The access to the asocket being closed is not allowed.
 *  -# DO NOT use asDisconnect in this callback function as the connection is closed already.
 *
 *  @param u32Status [out] The reason why the connection is closed.
 */
typedef u32 (* jf_network_fnAssocketOnDisconnect_t)(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u32 u32Status, void * pUser);

/** The function is to notify upper layer the data is sent to peer successfully.
 */
typedef u32 (* jf_network_fnAssocketOnSendData_t)(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser);

/** Define parameter for creating async socket.
 */
typedef struct
{
    /**The initial size of the receive buffer.*/
    olsize_t jnacp_sInitialBuf;
    /**The max number of simultaneous connections that will be allowed.*/
    u32 jnacp_u32MaxConn;
    u32 jnacp_u32Reserved;
    jf_ipaddr_t jnacp_jiServer;
    /**The port number to bind to. 0 will select a random port.*/
    u16 jnacp_u16ServerPort;
    u16 jnacp_u16Reserved[3];
    /**Function that triggers when a connection is established.*/
    jf_network_fnAssocketOnConnect_t jnacp_fnOnConnect;
    /**Function that triggers when a connection is closed.*/
    jf_network_fnAssocketOnDisconnect_t jnacp_fnOnDisconnect;
    /**Function that triggers when data is coming.*/
    jf_network_fnAssocketOnData_t jnacp_fnOnData;
    /**Function that triggers when pending sends are complete.*/
    jf_network_fnAssocketOnSendData_t jnacp_fnOnSendData;
    olchar_t * jnacp_pstrName;
} jf_network_assocket_create_param_t;

/*  Async client socket.
 */

/** The function is to notify upper layer there are incoming data.
 */
typedef u32 (* jf_network_fnAcsocketOnData_t)(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser);

/** The function is to notify upper layer there are new connection.
 */
typedef u32 (* jf_network_fnAcsocketOnConnect_t)(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    void * pUser);

/** The function is to notify upper layer a connection is closed.
 *
 *  @note
 *  -# The access to the asocket being closed is not allowed.
 *  -# DO NOT use disconnectAsocket in this callback function as asocket can handle it by itself.
 *
 *  @param u32Status [in] The reason why the connection is closed.
 */
typedef u32 (* jf_network_fnAcsocketOnDisconnect_t)(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    void * pUser);

/** The function is to notify upper layer the data is sent to peer successfully.
 */
typedef u32 (* jf_network_fnAcsocketOnSendData_t)(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    u8 * pu8Buffer, olsize_t sBuf, void * pUser);

/** Define parameter for creating async client socket.
 */
typedef struct
{
    /**The initial size of the receive buffer.*/
    olsize_t jnacp_sInitialBuf;
    /**The max number of simultaneous connections that will be allowed.*/
    u32 jnacp_u32MaxConn;
    u8 jnacp_u8Reserved[8];
    /**Callback function that triggers when a connection is established.*/
    jf_network_fnAcsocketOnConnect_t jnacp_fnOnConnect;
    /**Callback function that triggers when a connection is closed.*/
    jf_network_fnAcsocketOnDisconnect_t jnacp_fnOnDisconnect;
    /**Callback function that triggers when data is received.*/
    jf_network_fnAcsocketOnData_t jnacp_fnOnData;
    /**Callback function that triggers when pending sends are complete.*/
    jf_network_fnAcsocketOnSendData_t jnacp_fnOnSendData;
    olchar_t * jnacp_pstrName;
} jf_network_acsocket_create_param_t;


/* --- functional routines ---------------------------------------------------------------------- */

/*  Network socket routine.
 */

NETWORKAPI u32 NETWORKCALL jf_network_createSocket(
    olint_t domain, olint_t type, olint_t protocol, jf_network_socket_t ** ppSocket);

NETWORKAPI u32 NETWORKCALL jf_network_destroySocket(jf_network_socket_t ** ppSocket);

/** Allocates a UDP socket for a given interface.
 *
 *  @note
 *  -# choose a random port number from 55000 to 65000.
 *
 *  @param pjiLocal [in] The interface to bind to.
 *  @param pu16Port [in] The port number.
 *  @param ppSocket [out] The created UDP socket. 
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createDgramSocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, jf_network_socket_t ** ppSocket);

/** Allocates a TCP socket for a given interface.
 *
 *  @note
 *  -# Choose a random port number from 50000 to 65000 if port is 0, it will bind the address
 *   to socket.
 *
 *  @param pjiLocal [in] The interface to bind to.
 *  @param pu16Port [in/out] The port number to bind to.
 *  @param ppSocket [out] The created UDP socket. 
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createStreamSocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, jf_network_socket_t ** ppSocket);

/** Create a TCP socket according to address type.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createTypeStreamSocket(
    u8 u8AddrType, jf_network_socket_t ** ppSocket);

/** Create a UDP socket according to address type.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createTypeDgramSocket(
    u8 u8AddrType, jf_network_socket_t ** ppSocket);

/** IO control the socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_ioctlSocket(
    jf_network_socket_t * pSocket, olint_t req, void * pArg);

/** Set the socket to block mode.
 */
NETWORKAPI u32 NETWORKCALL jf_network_setSocketBlock(jf_network_socket_t * pSocket);

/** Set the socket to unblock mode.
 */
NETWORKAPI u32 NETWORKCALL jf_network_setSocketNonblock(jf_network_socket_t * pSocket);

/** Joining the socket to multicast group.
 */
NETWORKAPI u32 NETWORKCALL jf_network_joinMulticastGroup(
    jf_network_socket_t * pSocket, jf_ipaddr_t * pjiAddr, jf_ipaddr_t * pjiMulticaseAddr);

/** Enabling broadcast for the socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_enableBroadcast(jf_network_socket_t * pSocket);

/** Try to send all data but only send once.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_send(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend);

/** Try to send all data but only send once unless timeout.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to send all data with possible several round, until an error occurs.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendn(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend);

/** Try to send all data with possible several round, until an error occurs or timeout.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendnWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to receive all data but only receive once.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recv(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv);

/** Try to receive all data but only receive once unless timeout.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

/** Try to recveive all data with possible several round, until an error occurs.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvn(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv);

/** Try to recveive all data with possible several round, until an error occurs or timeout.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvnWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

/** Try to receive all data but only receive once, unless timeout.
 *
 *  @note
 *  -# The actual received size is in psRecv.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvfromWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv,
    u32 u32Timeout, jf_ipaddr_t * pjiFrom, u16 * pu16Port);

/** Connect the socket to remote server.
 */
NETWORKAPI u32 NETWORKCALL jf_network_connect(
    jf_network_socket_t * pSocket, const jf_ipaddr_t * pji, u16 u16Port);

/** Connect the socket to remote server with timeout.
 */
NETWORKAPI u32 NETWORKCALL jf_network_connectWithTimeout(
    jf_network_socket_t * pSocket, const jf_ipaddr_t * pji, u16 u16Port, u32 u32Timeout);

/** Listen on the socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_listen(jf_network_socket_t * pSocket, olint_t backlog);

/** Accept the connection for the listening socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_accept(
    jf_network_socket_t * pListen, jf_ipaddr_t * pji, u16 * pu16Port,
    jf_network_socket_t ** ppSocket);

/** Send data to remote address.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendto(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend, const jf_ipaddr_t * pjiTo,
    u16 u16Port);

/** Receive data from remote address.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvfrom(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv, jf_ipaddr_t * pjiTo,
    u16 * pu16Port);

/** Create the socket pair.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createSocketPair(
    olint_t domain, olint_t type, jf_network_socket_t * psPair[2]);

/** Destroy the socket pair.
 */
NETWORKAPI u32 NETWORKCALL jf_network_destroySocketPair(jf_network_socket_t * sPair[2]);

/** Monitor the fd set.
 */
NETWORKAPI u32 NETWORKCALL jf_network_select(
    fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout,
    u32 * pu32Ready);

/** Get socket name.
 */
NETWORKAPI u32 NETWORKCALL jf_network_getSocketName(
    jf_network_socket_t * pSocket, struct sockaddr * pName, olint_t * pnNameLen);

/** Clear socket in fd set.
 */
NETWORKAPI void NETWORKCALL jf_network_clearSocketFromFdSet(
    jf_network_socket_t * pSocket, fd_set * set);

/** Check if the socket is set in fd set.
 */
NETWORKAPI boolean_t NETWORKCALL jf_network_isSocketSetInFdSet(
    jf_network_socket_t * pSocket, fd_set * set);

/** Set socket to fd set.
 */
NETWORKAPI void NETWORKCALL jf_network_setSocketToFdSet(
    jf_network_socket_t * pSocket, fd_set * set);

/** Clear fd set.
 */
NETWORKAPI void NETWORKCALL jf_network_clearFdSet(fd_set * set);

/** Get options on socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_getSocketOption(
    jf_network_socket_t * pSocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t * psOptval);

/** Set options on socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_setSocketOption(
    jf_network_socket_t * pSocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t sOptval);

/*  Async socket.
 */
/** Get options on async socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_getAsocketOption(
    jf_network_asocket_t * pAsocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t * psOptval);

/** Set options on async socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_setAsocketOption(
    jf_network_asocket_t * pAsocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t sOptval);

/*  Network chain definition.
 */

/** Create a chain.
 *
 *  @param ppChain [out] The chain to create.
 * 
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createChain(jf_network_chain_t ** ppChain);

/** Destroy the chain.
 *
 *  @param ppChain [in/out] The chain to destory.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_destroyChain(jf_network_chain_t ** ppChain);

/** Add links to the chain.
 *
 *  @note
 *  -# All objects added to the chain must extend implement jf_network_chain_object_header_t.
 *  @par Example
 *  @code
 *  struct object
 *  {
 *      jf_network_chain_object_header_t header
 *      ...;
 *  }
 *  @endcode
 *
 *  @param pChain [in] The chain to add the link to.
 *  @param pObject [in] The link to add to the chain.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_appendToChain(
    jf_network_chain_t * pChain, jf_network_chain_object_t * pObject);

/** Start a Chain
 *
 *  @note
 *  -# This method will use the current thread. All events and processing will be done on this
 *   thread. This method will not return until jf_network_stopChain() is called.
 *
 *  @param pChain [in] The chain to start.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_startChain(jf_network_chain_t * pChain);

/** Stop a chain, imply the destruction of the chain.
 *
 *  @note
 *  -# This will signal the process to shutdown. When the chain cleans itself up, the thread that
 *   is locked on pChain will return.
 *
 *  @param pChain [in] The chain to stop.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_stopChain(jf_network_chain_t * pChain);

/** Wakeup the chain.
 *
 *  @note This will wake up a chain from sleeping by close the fake socket.
 *
 *  @param pChain [in] The chain to stop.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_wakeupChain(jf_network_chain_t * pChain);

/*  Network utimer definition.
 */

/** Add a timed callback with second granularity.
 *
 *  @param pUtimer [in] The timer.
 *  @param pData [in] The data object to associate with the timed callback.
 *  @param u32Seconds [in] number of seconds for the timed callback.
 *  @param fnCallback [in] The callback function pointer.
 *  @param fnDestroy [in] The abort function pointer, which triggers all non-triggered timed
 *   callbacks, upon shutdown.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_addUtimerItem(
    jf_network_utimer_t * pUtimer, void * pData, u32 u32Seconds,
    jf_network_fnCallbackOfUtimerItem_t fnCallback, jf_network_fnDestroyUtimerItemData_t fnDestroy);

/** Removes timed callback(s) specified by pData from an utimer.
 *
 *  @note
 *  -# If there are multiple item pertaining to pData, all of them are removed.
 *  -# Before destroying the utimer item structure, (*fnDestroy)() is called.
 *
 *  @param pUtimer [in] The utimer object to remove the callback from.
 *  @param pData [in] The data object to remove.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_removeUtimerItem(jf_network_utimer_t * pUtimer, void * pData);

/** Destroy a timer.
 *
 *  @note
 *  -# This method never needs to be explicitly called by the developer. It is called by the chain
 *   as a Destroy.
 *
 *  @param ppUtimer [in/out] The utimer object.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_destroyUtimer(jf_network_utimer_t ** ppUtimer);

/** Creates an empty utimer.
 *
 *  @note All events are triggered on the thread. Developers must NEVER block this thread.
 *
 *  @param pChain [in] The chain to add the utimer to.
 *  @param ppUtimer [out] The utimer.
 *  @param pstrName [in] The name of the utimer object.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createUtimer(
    jf_network_chain_t * pChain, jf_network_utimer_t ** ppUtimer, const olchar_t * pstrName);

NETWORKAPI void NETWORKCALL jf_network_dumpUtimerItem(jf_network_utimer_t * pUtimer);

/** Async server socket.
 */

/** Create async server socket.
 *
 *  @param pChain [in] The chain to add this assocket to.
 *  @param ppAssocket [out] The async server socket.
 *  @param pjnacp [in] The parameters for creating assocket.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createAssocket(
    jf_network_chain_t * pChain, jf_network_assocket_t ** ppAssocket,
    jf_network_assocket_create_param_t * pjnacp);

/** Destroy async server socket.
 *
 */
NETWORKAPI u32 NETWORKCALL jf_network_destroyAssocket(jf_network_assocket_t ** ppAssocket);

/** Returns the port number the server is bound to.
 *
 *  @param pAssocket [in] The assocket to query.
 *
 *  @return The listening port number.
 */
NETWORKAPI u16 NETWORKCALL jf_network_getPortNumberOfAssocket(jf_network_assocket_t * pAssocket);

/** Returns the user's Tag associated with the assocket.
 *
 *  @param pAssocket [in] The assocket to query.
 *
 *  @return The user tag.
 */
NETWORKAPI void * NETWORKCALL jf_network_getTagOfAssocket(jf_network_assocket_t * pAssocket);

/** Sets the user's tag associated with the assocket.
 *
 *  @param pAssocket [in] The assocket to save the tag to.
 *  @param pTag [in] The tag.
 */
NETWORKAPI void NETWORKCALL jf_network_setTagOfAssocket(
    jf_network_assocket_t * pAssocket, void * pTag);

/** Disconnect the async socket accepted by server socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_disconnectAssocket(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket);

/** Send data on the async server socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendAssocketData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t sBuf);

/** Async client socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createAcsocket(
    jf_network_chain_t * pChain, jf_network_acsocket_t ** ppAcsocket,
    jf_network_acsocket_create_param_t * pjnacp);

/** Destroy async client socket.
 *
 *  @param ppAcsocket [in/out] The async client socket.
 *
 *  @return The error code.
 */
NETWORKAPI u32 NETWORKCALL jf_network_destroyAcsocket(jf_network_acsocket_t ** ppAcsocket);

/** Returns the user's tag associated with the acsocket.
 *
 *  @param pAcsocket [in] The acsocket to query.
 *
 *  @return The user Tag.
 */
NETWORKAPI void * NETWORKCALL jf_network_getTagOfAcsocket(jf_network_acsocket_t * pAcsocket);

/** Sets the user's tag associated with the acsocket.
 *
 *  @param pAcsocket [in] The acsocket to save the tag to.
 *  @param pTag [in] The user's tag.
 */
NETWORKAPI void NETWORKCALL jf_network_setTagOfAcsocket(
    jf_network_acsocket_t * pAcsocket, void * pTag);

/** Check if the async socket is free.
 */
NETWORKAPI boolean_t NETWORKCALL jf_network_isAcsocketFree(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket);

/** Connect the async client socket to remote server.
 */
NETWORKAPI u32 NETWORKCALL jf_network_connectAcsocketTo(
    jf_network_acsocket_t * pAcsocket, jf_ipaddr_t * pjiRemote, u16 u16RemotePort, void * pUser);

/** Disconnect the async socket in async client socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_disconnectAcsocket(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket);

/** Send data on the async client socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendAcsocketData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t sBuf);

/** Get local interface of the async socket.
 */
NETWORKAPI void NETWORKCALL jf_network_getLocalInterfaceOfAcsocket(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr);

/** Resolve host name to IP.
 */
NETWORKAPI u32 NETWORKCALL jf_network_getHostByName(
    const olchar_t * pstrName, struct hostent ** ppHostent);

#endif /*JIUFENG_NETWORK_H */

/*------------------------------------------------------------------------------------------------*/

