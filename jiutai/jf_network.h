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
 *  @param readset [in/out] Read file descriptor set.
 *  @param writeset [in/out] Write file descriptor set.
 *  @param errorset [in/out] Error file descriptor set.
 *  @param pu32BlockTime [in/out] Timeout in millisecond for select.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnPreSelectChainObject_t)(
    jf_network_chain_object_t * pObject, fd_set * readset, fd_set * writeset, fd_set * errorset,
    u32 * pu32BlockTime);

/** Callback function after select().
 *
 *  @param pObject [in] Chain object.
 *  @param nReady [in] Number of ready file descriptor.
 *  @param readset [in] Read file descriptor set.
 *  @param writeset [in] Write file descriptor set.
 *  @param errorset [in] Error file descriptor set.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnPostSelectChainObject_t)(
    jf_network_chain_object_t * pObject, olint_t nReady, fd_set * readset, fd_set * writeset,
    fd_set * errorset);

/** Header of chain object, MUST be placed at the beginning of the object.
 */
typedef struct
{
    /*The callback function which is called before entering select.*/
    jf_network_fnPreSelectChainObject_t jncoh_fnPreSelect;
    /*The callback function which is called after exiting select.*/
    jf_network_fnPostSelectChainObject_t jncoh_fnPostSelect;
} jf_network_chain_object_header_t;

/** Define the network utimer data type.
 */
typedef void  jf_network_utimer_t;

/** The callback function is called when the timer is triggerred.
 *
 *  @param pData [in] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnCallbackOfUtimerItem_t)(void * pData);

/** The callback function is called when the timer item is destroyed.
 *
 *  @param ppData [in/out] The argument for the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnDestroyUtimerItemData_t)(void ** ppData);

/*  Async server socket.
 */

/** The function is to notify upper layer there are incoming data.
 *
 *  @note
 *  -# The user object is set by fnAssocketOnConnect_t when a new incoming connection is accepted.
 *  -# After process, the begin pointer should be set to end pointer so the buffer is recycled.
 *  -# If the begin pointer is not changed, the data will be kept in buffer.
 *
 *  @param pAssocket [in] The async server socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param pu8Buffer [in] The buffer for the incoming data.
 *  @param psBeginPointer [in/out] The begin pointer for the available data.
 *  @param sEndPointer [in] The end pointer for the available data.
 *  @param pUser [in] User object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnAssocketOnData_t)(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser);

/** The function is to notify upper layer there are new connection.
 *
 *  @note
 *  -# The connection is already accepted.
 *
 *  @param pAssocket [in] The async server socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param ppUser [out] User object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnAssocketOnConnect_t)(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, void ** ppUser);

/** The function is to notify upper layer a connection is closed.
 *
 *  @note
 *  -# The access to the asocket being closed is not allowed.
 *  -# DO NOT use asDisconnect in this callback function as the connection is closed already.
 *
 *  @param pAssocket [in] The async server socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param u32Status [in] The reason why the connection is closed.
 *  @param pUser [in] User object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnAssocketOnDisconnect_t)(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    void * pUser);

/** The function is to notify upper layer the data is sent to peer successfully.
 *
 *  @note
 *  -# The status should be checked for the send operation. The data is successfully sent if the
 *   status is JF_ERR_NO_ERROR; otherwise it's failed.
 *  -# The data content can be checked. And the buffer pointer is not the oririnal one. 
 *
 *  @param pAssocket [in] The async server socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param u32Status [in] The status of the send operation.
 *  @param pu8Buffer [in] The sent buffer.
 *  @param sBuf [in] The length of the buffer.
 *  @param pUser [in] User object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
    /**Address of server.*/
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
 *
 *  @note
 *  -# The user object is passed by jf_network_connectAcsocketTo() when trying to initiate a new
 *   connection.
 *  -# After process, the begin pointer should be set to end pointer so the buffer is recycled.
 *  -# If the begin pointer is not changed, the data will be kept in buffer.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param pu8Buffer [in] The buffer for the incoming data.
 *  @param psBeginPointer [in/out] The begin pointer for the available data.
 *  @param sEndPointer [in] The end pointer for the available data.
 *  @param pUser [in] User object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnAcsocketOnData_t)(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser);

