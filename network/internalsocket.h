/**
 *  @file internalsocket.h
 *
 *  @brief Header file defines the internal socket data structure and routines shared in network
 *   library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef NETWORK_INTERNALSOCKET_H
#define NETWORK_INTERNALSOCKET_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_network.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

#if defined(LINUX)

/** Define the socket data type for Linux.
 */
    typedef olint_t                            isocket_t;

/** Define the invalid socket constant for Linux.
 */
    #define INVALID_ISOCKET                    (-1)

#elif defined(WINDOWS)

/** Define the socket data type for Windows.
 */
    typedef SOCKET                             isocket_t;

/** Define the invalid socket constant for Windows.
 */
    #define INVALID_ISOCKET                    INVALID_SOCKET

#endif

/** Define the internal socket data type.
 */
typedef struct
{
    /**The socket.*/
    isocket_t is_isSocket;
    /**It's secure socket if it's TRUE.*/
    boolean_t is_bSecure;
    u8 is_u8Reserved[7];
    u32 is_u32Reserved[8];
    /**Private data for the internal socket.*/
    void * is_pPrivate;
} internal_socket_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Create an empty internal socket.
 *
 *  @param ppIsocket [out] The internal socket to be created and returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 newIsocket(internal_socket_t ** ppIsocket);

/** Create an internal socket with specified socket.
 *
 *  @param ppIsocket [out] The internal socket to be created and returned.
 *  @param sock [in] The socket from user.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 newIsocketWithSocket(internal_socket_t ** ppIsocket, isocket_t sock);

/** Free the internal socket.
 *
 *  @param ppIsocket [in/out] The internal socket to be freed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 freeIsocket(internal_socket_t ** ppIsocket);

/** Create an internal socket with specified domain, type and protocol.
 *
 *  @param domain [in] The communication domain, one of AF_INET, AF_INET6 or AF_UNIX.
 *  @param type [in] The communication semantics, one of SOCK_STREAM, SOCK_DGRAM.
 *  @param protocol [in] Specifie a particular protocol to be used with the socket, usually 0.
 *  @param ppIsocket [out] The internal socket to be created and returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_CREATE_SOCKET Failed to create socket.
 */
u32 createIsocket(
    olint_t domain, olint_t type, olint_t protocol, internal_socket_t ** ppIsocket);

/** Destroy the internal socket.
 *
 *  @param ppIsocket [in/out] The internal socket to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 destroyIsocket(internal_socket_t ** ppIsocket);

/** Allocates a UDP socket for a given interface.
 *
 *  @note
 *  -# Choose a random port number from 55000 to 65000 and bind the address to socket.
 *  -# The domain is determined by address type.
 *  -# The type is always SOCK_DGRAM.
 *  -# The protocol is always 0.
 *
 *  @param pjiLocal [in] The interface to bind to.
 *  @param pu16Port [in/out] The port number to bind to.
 *  @param ppIsocket [out] The created datagram socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 createDgramIsocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, internal_socket_t ** ppIsocket);

/** Allocates a TCP socket for a given interface.
 *
 *  @note
 *  -# Choose a random port number from 50000 to 65000 if port is 0, it will bind the address
 *   to socket.
 *  -# The domain is determined by address type.
 *  -# The type is always SOCK_STREAM.
 *  -# The protocol is always 0.
 *  -# For unix domain socket, port number is ignored.
 *
 *  @param pjiLocal [in] The interface to bind to.
 *  @param pu16Port [in/out] The port number to bind to.
 *  @param ppIsocket [out] The created stream socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 createStreamIsocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, internal_socket_t ** ppIsocket);

/** IO control the internal socket.
 *
 *  @param pis [in] The internal socket to control.
 *  @param req [in] The request id.
 *  @param pArg [in] Argument for the request.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 ioctlIsocket(internal_socket_t * pis, olint_t req, void * pArg);

/** Set the internal socket to block mode.
 *
 *  @param pis [in] The internal socket to control.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 setIsocketBlock(internal_socket_t * pis);

/** Set the internal socket to unblock mode.
 *
 *  @param pis [in] The internal socket to control.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 setIsocketNonblock(internal_socket_t * pis);

/** Joining the socket to multicast group.
 *
 *  @param pis [in] The internal socket to control.
 *  @param pjiAddr [in] The interface to join the multcast address.
 *  @param pjiMulticaseAddr [in] The multcast address.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 isJoinMulticastGroup(
    internal_socket_t * pis, jf_ipaddr_t * pjiAddr, jf_ipaddr_t * pjiMulticaseAddr);

/** Enabling broadcast for the internal socket.
 *
 *  @param pis [in] The internal socket to control.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 isEnableBroadcast(internal_socket_t * pis);

/** Try to send all data but only send once.
 *
 *  @param pis [in] The internal socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 */
