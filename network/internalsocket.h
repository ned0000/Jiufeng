/**
 *  @file internalsocket.h
 *
 *  @brief internal socket data structure and routines shared in network library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef NETWORK_INTERNALSOCKET_H
#define NETWORK_INTERNALSOCKET_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "network.h"

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */

#if defined(LINUX)
    typedef olint_t isocket_t;
    #define INVALID_ISOCKET -1
#elif defined(WINDOWS)
    typedef SOCKET isocket_t;
    #define INVALID_ISOCKET INVALID_SOCKET
#endif

typedef struct
{
    isocket_t is_isSocket;
    boolean_t is_bSecure;
    u8 is_u8Reserved[7];
    u32 is_u32Reserved[8];
    void * is_pPrivate;
} internal_socket_t;

/* --- functional routines ------------------------------------------------- */
u32 newIsocket(internal_socket_t ** ppIsocket);

u32 newIsocketWithSocket(internal_socket_t ** ppIsocket, isocket_t sock);

u32 freeIsocket(internal_socket_t ** ppIsocket);

u32 createIsocket(
    olint_t domain, olint_t type, olint_t protocol,
    internal_socket_t ** ppIsocket);

u32 destroyIsocket(internal_socket_t ** ppIsocket);

/** Allocates a UDP socket for a given interface, choosing a random port number
 *  from 55000 to 65000
 */
u32 createDgramIsocket(
    ip_addr_t * piaLocal, u16 * pu16Port, internal_socket_t ** ppIsocket);

/** Allocates a TCP socket for a given interface, choosing a random port number
 *  from 55000 to 65000
 */
u32 createStreamIsocket(
    ip_addr_t * piaLocal, u16 * pu16Port, internal_socket_t ** ppIsocket);

u32 ioctlIsocket(
    internal_socket_t * pis, olint_t req, void * pArg);

u32 setIsocketBlock(internal_socket_t * pis);

u32 setIsocketNonblock(internal_socket_t * pis);

u32 isJoinMulticastGroup(
    internal_socket_t * pis, ip_addr_t * piaAddr, ip_addr_t * piaMulticaseAddr);

u32 isEnableBroadcast(internal_socket_t * pis);

/** Try to send all data but only send once, the actual sent size is in psSend
 */
u32 isSend(internal_socket_t * pis, void * pBuffer, olsize_t * psSend);

/** Try to send all data but only send once, unless timeout the actual sent
 *  size is in psSend
 */
u32 isSendWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to send all data with possible several round, until an error occurs,
 *  the actual sent size is in psSend
 */
u32 isSendn(
    internal_socket_t * pis, void * pBuffer, olsize_t * psSend);

/** Try to send all data with possible several round, until an error occurs or
 *  timeout, the actual sent size is in psSend
 */
u32 isSendnWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to recveive all data but only recveive once, the actual received size is
 *  in psRecv
 */
u32 isRecv(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv);

/** Try to recveive all data but only recveive once, unless timeout the actual
 *  received size is in psRecv
 */
u32 isRecvWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

u32 isRecvfromWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv,
    u32 u32Timeout, ip_addr_t * piaFrom, u16 * pu16Port);

/** Try to recveive all data with possible several round, until an error occurs,
 *  the actual recveived size is in psRecv
 */
u32 isRecvn(internal_socket_t * pis,
    void * pBuffer, olsize_t * psRecv);

/** Try to recveive all data with possible several round, until an error occurs
 *  or timeout, the actual recveived size is in psRecv
 */
u32 isRecvnWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

u32 isConnect(
    internal_socket_t * pis, const ip_addr_t * pia, u16 u16Port);

u32 isConnectWithTimeout(
    internal_socket_t * pis, const ip_addr_t * pia, u16 u16Port,
    u32 u32Timeout);

u32 isListen(internal_socket_t * pis, olint_t backlog);

u32 isAccept(
    internal_socket_t * pisListen, ip_addr_t * pia,
    u16 * pu16Port, internal_socket_t ** ppIsocket);

u32 isSendto(
    internal_socket_t * pis, void * pBuffer,
    olsize_t * psSend, const ip_addr_t * piaTo, u16 u16Port);

u32 isRecvfrom(
    internal_socket_t * pis, void * pBuffer,
    olsize_t * psRecv, ip_addr_t * piaTo, u16 * pu16Port);

u32 isSelect(
    fd_set * readfds, fd_set * writefds,
    fd_set * exceptfds, struct timeval * timeout, u32 * pu32Ready);

u32 getIsocketName(
    internal_socket_t * pis, struct sockaddr * pName, olint_t * pnNameLen);

void clearIsocketFromFdSet(
    internal_socket_t * pis, fd_set * set);

boolean_t isIsocketSetInFdSet(
    internal_socket_t * pis, fd_set * set);

void setIsocketToFdSet(internal_socket_t * pis, fd_set * set);

void clearIsocketFdSet(fd_set * set);

u32 isGetSockOpt(
    internal_socket_t * pis, olint_t level, olint_t optname,
    void * optval, olsize_t * optlen);

u32 isSetSockOpt(
    internal_socket_t * pis, olint_t level, olint_t optname,
    void * optval, olsize_t optlen);

#if defined(WINDOWS)
/** For internal use and Windows platform only, a wrapper to WSAIoctl
 */
u32 WSAIoctlIsocket(
    internal_socket_t * pis, DWORD dwIoControlCode,
    LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer,
    DWORD cbOutBuffer, LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped,
    LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
#endif

#endif /*NETWORK_INTERNALSOCKET_H*/

/*---------------------------------------------------------------------------*/