/** The function is to notify upper layer there are new connection.
 *
 *  @note
 *  -# The status should be checked for the connection. The connection is successfully established
 *   if the status is JF_ERR_NO_ERROR; otherwise it's failed.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param u32Status [in] The connection status.
 *  @param pUser [in] User object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param u32Status [in] The reason why the connection is closed.
 *  @param pUser [in] User object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
typedef u32 (* jf_network_fnAcsocketOnDisconnect_t)(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u32 u32Status,
    void * pUser);

/** The function is to notify upper layer the data is sent to peer successfully.
 *
 *  -# The status should be checked for the send operation. The data is successfully sent if the
 *   status is JF_ERR_NO_ERROR; otherwise it's failed.
 *  -# The data content can be checked. And the buffer pointer is not the oririnal one. 
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param u32Status [in] The status of the send operation.
 *  @param pu8Buffer [in] The sent buffer.
 *  @param sBuf [in] The length of the buffer.
 *  @param pUser [in] User object.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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


/* Data transfer */

/** Callback function for getting full data size received from server based on the header.
 */
typedef olsize_t (* jf_network_fnGetFullDataSize_t)(void * pHeader, olsize_t sHeader);

/** Define the parameter for transfering data.
 */
typedef struct
{
    /**Server should reply if it's TRUE.*/
    boolean_t jntdp_bReply;
    u8 jntdp_u8Reserved[7];
    /**Server address.*/
    jf_ipaddr_t * jntdp_pjiServer;
    /**Server port.*/
    u16 jntdp_u16Port;
    u16 jntdp_u16Reserved;
    /**Timeout for connecting to server, sending and receiving data.*/
    u32 jntdp_u32Timeout;
    /**Buffer for sending data.*/
    void * jntdp_pSendBuf;
    /**Size of sending data buffer.*/
    olsize_t jntdp_sSendBuf;
    /**Buffer for receiving data.*/
    void * jntdp_pRecvBuf;
    /**Size of receiving data buffer.*/
    olsize_t jntdp_sRecvBuf;
    /**Size of received data.*/
    olsize_t jntdp_sRecvData;
    /**Size of the header for receiving data.*/
    olsize_t jntdp_sHeader;
    /**Callback function to get full data size.*/
    jf_network_fnGetFullDataSize_t jntdp_fnGetFullDataSize;
} jf_network_transfer_data_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/*  Network socket routine.
 */