u32 isSend(internal_socket_t * pis, void * pBuffer, olsize_t * psSend);

/** Try to send all data but only send once unless timeout.
 *
 *  @param pis [in] The internal socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 */
u32 isSendWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to send all data with possible several round, until an error occurs.
 *
 *  @param pis [in] The internal socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 */
u32 isSendn(internal_socket_t * pis, void * pBuffer, olsize_t * psSend);

/** Try to send all data with possible several round, until an error occurs or timeout.
 *
 *  @param pis [in] The internal socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 */
u32 isSendnWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to recveive all data but only recveive once.
 *
 *  @param pis [in] The internal socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
u32 isRecv(internal_socket_t * pis, void * pBuffer, olsize_t * psRecv);

/** Try to recveive all data but only recveive once unless timeout.
 *
 *  @param pis [in] The internal socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
u32 isRecvWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

/** Try to recveive all data with possible several round, until an error occurs.
 *
 *  @param pis [in] The internal socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
u32 isRecvn(internal_socket_t * pis, void * pBuffer, olsize_t * psRecv);

/** Try to recveive all data with possible several round, until an error occurs or timeout.
 *
 *  @param pis [in] The internal socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
u32 isRecvnWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

/** Connect the internal socket to remote server.
 *
 *  @param pis [in] The internal socket to connect.
 *  @param pji [in] The remote server's address.
 *  @param u16Port [in] The remote server's port number.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_INITIATE_CONNECTION Failed to initiate connection.
 */
u32 isConnect(
    internal_socket_t * pis, const jf_ipaddr_t * pji, u16 u16Port);

/** Connect the internal socket to remote server with timeout.
 *
 *  @param pis [in] The internal socket to connect.
 *  @param pji [in] The remote server's address.
 *  @param u16Port [in] The remote server's port number.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_INITIATE_CONNECTION Failed to initiate connection.
 */
u32 isConnectWithTimeout(
    internal_socket_t * pis, const jf_ipaddr_t * pji, u16 u16Port, u32 u32Timeout);

/** Listen on the internal socket.
 *
 *  @param pis [in] The internal socket to listen on.
 *  @param backlog [in] Defines the maximum length to which the queue of pending connections.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_LISTEN_ON_SOCKET Failed to listen on socket.
 */
u32 isListen(internal_socket_t * pis, olint_t backlog);

/** Accept the connection for the listening socket.
 *
 *  @param pisListen [in] The internal socket to listen on.
 *  @param pji [out] The address of peer socket.
 *  @param pu16Port [out] The port of peer socket.
 *  @param ppIsocket [out] The accepted internal socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_ACCEPT_CONNECTION Failed to accept connection.
 */
u32 isAccept(
    internal_socket_t * pisListen, jf_ipaddr_t * pji, u16 * pu16Port,
    internal_socket_t ** ppIsocket);