/** Create socket.
 *
 *  @param domain [in] The communication domain, one of AF_INET, AF_INET6 or AF_UNIX.
 *  @param type [in] The communication semantics, one of SOCK_STREAM, SOCK_DGRAM.
 *  @param protocol [in] Specifie a particular protocol to be used with the socket, usually 0.
 *  @param ppSocket [out] The socket to be created and returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_CREATE_SOCKET Failed to create socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createSocket(
    olint_t domain, olint_t type, olint_t protocol, jf_network_socket_t ** ppSocket);

/** Destroy socket.
 *
 *  @param ppSocket [in/out] The socket to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_CLOSE_SOCKET Failed to close socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_destroySocket(jf_network_socket_t ** ppSocket);

/** Allocates a UDP socket for a given interface.
 *
 *  @note
 *  -# Choose a random port number from 55000 to 65000 and bind the address to socket.
 *  -# The domain is determined by address type.
 *  -# The type is always SOCK_DGRAM.
 *  -# The protocol is always 0.
 *
 *  @param pjiLocal [in] The interface to bind to.
 *  @param pu16Port [in/out] The port number.
 *  @param ppSocket [out] The created UDP socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createDgramSocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, jf_network_socket_t ** ppSocket);

/** Allocates a TCP socket for a given interface.
 *
 *  @note
 *  -# Choose a random port number from 50000 to 65000 if port is 0, it will bind the address
 *   to socket.
 *  -# The domain is determined by address type.
 *  -# The type is always SOCK_STREAM.
 *  -# The protocol is always 0.
 *
 *  @param pjiLocal [in] The interface to bind to.
 *  @param pu16Port [in/out] The port number to bind to.
 *  @param ppSocket [out] The created UDP socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createStreamSocket(
    jf_ipaddr_t * pjiLocal, u16 * pu16Port, jf_network_socket_t ** ppSocket);

/** Create a TCP socket according to address type.
 *
 *  @note
 *  -# The domain is determined by address type.
 *  -# The type is always SOCK_STREAM.
 *  -# The protocol is always 0.
 *  -# No binding operation in this function as no port is specified.
 *
 *  @param u8AddrType [in] The address type.
 *  @param ppSocket [out] The socket to be created and returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createTypeStreamSocket(
    u8 u8AddrType, jf_network_socket_t ** ppSocket);

/** Create a UDP socket according to address type.
 *
 *  @note
 *  -# The domain is determined by address type.
 *  -# The type is always SOCK_DGRAM.
 *  -# The protocol is always 0.
 *  -# No binding operation in this function as no port is specified.
 *
 *  @param u8AddrType [in] The address type.
 *  @param ppSocket [out] The socket to be created and returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createTypeDgramSocket(
    u8 u8AddrType, jf_network_socket_t ** ppSocket);

/** IO control the socket.
 *
 *  @param pSocket [in] The socket to control.
 *  @param req [in] The request id.
 *  @param pArg [in] Argument for the request.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_ioctlSocket(
    jf_network_socket_t * pSocket, olint_t req, void * pArg);

/** Set the socket to block mode.
 *
 *  @param pSocket [in] The socket to control.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_setSocketBlock(jf_network_socket_t * pSocket);

/** Set the socket to unblock mode.
 *
 *  @param pSocket [in] The socket to control.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_setSocketNonblock(jf_network_socket_t * pSocket);

/** Joining the socket to multicast group.
 *
 *  @param pSocket [in] The socket to control.
 *  @param pjiAddr [in] The interface to join the multcast address.
 *  @param pjiMulticaseAddr [in] The multcast address.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_joinMulticastGroup(
    jf_network_socket_t * pSocket, jf_ipaddr_t * pjiAddr, jf_ipaddr_t * pjiMulticaseAddr);

/** Enabling broadcast for the socket.
 *
 *  @param pSocket [in] The socket to control.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_enableBroadcast(jf_network_socket_t * pSocket);

/** Try to send all data but only send once.
 *
 *  @note
 *  -# Data is send only once.
 *  -# Data may be partially sent.
 *  -# The send operation may be interrupted by signal, or full output queue.
 *  -# If the socket is blocking, the routine will not return until error or all data are sent.
 *
 *  @param pSocket [in] The socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_send(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend);

/** Try to send all data but only send once, the send operation will stop if timeout.
 *
 *  @note
 *  -# Data is send only once.
 *  -# Data may be partially sent.
 *  -# The send operation may be interrupted by signal, or full output queue, or timeout.
 *  -# The socket is watched to see if space is available for write by multiplexing.
 *
 *  @param pSocket [in] The socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to send all data with possible several rounds, until an error occurs.
 *
 *  @note
 *  -# Data is send with several rounds.
 *  -# Data may be partially sent.
 *  -# Signal is handled.
 *  -# The send operation may be interrupted by other errors like full output queue.
 *  -# If the socket is blocking, the routine will not return until error or all data are sent.
 *
 *  @param pSocket [in] The socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendn(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend);

/** Try to send all data with possible several rounds, until an error occurs or timeout.
 *
 *  @note
 *  -# Data is send with several rounds.
 *  -# Data may be partially sent.
 *  -# Signal is handled.
 *  -# The send operation may be interrupted by other errors like full output queue or timeout.
 *  -# The socket is watched to see if space is available for write by multiplexing.
 *
 *  @param pSocket [in] The socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendnWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to receive all data but only receive once.
 *
 *  @note
 *  -# Data is received only once.
 *  -# Data may be partially received due to signal, or other errors.
 *  -# If the socket is blocking, the routine will not return until error or all data are received.
 *
 *  @param pSocket [in] The socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recv(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv);

/** Try to receive all data but only receive once, the receive operation will stop if timeout.
 *
 *  @note
 *  -# Data is received only once.
 *  -# Data may be partially received due to signal, timeout or other errors.
 *  -# The socket is watched to see if data is available for read by multiplexing.
 *
 *  @param pSocket [in] The socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

/** Try to receive all data with possible several round, until an error occurs.
 *
 *  @note
 *  -# Data is received with several rounds.
 *  -# Data may be partially received due to the errors.
 *  -# Signal is handled.
 *  -# If the socket is blocking, the routine will not return until error or all data are received.
 *
 *  @param pSocket [in] The socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvn(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv);

/** Try to recveive all data with possible several round, until an error occurs or timeout.
 *
 *  @note
 *  -# Data is received with several rounds.
 *  -# Data may be partially received due to timeout or other errors.
 *  -# Signal is handled.
 *  -# The socket is watched to see if data is available for read by multiplexing.
 *
 *  @param pSocket [in] The socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvnWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

/** Connect the socket to remote server.
 *
 *  @param pSocket [in] The socket to connect.
 *  @param pji [in] The remote server's address.
 *  @param u16Port [in] The remote server's port number.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_INITIATE_CONNECTION Failed to initiate connection.
 */
NETWORKAPI u32 NETWORKCALL jf_network_connect(
    jf_network_socket_t * pSocket, const jf_ipaddr_t * pji, u16 u16Port);

/** Connect the socket to remote server with timeout.
 *
 *  @param pSocket [in] The socket to connect.
 *  @param pji [in] The remote server's address.
 *  @param u16Port [in] The remote server's port number.
 *  @param u32Timeout [in] The timeout value in second.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_INITIATE_CONNECTION Failed to initiate connection.
 */
NETWORKAPI u32 NETWORKCALL jf_network_connectWithTimeout(
    jf_network_socket_t * pSocket, const jf_ipaddr_t * pji, u16 u16Port, u32 u32Timeout);

/** Listen on the socket.
 *
 *  @param pSocket [in] The listening socket.
 *  @param backlog [in] Defines the maximum length to which the queue of pending connections.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_LISTEN_ON_SOCKET Failed to listen on socket.
 */
NETWORKAPI u32 NETWORKCALL jf_network_listen(jf_network_socket_t * pSocket, olint_t backlog);

/** Accept the connection for the listening socket.
 *
 *  @param pListen [in] The listening socket.
 *  @param pji [out] The address of peer socket.
 *  @param pu16Port [out] The port of peer socket. 
 *  @param ppSocket [out] The accepted socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_ACCEPT_CONNECTION Failed to accept connection.
 */
NETWORKAPI u32 NETWORKCALL jf_network_accept(
    jf_network_socket_t * pListen, jf_ipaddr_t * pji, u16 * pu16Port,
    jf_network_socket_t ** ppSocket);

/** Send data to remote address.
 *
 *  @note
 *  -# This function is for SOCK_DGRAM type socket only.
 *  -# The actual sent size is in psSend.
 *
 *  @param pSocket [in] The socket to send data.
 *  @param pBuffer [in] The data buffer.
 *  @param psSend [in/out] The buffer size as in parameter and actual sent size as out parameter.
 *  @param pjiTo [out] The address data should send to.
 *  @param u16Port [out] The port data should send to.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendto(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psSend, const jf_ipaddr_t * pjiTo,
    u16 u16Port);

/** Receive data from remote address.
 *
 *  @note
 *  -# This function is for SOCK_DGRAM type socket only.
 *
 *  @param pSocket [in] The socket to receive data.
 *  @param pBuffer [in] The data buffer.
 *  @param psRecv [in/out] The buffer size as in parameter and actual received size as out
 *   parameter.
 *  @param pjiFrom [out] The data received from.
 *  @param pu16Port [out] The data received from.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_recvfrom(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv, jf_ipaddr_t * pjiFrom,
    u16 * pu16Port);

/** Try to receive all data but only receive once, unless timeout.
 *
 *  @note
 *  -# This function is for SOCK_DGRAM type socket only.
 *
 *  @param pSocket [in] The socket to receive data.
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
NETWORKAPI u32 NETWORKCALL jf_network_recvfromWithTimeout(
    jf_network_socket_t * pSocket, void * pBuffer, olsize_t * psRecv, u32 u32Timeout,
    jf_ipaddr_t * pjiFrom, u16 * pu16Port);

/** Create the socket pair.
 *
 *  @param domain [in] The communication domain, one of AF_INET, AF_INET6 or AF_UNIX.
 *  @param type [in] The communication semantics, one of SOCK_STREAM, SOCK_DGRAM.
 *  @param psPair [out] The socket pair to be created and returned.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createSocketPair(
    olint_t domain, olint_t type, jf_network_socket_t * psPair[2]);

/** Destroy the socket pair.
 *
 *  @param psPair [in/out] The socket pair to be destroyed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_destroySocketPair(jf_network_socket_t * psPair[2]);

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
NETWORKAPI u32 NETWORKCALL jf_network_select(
    fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout,
    u32 * pu32Ready);

/** Return current address to which the socket is bound.
 *
 *  @param pSocket [in] The socket to get name.
 *  @param pName [out] The address buffer.
 *  @param pnNameLen [out] The size of the buffer as in parameter and actual size as out parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_getSocketName(
    jf_network_socket_t * pSocket, struct sockaddr * pName, olint_t * pnNameLen);

/** Clear socket in file descriptor set.
 *
 *  @param pSocket [in] The socket to clear.
 *  @param set [in] The file descriptor set.
 *
 *  @return Void.
 */