/** Send data to remote address.
 *
 *  @note
 *  -# This function is for SOCK_DGRAM type socket only.
 *
 *  @param pis [in] The internal socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *  @param pjiTo [out] The address data should send to.
 *  @param u16Port [out] The port data should send to.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 isSendto(
    internal_socket_t * pis, void * pBuffer, olsize_t * psSend, const jf_ipaddr_t * pjiTo,
    u16 u16Port);

/** Receive data from remote address.
 *
 *  @note
 *  -# This function is for SOCK_DGRAM type socket only.
 *
 *  @param pis [in] The internal socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *  @param pjiFrom [out] The data received from.
 *  @param pu16Port [out] The data received from.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 isRecvfrom(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv, jf_ipaddr_t * pjiFrom,
    u16 * pu16Port);

/** Try to receive all data but only receive once unless timeout.
 *
 *  @note
 *  -# This function is for SOCK_DGRAM type socket only.
 *
 *  @param pis [in] The internal socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *  @param pjiFrom [out] The data received from.
 *  @param pu16Port [out] The data received from.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 isRecvfromWithTimeout(
    internal_socket_t * pis, void * pBuffer, olsize_t * psRecv, u32 u32Timeout,
    jf_ipaddr_t * pjiFrom, u16 * pu16Port);

/** Monitor the file descriptor set.
 *
 *  @param readfds [in/out] The read file descriptor set.
 *  @param writefds [in/out] The write file descriptor set.
 *  @param exceptfds [in/out] The exception file descriptor set.
 *  @param timeout [in/out] The timeout value.
 *  @param pu32Ready [out] Number of socket ready.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 isSelect(
    fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout,
    u32 * pu32Ready);

/** Return current address to which the socket is bound.
 *
 *  @param pis [in] The internal socket to get name.
 *  @param pName [out] The address buffer.
 *  @param pnNameLen [out] The size of the buffer as in parameter and actual size as out parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 getIsocketName(
    internal_socket_t * pis, struct sockaddr * pName, olint_t * pnNameLen);

/** Clear file descriptor set.
 *
 *  @param pis [in] The internal socket.
 *  @param set [in] The file descriptor set to clear.
 *
 *  @return Void.
 */
void clearIsocketFromFdSet(internal_socket_t * pis, fd_set * set);

/** Check if the internal socket is set in file descriptor set.
 *
 *  @param pis [in] The internal socket to check.
 *  @param set [in] The file descriptor set.
 *
 *  @return The status.
 *  @retval TRUE The socket is set in the file descriptor set.
 *  @retval FALSE The socket is not set in the file descriptor set.
 */
boolean_t isIsocketSetInFdSet(internal_socket_t * pis, fd_set * set);

/** Set socket to file descriptor set.
 *
 *  @param pis [in] The internal socket to set.
 *  @param set [in] The file descriptor set.
 *
 *  @return Void.
 */
void setIsocketToFdSet(internal_socket_t * pis, fd_set * set);

/** Clear file descriptor set.
 *
 *  @param set [in] The file descriptor set to clear.
 *
 *  @return Void.
 */
void clearIsocketFdSet(fd_set * set);

/** Get options on the internal socket.
 *
 *  @param pis [in] The internal socket to get option.
 *  @param level [in] The level of the option.
 *  @param optname [in] The option name.
 *  @param optval [out] The option value buffer.
 *  @param optlen [in/out] The option value buffer size as in parameter and actual size as out
 *   parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 isGetSockOpt(
    internal_socket_t * pis, olint_t level, olint_t optname, void * optval, olsize_t * optlen);

/** Set options on the internal socket.
 *
 *  @param pis [in] The internal socket to set option.
 *  @param level [in] The level of the option.
 *  @param optname [in] The option name.
 *  @param optval [in] The option value.
 *  @param optlen [in] The option value size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 isSetSockOpt(
    internal_socket_t * pis, olint_t level, olint_t optname, void * optval, olsize_t optlen);

#if defined(WINDOWS)
/** For internal use and Windows platform only, a wrapper to WSAIoctl
 *
 *  @param pis [in] The internal socket to control.
 *  @param dwIoControlCode [in] IO control code.
 *  @param lpvInBuffer [in] The in buffer.
 *  @param cbInBuffer [in] Size of in buffer.
 *  @param lpvOutBuffer [out] The out buffer.
 *  @param cbOutBuffer [in] Size of out buffer.
 *  @param lpcbBytesReturned [out] Size of bytes returned in out buffer.
 *  @param lpOverlapped [in] Overlap option.
 *  @param lpCompletionRoutine [in] Completion routine.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
u32 WSAIoctlIsocket(
    internal_socket_t * pis, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer,
    LPVOID lpvOutBuffer, DWORD cbOutBuffer, LPDWORD lpcbBytesReturned,
    LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
#endif

#endif /*NETWORK_INTERNALSOCKET_H*/

/*------------------------------------------------------------------------------------------------*/