NETWORKAPI void NETWORKCALL jf_network_clearSocketFromFdSet(
    jf_network_socket_t * pSocket, fd_set * set);

/** Check if the socket is set in file descriptor set.
 *
 *  @param pSocket [in] The socket to check.
 *  @param set [in] The file descriptor set.
 *
 *  @return The status.
 *  @retval TRUE The socket is set in the file descriptor set.
 *  @retval FALSE The socket is not set in the file descriptor set.
 */
NETWORKAPI boolean_t NETWORKCALL jf_network_isSocketSetInFdSet(
    jf_network_socket_t * pSocket, fd_set * set);

/** Set socket to file descriptor set.
 *
 *  @param pSocket [in] The socket to set.
 *  @param set [in] The file descriptor set.
 *
 *  @return Void.
 */
NETWORKAPI void NETWORKCALL jf_network_setSocketToFdSet(
    jf_network_socket_t * pSocket, fd_set * set);

/** Clear file descriptor set.
 *
 *  @param set [in] The file descriptor set to clear.
 *
 *  @return Void.
 */
NETWORKAPI void NETWORKCALL jf_network_clearFdSet(fd_set * set);

/** Get options on socket.
 *
 *  @param pSocket [in] The socket to get option.
 *  @param level [in] The level of the option.
 *  @param optname [in] The option name.
 *  @param pOptval [out] The option value buffer.
 *  @param psOptval [in/out] The option value buffer size as in parameter and actual size as out
 *   parameter.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_getSocketOption(
    jf_network_socket_t * pSocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t * psOptval);

/** Set options on socket.
 *
 *  @param pSocket [in] The socket to set option.
 *  @param level [in] The level of the option.
 *  @param optname [in] The option name.
 *  @param pOptval [in] The option value.
 *  @param sOptval [in] The option value size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_setSocketOption(
    jf_network_socket_t * pSocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t sOptval);

/*  Async socket. */

/** Get options on async socket.
 *
 *  @param pAsocket [in] The async socket to get option.
 *  @param level [in] The level of the option.
 *  @param optname [in] The option name.
 *  @param pOptval [out] The option value.
 *  @param psOptval [out] The option value size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_getAsocketOption(
    jf_network_asocket_t * pAsocket, olint_t level, olint_t optname, void * pOptval,
    olsize_t * psOptval);

/** Set options on async socket.
 *
 *  @param pAsocket [in] The async socket to set option.
 *  @param level [in] The level of the option.
 *  @param optname [in] The option name.
 *  @param pOptval [in] The option value.
 *  @param sOptval [in] The option value size.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createChain(jf_network_chain_t ** ppChain);

/** Destroy the chain.
 *
 *  @param ppChain [in/out] The chain to destory.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_stopChain(jf_network_chain_t * pChain);

/** Wakeup the chain.
 *
 *  @note This will wake up a chain from sleeping by close the fake socket.
 *
 *  @param pChain [in] The chain to stop.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
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
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createAssocket(
    jf_network_chain_t * pChain, jf_network_assocket_t ** ppAssocket,
    jf_network_assocket_create_param_t * pjnacp);

/** Destroy async server socket.
 *
 *  @param ppAssocket [in/out] The async server socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
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
 *
 *  @return Void.
 */
NETWORKAPI void NETWORKCALL jf_network_setTagOfAssocket(
    jf_network_assocket_t * pAssocket, void * pTag);

/** Disconnect the async socket accepted by server socket.
 *
 *  @note
 *  -# Connection is not closed in this function, utimer is started to do the job. Application
 *   should wait for the disconnect event by jf_network_fnAssocketOnDisconnect_t.
 *  -# Since utimer is used for disconnection, if network chain is stopped, the utimer handler can
 *   not be triggerred. In this case, use jf_network_destroyAssocket() instead.
 *  -# The timer handler will clear all the pending send data, callback function
 *   jf_network_fnAssocketOnSendData_t is called to notify upper layer.
 *
 *  @param pAssocket [in] The async server socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_disconnectAssocket(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket);

/** Send data to remote client.
 *
 *  @note
 *  -# The data is cloned and then send to the remote client.
 *
 *  @param pAssocket [in] The async server socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param pu8Buffer [in] The buffer to send.
 *  @param sBuf [in] The length of the buffer to send.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendAssocketData(
    jf_network_assocket_t * pAssocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t sBuf);

/* Async client socket */

/** Create a async client socket.
 *
 *  @param pChain [in] The chain object to add the async client socket.
 *  @param ppAcsocket [out] The async client socket object to be created.
 *  @param pjnacp [in] The parameter for creating the async client socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_createAcsocket(
    jf_network_chain_t * pChain, jf_network_acsocket_t ** ppAcsocket,
    jf_network_acsocket_create_param_t * pjnacp);

/** Destroy async client socket.
 *
 *  @note
 *  -# All the connections are closed immediately. Async client socket will not notify upper layer
 *   for the disconnection.
 *  -# All the pending send data are cleared immediately, callback function
 *   jf_network_fnAcsocketOnSendData_t is called to notify upper layer.
 *
 *  @param ppAcsocket [in/out] The async client socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_destroyAcsocket(jf_network_acsocket_t ** ppAcsocket);

/** Returns the user's tag associated with the async client socket.
 *
 *  @param pAcsocket [in] The async client socket.
 *
 *  @return The user Tag.
 */
NETWORKAPI void * NETWORKCALL jf_network_getTagOfAcsocket(jf_network_acsocket_t * pAcsocket);

/** Sets the user's tag associated with the async client socket.
 *
 *  @param pAcsocket [in] The async client socket to save the tag to.
 *  @param pTag [in] The user's tag.
 *
 *  @return Void.
 */
NETWORKAPI void NETWORKCALL jf_network_setTagOfAcsocket(
    jf_network_acsocket_t * pAcsocket, void * pTag);

/** Connect the async client socket to remote server.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pjiRemote [in] The remote interface to connect to.
 *  @param u16RemotePort [in] The remote port to connect to.
 *  @param pUser [in] User object that will be passed to the callback function.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_connectAcsocketTo(
    jf_network_acsocket_t * pAcsocket, jf_ipaddr_t * pjiRemote, u16 u16RemotePort, void * pUser);

/** Disconnect the async socket in async client socket.
 *
 *  @note
 *  -# Connection is not closed in this function, utimer is started to do the job. Application
 *   should wait for the disconnect event by jf_network_fnAcsocketOnDisconnect_t.
 *  -# Since utimer is used for disconnection, if network chain is stopped, the utimer handler can
 *   not be triggerred. In this case, use jf_network_destroyAcsocket() instead.
 *  -# The timer handler will clear all the pending send data, callback function
 *   jf_network_fnAcsocketOnSendData_t is called to notify upper layer.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_disconnectAcsocket(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket);

/** Send data on the async client socket.
 *
 *  @note
 *  -# The data is cloned and added to queue, then the chain is waked up.
 *  -# When the function returns, it doesn't mean the data is sent successfully.
 *  -# After wakeup, the chain will send the data and notify application with the callback function.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param pu8Buffer [in] The buffer to send.
 *  @param sBuf [in] The length of the buffer to send.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_sendAcsocketData(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t sBuf);

/** Get local interface of the async socket.
 *
 *  @param pAcsocket [in] The async client socket.
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param pjiAddr [in] The local interface.
 *
 *  @return Void.
 */
NETWORKAPI void NETWORKCALL jf_network_getLocalInterfaceOfAcsocket(
    jf_network_acsocket_t * pAcsocket, jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr);

/* Name resolution */

/** Resolve host name to IP.
 *
 *  @param pstrName [in] The host name.
 *  @param ppHostent [out] The resolve result.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
NETWORKAPI u32 NETWORKCALL jf_network_getHostByName(
    const olchar_t * pstrName, struct hostent ** ppHostent);

/* Data transfer */

/** Transfer data to a specified address.
 *
 *  @note
 *  -# Socket is created and connected to the server address, then data is sent and received.
 *   The socket is closed after the session.
 *  -# Reply from server is not necessary.
 *  -# For the reply, it's divided into 2 parts. One is header with fixed length, another is
 *   payload with variable length. A callback function should be provided to get full data size.
 *
 *  @param transfer [in] The data structure containing the detailed information of the transfer. 
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 *  @retval JF_ERR_FAIL_SEND_DATA Failed to send data.
 *  @retval JF_ERR_FAIL_RECV_DATA Failed to receive data.
 */
NETWORKAPI u32 NETWORKCALL jf_network_transferData(jf_network_transfer_data_param_t * transfer);

#endif /*JIUFENG_NETWORK_H */

/*------------------------------------------------------------------------------------------------*/
