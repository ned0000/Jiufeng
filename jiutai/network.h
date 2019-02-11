/**
 *  @file network.h
 *
 *  @brief Header file of network library
 *
 *  @author Min Zhang
 *
 *  @note timeout is in second if not specified
 *
 */

/*--------------------------------------------------------------------------*/

#ifndef JIUFENG_NETWORK_H
#define JIUFENG_NETWORK_H

/* --- standard C lib header files ----------------------------------------- */
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

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "ifmgmt.h"
#include "syncmutex.h"

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

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
#if defined(LINUX)

#elif defined(WINDOWS)

#endif

/* socket */

typedef void  socket_t;
typedef void  asocket_t;
typedef void  assocket_t;
typedef void  acsocket_t;
typedef void  adgram_t;

/* basic chain */

typedef void  basic_chain_t;
typedef void  basic_chain_object_t;

/** Callback function before select()
 *
 *  @param pObject [in] basic chain object
 *  @param readset [in/out] read fd set
 *  @param writeset [in/out] write fd set
 *  @param errorset [in/out] error fd set
 *  @param pu32BlockTime [in/out] timeout in millisecond for select()
 *
 *  @return the error code
 */
typedef u32 (* fnPreSelectChainObject_t)(
    basic_chain_object_t * pObject, fd_set * readset, fd_set * writeset,
    fd_set * errorset, u32 * pu32BlockTime);

/** Callback function after select()
 *
 *  @param pObject [in] basic chain object
 *  @param nReady [in] number of ready fd 
 *  @param readset [in] read fd set
 *  @param writeset [in] write fd set
 *  @param errorset [in] error fd set
 *
 *  @return the error code
 */
typedef u32 (* fnPostSelectChainObject_t)(
    basic_chain_object_t * pObject, olint_t nReady, fd_set * readset,
    fd_set * writeset, fd_set * errorset);

/** Header of basic chain object, MUST be placed at the beginning of the object
 */
typedef struct
{
    fnPreSelectChainObject_t bcoh_fnPreSelect;
    fnPostSelectChainObject_t bcoh_fnPostSelect;
} basic_chain_object_header_t;

/* utimer */

typedef void  utimer_t;

typedef u32 (* fnCallbackOfUtimerItem_t)(void * pData);
typedef u32 (* fnDestroyUtimerItem_t)(void ** ppData);


/* --- functional routines ------------------------------------------------- */

/** Initialize network 
 */
NETWORKAPI u32 NETWORKCALL initNetworkLib(void);

/** Finalize network 
 */
NETWORKAPI u32 NETWORKCALL finiNetworkLib(void);


/**  socket  */

NETWORKAPI u32 NETWORKCALL createSocket(
    olint_t domain, olint_t type, olint_t protocol, socket_t ** ppSocket);

NETWORKAPI u32 NETWORKCALL destroySocket(socket_t ** ppSocket);

/** Allocates a UDP socket for a given interface, choosing a random port number
 *  from 55000 to 65000
 *
 *  @param piaLocal [in] the interface to bind to 
 *  @param pu16Port [in] the port number
 *  @param ppSocket [out] the created UDP socket 
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL createDgramSocket(
    ip_addr_t * piaLocal, u16 * pu16Port, socket_t ** ppSocket);

/** Allocates a TCP socket for a given interface, choosing a random port number
 *  from 50000 to 65000 if port is 0, it will bind the address to socket
 *
 *  @note If the port is 0, we select a random port from the port number range
 *
 *  @param piaLocal [in] the interface to bind to 
 *  @param pu16Port [in/out] the port number to bind to
 *  @param ppSocket [out] the created UDP socket 
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL createStreamSocket(
    ip_addr_t * piaLocal, u16 * pu16Port, socket_t ** ppSocket);

/** Allocate a TCP socket according to address type
 */
NETWORKAPI u32 NETWORKCALL createTypeStreamSocket(
    u8 u8AddrType, socket_t ** ppSocket);

NETWORKAPI u32 NETWORKCALL ioctlSocket(
    socket_t * pSocket, olint_t req, void * pArg);

NETWORKAPI u32 NETWORKCALL setSocketBlock(socket_t * pSocket);

NETWORKAPI u32 NETWORKCALL setSocketNonblock(socket_t * pSocket);

NETWORKAPI u32 NETWORKCALL joinMulticastGroup(
    socket_t * pSocket, ip_addr_t * piaAddr, ip_addr_t * piaMulticaseAddr);

NETWORKAPI u32 NETWORKCALL enableBroadcast(socket_t * pSocket);

/** Try to send all data but only send once, the actual sent size is in psSend
 */
NETWORKAPI u32 NETWORKCALL sSend(
    socket_t * pSocket, void * pBuffer, olsize_t * psSend);

/** Try to send all data but only send once, unless timeout the actual sent size
 *  is in psSend
 */
NETWORKAPI u32 NETWORKCALL sSendWithTimeout(
    socket_t * pSocket, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to send all data with possible several round, until an error occurs,
 *  the actual sent size is in psSend
 */
NETWORKAPI u32 NETWORKCALL sSendn(
    socket_t * pSocket, void * pBuffer, olsize_t * psSend);

/** Try to send all data with possible several round, until an error occurs
 *  or timeout, the actual sent size is in psSend
 */
NETWORKAPI u32 NETWORKCALL sSendnWithTimeout(
    socket_t * pSocket, void * pBuffer, olsize_t * psSend, u32 u32Timeout);

/** Try to recveive all data but only recveive once, the actual received size is
 *  in psRecv
 */
NETWORKAPI u32 NETWORKCALL sRecv(
    socket_t * pSocket, void * pBuffer, olsize_t * psRecv);

/** Try to recveive all data but only recveive once, unless timeout the actual
 *  received size is in psRecv
 */
NETWORKAPI u32 NETWORKCALL sRecvWithTimeout(
    socket_t * pSocket, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

/** Try to recveive all data but only recveive once, unless timeout the actual
 *  received size is in psRecv
 */
NETWORKAPI u32 NETWORKCALL sRecvfromWithTimeout(
    socket_t * pSocket, void * pBuffer, olsize_t * psRecv,
    u32 u32Timeout, ip_addr_t * piaFrom, u16 * pu16Port);

/** Try to recveive all data with possible several round, until an error occurs,
 *  the actual recveived size is in psRecv
 */
NETWORKAPI u32 NETWORKCALL sRecvn(socket_t * pSocket,
    void * pBuffer, olsize_t * psRecv);

/** Try to recveive all data with possible several round, until an error occurs
 *  or timeout, the actual recveived size is in psRecv
 */
NETWORKAPI u32 NETWORKCALL sRecvnWithTimeout(
    socket_t * pSocket, void * pBuffer, olsize_t * psRecv, u32 u32Timeout);

NETWORKAPI u32 NETWORKCALL sConnect(
    socket_t * pSocket, const ip_addr_t * pia, u16 u16Port);

NETWORKAPI u32 NETWORKCALL sConnectWithTimeout(
    socket_t * pSocket, const ip_addr_t * pia, u16 u16Port, u32 u32Timeout);

NETWORKAPI u32 NETWORKCALL sListen(socket_t * pSocket, olint_t backlog);

NETWORKAPI u32 NETWORKCALL sAccept(
    socket_t * pListen, ip_addr_t * pia, u16 * pu16Port, socket_t ** ppSocket);

NETWORKAPI u32 NETWORKCALL sSendto(
    socket_t * pSocket, void * pBuffer, olsize_t * psSend,
    const ip_addr_t * piaTo, u16 u16Port);

NETWORKAPI u32 NETWORKCALL sRecvfrom(
    socket_t * pSocket, void * pBuffer, olsize_t * psRecv, ip_addr_t * piaTo,
    u16 * pu16Port);

NETWORKAPI u32 NETWORKCALL createSocketPair(
    olint_t domain, olint_t type, socket_t * psPair[2]);

NETWORKAPI u32 NETWORKCALL destroySocketPair(socket_t * sPair[2]);

NETWORKAPI u32 NETWORKCALL sSelect(
    fd_set * readfds, fd_set * writefds, fd_set * exceptfds,
    struct timeval * timeout, u32 * pu32Ready);

NETWORKAPI u32 NETWORKCALL getSocketName(
    socket_t * pSocket, struct sockaddr * pName, olint_t * pnNameLen);

NETWORKAPI u32 NETWORKCALL getSocketOpt(
    socket_t * pSocket, olint_t level, olint_t optname, void * optval,
    olsize_t * optlen);

NETWORKAPI u32 NETWORKCALL setSocketOpt(
    socket_t * pSocket, olint_t level, olint_t optname, void * optval,
    olsize_t optlen);

NETWORKAPI void NETWORKCALL clearSocketFromFdSet(
    socket_t * pSocket, fd_set * set);

NETWORKAPI boolean_t NETWORKCALL isSocketSetInFdSet(
    socket_t * pSocket, fd_set * set);

NETWORKAPI void NETWORKCALL setSocketToFdSet(socket_t * pSocket, fd_set * set);

NETWORKAPI void NETWORKCALL clearFdSet(fd_set * set);

/* basic chain */

/** Create a basic chain
 *
 *  @param ppChain [out] the chain to create 
 * 
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL createBasicChain(basic_chain_t ** ppChain);

/** Destroy the basic chain
 *
 *  @param ppChain [in/out] the Chain to destory 
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL destroyBasicChain(basic_chain_t ** ppChain);

/** Add links to the chain
 *
 *  @note All objects added to the chain must extend/implement
 *   basic_chain_object_header_t \n
 *  Eg.  \n
 *  struct object    \n
 *  {    \n
 *      basic_chain_object_header_t header \n
 *      ...;                               \n
 *  }   \n
 *
 *  @param pChain [in] the chain to add the link to 
 *  @param pObject [in] the link to add to the chain
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL appendToBasicChain(
    basic_chain_t * pChain, basic_chain_object_t * pObject);

/** Start a Chain
 *
 *  @note This method will use the current thread. All events and processing
 *   will be done on this thread. This method will not return until
 *   stopBasicChain is called.
 *
 *  @param pChain [in] the chain to start 
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL startBasicChain(basic_chain_t * pChain);

/** Stop a chain, imply the destruction of the chain
 *
 *  @note This will signal the process to shutdown. When the chain cleans itself
 *   up, the thread that is locked on pChain will return.
 *
 *  @param pChain [in] the chain to stop 
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL stopBasicChain(basic_chain_t * pChain);

/** Wakeup the basic chain
 *
 *  @note This will wake up a chain from sleeping by close the fake socket.
 *
 *  @param pChain [in] The chain to stop 
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL wakeupBasicChain(basic_chain_t * pChain);

/* utimer */

/** Add a timed callback with millisecond granularity
 *
 *  @param pUtimer [in] the timer
 *  @param pData [in] the data object to associate with the timed callback 
 *  @param u32Seconds [in] number of seconds for the timed callback 
 *  @param fnCallback [in] the callback function pointer 
 *  @param fnDestroy [in] the abort function pointer, which triggers all 
 *   non-triggered timed callbacks, upon shutdown
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL addUtimerItem(
    utimer_t * pUtimer, void * pData, u32 u32Seconds,
    fnCallbackOfUtimerItem_t fnCallback, fnDestroyUtimerItem_t fnDestroy);

/** Removes timed callback(s) specified by pData from an utimer
 *
 *  @note If there are multiple item pertaining to pData, all of them are
 *   removed
 *  @note Before destroying the utimer item structure, (*fnDestroy)() is called
 *
 *  @param pUtimer [in] the utimer object to remove the callback from 
 *  @param pData [in] the data object to remove 
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL removeUtimerItem(utimer_t * pUtimer, void * pData);

/** Destroy a timer
 *
 *  @note This method never needs to be explicitly called by the developer. It
 *   is called by the chain as a Destroy.
 *
 *  @param ppUtimer [in/out] the utimer object 
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL destroyUtimer(utimer_t ** ppUtimer);

/** Creates an empty utimer 
 *
 *  @note All events are triggered on the thread. Developers must NEVER block
 *   this thread
 *
 *  @param pChain [in] the chain to add the utimer to 
 *  @param ppUtimer [out] the utimer
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL createUtimer(
    basic_chain_t * pChain, utimer_t ** ppUtimer);

/* async socket */

/** memory ower
 */
typedef enum as_mem_owner
{
    AS_MEM_OWNER_ASOCKET = 0, /**< owner is asocket */
    AS_MEM_OWNER_STATIC,      /**< owner is uesr but it's static, asocket can
    use it until the data is sent. user will not recycle the momery until the
    send is ok. */
    AS_MEM_OWNER_USER,        /**< owner is user, asocket can use it in
    asSendData(), if the data is failed to be sent, asocket will duplicate the
    data and send it later. User can recycle the momery after asSendData()
    weather the data is sent or not */
} as_mem_owner_t;

/** The function is to notify upper layer there are data coming
 */
typedef u32 (* fnAsocketOnData_t)(
    asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser, boolean_t * pbPause);

/** The function is to notify upper layer if the connnection is established
 *  if bOK is true, connection is setup, otherwise not, upper layer SHOULD NOT
 *  call asDisconnect to close the connection, asocket will handle it by itself
 */
typedef u32 (* fnAsocketOnConnect_t)(
    asocket_t * pAsocket, boolean_t bOK, void * pUser);

/** The function is to notify upper layer the connnection is going to be closed
 *  the access to the asocket being closed is not recommended.
 *  DO NOT use asDisconnect in this callback function as asocket can handle it
 *  by itself. The u32Status is the reason why the connection is closed
 */
typedef u32 (* fnAsocketOnDisconnect_t)(
    asocket_t * pAsocket, u32 u32Status, void * pUser);

typedef u32 (* fnAsocketOnSendOK_t)(asocket_t * pAsocket, void * pUser);

typedef struct
{
    olsize_t ap_sInitialBuf;
    u32 ap_u32Reserved;
    fnAsocketOnData_t ap_fnOnData;
    fnAsocketOnConnect_t ap_fnOnConnect;
    fnAsocketOnDisconnect_t ap_fnOnDisconnect;
    fnAsocketOnSendOK_t ap_fnOnSendOK;
    /*do not read data*/
    boolean_t ap_bNoRead;
    u8 ap_u8Reserved[15];
    sync_mutex_t * ap_psmLock;
} asocket_param_t;

/** Creates a new asocket object
 *
 *  @param pChain [in] the basic chain object to add the asocket
 *  @param ppAsocket [in/out] the asocket object created
 *  @param pap [in] the parameter for creating the asocket
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL createAsocket(
    basic_chain_t * pChain, asocket_t ** ppAsocket, asocket_param_t * pap);

/** Destroy asocket object
 *
 *  @param ppAsocket [in/out] the asocket object
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL destroyAsocket(asocket_t ** ppAsocket);

/** Clears all the pending data to be sent for an asocket
 *
 *  @param pAsocket [in] the asocket to clear
 */
NETWORKAPI void NETWORKCALL clearPendingSendOfAsocket(asocket_t * pAsocket);

/** Determines if an asocket is utilized
 *
 *  @param pAsocket [in] the asocket to check
 *
 *  @return the status of the asocket
 *  @retval FALSE if utilized
 *  @retval TRUE if free
 */
NETWORKAPI boolean_t NETWORKCALL isAsocketFree(asocket_t * pAsocket);

/** Associates an actual socket with asocket. To associate asocket with an
 *  already connected socket.
 *
 *  @param pAsocket [in] the asocket to associate with
 *  @param pSocket [in] the socket to associate
 *  @param pUser [in] the user
 */
NETWORKAPI u32 NETWORKCALL useSocketForAsocket(
    asocket_t * pAsocket, socket_t * pSocket, void * pUser);

/** Returns the buffer associated with an asocket
 *
 *  @param pAsocket [in] the asocket to obtain the buffer from
 *  @param ppBuffer [out] the buffer
 *  @param psBeginPointer [out] the begin pointer of the buffer
 *  @param psEndPointer [out] The end pointer of the buffer
 */
NETWORKAPI void NETWORKCALL getBufferOfAsocket(
    asocket_t * pAsocket,
    u8 ** ppBuffer, olsize_t * psBeginPointer, olsize_t * psEndPointer);

NETWORKAPI void NETWORKCALL clearBufferOfAsocket(asocket_t * pAsocket);

/** Returns the number of bytes that are pending to be sent
 *
 *  @param pAsocket [in] the asocket to check
 *
 *  @return number of pending bytes
 */
NETWORKAPI olsize_t NETWORKCALL getPendingBytesToSendOfAsocket(
    asocket_t * pAsocket);

/** Returns the total number of bytes that have been sent, since the last reset
 *
 *  @param pAsocket [in] the asocket to check
 *
 *  @return number of bytes sent
 */
NETWORKAPI olsize_t NETWORKCALL getTotalBytesSentOfAsocket(
    asocket_t * pAsocket);

/** Resets the total bytes sent counter
 *
 *  @param pAsocket [in] the asocket to reset
 */
NETWORKAPI void NETWORKCALL resetTotalBytesSentOfAsocket(asocket_t * pAsocket);

/** Returns the Local Interface of a connected socket
 *
 *  @param pAsocket [in] the asocket to query
 *  @param piaAddr [in] the local interface
 */
NETWORKAPI void NETWORKCALL getLocalInterfaceOfAsocket(
    asocket_t * pAsocket, ip_addr_t * piaAddr);

/** Returns the Remote Interface of a connected socket
 *
 *  @param pAsocket [in] the asocket to query
 *  @param piaAddr [out] the remote interface
 */
NETWORKAPI void NETWORKCALL getRemoteInterfaceOfAsocket(
    asocket_t * pAsocket, ip_addr_t * piaAddr);

/** Sets the remote address field. This is utilized by the asocket.
 *
 *  @param pAsocket [in] the asocket to modify
 *  @param piaAddr [in] the remote interface
 */
NETWORKAPI void NETWORKCALL setRemoteAddressOfAsocket(
    asocket_t * pAsocket, ip_addr_t * piaAddr);

/** Returns the user's tag associated with the asocket
 *
 *  @param pAsocket [in] the asocket to query
 *
 *  @return The user Tag
 */
NETWORKAPI void * NETWORKCALL getTagOfAsocket(asocket_t * pAsocket);

/** Sets the user's tag asociated with the asocket
 *
 *  @param pAsocket [in] the asocket to save the tag to
 *  @param pTag [in] the user's tag
 */
NETWORKAPI void NETWORKCALL setTagOfAsocket(
    asocket_t * pAsocket, void * pTag);

/** Disconnects an asocket
 *
 *  @param pAsocket [in] the asocket to disconnect
 */
NETWORKAPI u32 NETWORKCALL asDisconnect(asocket_t * pAsocket);

/** Try to send the data and cache the data if the socket is temporarily not
 *  writable
 *
 *  @param pAsocket [in] the asocket to send data on
 *  @param pu8Buffer [in] the buffer to send
 *  @param sBuf [in] the length of the buffer to send
 *  @param memowner [in] flag indicating memory ownership.
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL asSendData(
    asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t sBuf, as_mem_owner_t memowner);

/** Try to send the data and not return until all data is sent
 */
NETWORKAPI u32 NETWORKCALL asSendn(
    asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBuf);

NETWORKAPI u32 NETWORKCALL asRecvData(
    asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psRecv);

/** Attempts to establish a TCP connection
 *
 *  @param pAsocket [in] the asocket to initiate the connection
 *  @param piaRemote [in] the remote interface to connect to
 *  @param u16RemotePortNumber [in] the remote port to connect to
 *  @param pUser [in] user object that will be passed to other method
 */
NETWORKAPI u32 NETWORKCALL asConnectTo(
    asocket_t * pAsocket, ip_addr_t * piaRemote,
    u16 u16RemotePortNumber, void * pUser);

/** Resumes a paused asocket. Sessions can be paused, such that further data is
 *  not read from the socket until resumed
 *
 *  @param pAsocket [in] the asocket to resume
 */
NETWORKAPI u32 NETWORKCALL resumeAsocket(asocket_t * pAsocket);

NETWORKAPI u32 NETWORKCALL getAsocketOpt(
    asocket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t * optlen);

NETWORKAPI u32 NETWORKCALL setAsocketOpt(
    asocket_t * pAsocket,
    olint_t level, olint_t optname, void * optval, olsize_t optlen);

/* async server socket */

/** The function is to notify upper layer there are incoming data. The pUser is
 *  set by fnAssocketOnConnect_t when a new incoming connection is accepted
 */
typedef u32 (* fnAssocketOnData_t)(
    assocket_t * pAssocket, asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser,
    boolean_t * pbPause);

/** The function is to notify upper layer there are new connection
 */
typedef u32 (* fnAssocketOnConnect_t)(
    assocket_t * pAssocket, asocket_t * pAsocket, void ** ppUser);

/** The function is to notify upper layer a connection is closed.
 *
 *  @note The access to the asocket being closed is not recommended.
 *  @note DO NOT use asDisconnect in this callback function as asocket can
 *   handle it by itself.
 *
 *  @param u32Status [in] the reason why the connection is closed
 */
typedef u32 (* fnAssocketOnDisconnect_t)(
    assocket_t * pAssocket, asocket_t * pAsocket, u32 u32Status, void * pUser);

/** The function is to notify upper layer the data is sent to peer successfully
 */
typedef u32 (* fnAssocketOnSendOK_t)(
    assocket_t * pAssocket, asocket_t * pAsocket, void * pUser);

typedef struct
{
    /**The initial size of the receive buffer*/
    olsize_t ap_sInitialBuf;
    /**The max number of simultaneous connections that will be allowed*/
    u32 ap_u32MaxConn;
    boolean_t ap_bNoRead;
    u8 ap_u8Reserved[1];
    /**The port number to bind to. 0 will select a random port*/
    u16 ap_u16PortNumber;
    ip_addr_t ap_iaAddr;
    /**Function that triggers when a connection is established*/
    fnAssocketOnConnect_t ap_fnOnConnect;
    /**Function that triggers when a connection is closed*/
    fnAssocketOnDisconnect_t ap_fnOnDisconnect;
    /**Function that triggers when data is coming*/
    fnAssocketOnData_t ap_fnOnData;
    /**Function that triggers when pending sends are complete*/
    fnAssocketOnSendOK_t ap_fnOnSendOK;
    u32 ap_u32Reserved2[4];
} assocket_param_t;

/** Create async server socket.
 *
 *  @param: pChain: The chain to add this module to
 */
NETWORKAPI u32 NETWORKCALL createAssocket(
    basic_chain_t * pChain, assocket_t ** ppAssocket, assocket_param_t * pap);

/** Destroy async server socket.
 *
 */
NETWORKAPI u32 NETWORKCALL destroyAssocket(assocket_t ** ppAssocket);

/** Returns the port number the server is bound to
 *
 *  @param pAssocket [in] the assocket to query
 *
 *  @return the listening port number
 */
NETWORKAPI u16 NETWORKCALL getPortNumberOfAssocket(assocket_t * pAssocket);

/** Returns the user's Tag associated with the assocket
 *
 *  @param pAssocket [in] the assocket to query
 *
 *  @return the user tag
 */
NETWORKAPI void * NETWORKCALL getTagOfAssocket(assocket_t * pAssocket);

/** Sets the user's tag associated with the assocket
 *
 *  @param pAssocket [in] the assocket to save the tag to
 *  @param pTag [in] the tag
 */
NETWORKAPI void NETWORKCALL setTagOfAssocket(
    assocket_t * pAssocket, void * pTag);

NETWORKAPI boolean_t NETWORKCALL isAssocketFree(
    assocket_t * pAssocket, asocket_t * pAsocket);

NETWORKAPI u32 NETWORKCALL assDisconnect(
    assocket_t * pAssocket, asocket_t * pAsocket);

/** Resumes a paused asocket. Sessions can be paused, such that further data is
 *  not read from the socket until resumed
 *
 *  @param pAssocket [in] the assocket object
 *  @param pAsocket [in] the asocket to resume
 */
NETWORKAPI u32 NETWORKCALL resumeAssocket(
    assocket_t * pAssocket, asocket_t * pAsocket);

NETWORKAPI u32 NETWORKCALL assRecvData(
    assocket_t * pAssocket, asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psRecv);

NETWORKAPI u32 NETWORKCALL getAssocketOpt(
    assocket_t * pAssocket, asocket_t * pAsocket, olint_t level,
    olint_t optname, void * optval, olsize_t * optlen);

NETWORKAPI u32 NETWORKCALL setAssocketOpt(
    assocket_t * pAssocket, socket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t optlen);

NETWORKAPI u32 NETWORKCALL assSendData(
    assocket_t * pAssocket, asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf,
    as_mem_owner_t memowner);

NETWORKAPI u32 NETWORKCALL assSendn(
    assocket_t * pAssocket, asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBuf);

#define getPendingBytesToSendOfAssocket(pAssocket, pAsocket) \
        getPendingBytesToSendOfAsocket(pAsocket)

#define getTotalBytesSentOfAssocket(pAssocket, pAsocket) \
        getTotalBytesSentOfAsocket(pAsocket)

#define resetTotalBytesSentOfAssocket(pAssocket, pAsocket) \
        resetTotalBytesSentOfAsocket(pAsocket)


/** async client socket
 */

/** The function is to notify upper layer there are incoming data
 */
typedef u32 (* fnAcsocketOnData_t)(
    acsocket_t * pAcsocket, asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser,
    boolean_t * pbPause);

/** The function is to notify upper layer there are new connection
 */
typedef u32 (* fnAcsocketOnConnect_t)(
    acsocket_t * pAcsocket, asocket_t * pAsocket, boolean_t bOK, void * pUser);

/** The function is to notify upper layer a connection is closed.
 *
 *  @note The access to the asocket being closed is not recommended.
 *  @note DO NOT use asDisconnect in this callback function as asocket can
 *   handle it by itself.
 *
 *  @param u32Status [in] the reason why the connection is closed
 */
typedef u32 (* fnAcsocketOnDisconnect_t)(
    acsocket_t * pAcsocket, asocket_t * pAsocket, u32 u32Status, void * pUser);

/** The function is to notify upper layer the data is sent to peer successfully
 */
typedef u32 (* fnAcsocketOnSendOK_t)(
    acsocket_t * pAcsocket, asocket_t * pAsocket, void * pUser);

typedef struct
{
    /**The initial size of the receive buffer*/
    olsize_t ap_sInitialBuf;
    /**The max number of simultaneous connections that will be allowed*/
    u32 ap_u32MaxConn;
    boolean_t ap_bNoRead;
    u8 ap_u8Reserved[7];
    /**Function that triggers when a connection is established*/
    fnAcsocketOnConnect_t ap_fnOnConnect;
    /**Function that triggers when a connection is closed*/
    fnAcsocketOnDisconnect_t ap_fnOnDisconnect;
    fnAcsocketOnData_t ap_fnOnData;
    /**Function that triggers when pending sends are complete*/
    fnAcsocketOnSendOK_t ap_fnOnSendOK;
} acsocket_param_t;

NETWORKAPI u32 NETWORKCALL createAcsocket(
    basic_chain_t * pChain, acsocket_t ** ppAcsocket, acsocket_param_t * pap);

NETWORKAPI u32 NETWORKCALL destroyAcsocket(acsocket_t ** ppAcsocket);

/** Returns the user's tag associated with the acsocket
 *
 *  @param: pAcsocket [in] the acsocket to query
 *
 *  @return the user Tag
 */
NETWORKAPI void * NETWORKCALL getTagOfAcsocket(acsocket_t * pAcsocket);

/** Sets the user's tag associated with the acsocket
 *
 *  @param pAcsocket [in] the acsocket to save the tag to
 *  @param pTag [in] the user's tag
 */
NETWORKAPI void NETWORKCALL setTagOfAcsocket(
    acsocket_t * pAcsocket, void * pTag);

NETWORKAPI boolean_t NETWORKCALL isAcsocketFree(
    acsocket_t * pAcsocket, asocket_t * pAsocket);

NETWORKAPI u32 NETWORKCALL acsConnectTo(
    acsocket_t * pAcsocket,
    ip_addr_t * piaRemote, u16 u16RemotePortNumber, void * pUser);

NETWORKAPI u32 NETWORKCALL acsDisconnect(acsocket_t * pAcsocket,
    asocket_t * pAsocket);

/** Resumes a paused asocket. Sessions can be paused, such that further data is
 *  not read from the socket until resumed
 *
 *  @param pAcsocket [in] the client socket
 *  @param pAsocket [in] the asocket to resume
 */
NETWORKAPI u32 NETWORKCALL resumeAcsocket(
    acsocket_t * pAcsocket, asocket_t * pAsocket);

NETWORKAPI u32 NETWORKCALL acsRecvData(
    acsocket_t * pAcsocket, asocket_t * pAsocket,
    u8 * pu8Buffer, olsize_t * psRecv);

NETWORKAPI u32 NETWORKCALL getAcsocketOpt(
    acsocket_t * pAcsocket, asocket_t * pAsocket, olint_t level,
    olint_t optname, void * optval, olsize_t * optlen);

NETWORKAPI u32 NETWORKCALL setAcsocketOpt(
    acsocket_t * pAcsocket, socket_t * pAsocket, olint_t level, olint_t optname,
    void * optval, olsize_t optlen);

NETWORKAPI u32 NETWORKCALL acsSendData(
    acsocket_t * pAcsocket, asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf,
    as_mem_owner_t memowner);

NETWORKAPI u32 NETWORKCALL acsSendn(
    acsocket_t * pAcsocket, asocket_t * pAsocket, u8 * pu8Buffer,
    olsize_t * psBuf);

#define getPendingBytesToSendOfAcsocket(pAcsocket, pAsocket) \
        getPendingBytesToSendOfAsocket(pAsocket)

#define getTotalBytesSentOfAcsocket(pAcsocket, pAsocket) \
        getTotalBytesSentOfAsocket(pAsocket)

#define resetTotalBytesSentOfAcsocket(pAcsocket, pAsocket) \
        resetTotalBytesSentOfAsocket(pAsocket)

/** async datagram socket
 */

typedef enum ad_mem_owner
{
    AD_MEM_OWNER_ADGRAM = 0, /*owner is adgram*/
    AD_MEM_OWNER_STATIC,     /*owner is uesr but it's static, adgram can use it
                               until the data is sent*/
    AD_MEM_OWNER_USER,    /*owner is user, asocket can use it in adSendData()*/
} ad_mem_owner_t;

typedef u32 (* fnAdgramOnData_t)(
    adgram_t * pAdgram, u8 * pu8Buffer,
    olsize_t * psBeginPointer, olsize_t sEndPointer, void * pUser,
    boolean_t * pbPause, ip_addr_t * piaRemote, u16 u16Port);

typedef u32 (* fnAdgramOnSendOK_t)(adgram_t * pAdgram, void * pUser);

typedef struct
{
    olsize_t ap_sInitialBuf;
    u32 ap_u32Reserved;
    fnAdgramOnData_t ap_fnOnData;
    fnAdgramOnSendOK_t ap_fnOnSendOK;
    u8 ap_u8Reserved[16];
    void * ap_pUser;
} adgram_param_t;

/** Creates a new adgram object
 *
 *  @param pChain [in] the basic chain object to add the adgram object
 *  @param ppAdgram [out] the adgram object created
 *  @param pap [in] the parameter for creating the adgram
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL createAdgram(
    basic_chain_t * pChain, adgram_t ** ppAdgram, adgram_param_t * pap);

NETWORKAPI u32 NETWORKCALL destroyAdgram(adgram_t ** ppAdgram);

/** Clears all the pending data to be sent
 *
 *  @param pAdgram [in] the adgram to clear
 */
NETWORKAPI u32 NETWORKCALL clearPendingSendOfAdgram(adgram_t *pAdgram);

/** Determines if an adgram is utilized
 *
 *  @param pAdgram [in] the asocket to check
 *
 *  @return the status of adgram
 *  @retval FALSE if utilized
 *  @retval TRUE if free
 */
NETWORKAPI boolean_t NETWORKCALL isAdgramFree(adgram_t *pAdgram);

/** Associates an actual socket with adgram. To associate adgram with an already
 *  connected socket.
 *
 *  @param pAdgram [in] the adgram to associate with
 *  @param pSocket [in] the socket to associate
 *  @param pUser [in] the user 
 */
NETWORKAPI u32 NETWORKCALL useSocketForAdgram(
    adgram_t * pAdgram, socket_t * pSocket, void * pUser);

NETWORKAPI u32 NETWORKCALL freeSocketForAdgram(adgram_t * pAdgram);

/** Returns the buffer associated with an adgram
 *
 *  @param pAdgram [in] the adgram to obtain the buffer from
 *  @param ppBuffer [out] the buffer
 *  @param psBeginPointer [out] the begin pointer of the buffer
 *  @param psEndPointer [out]: the end pointer of the buffer
 */
NETWORKAPI void NETWORKCALL getBufferOfAdgram(
    adgram_t *pAdgram, u8 ** ppBuffer,
    olsize_t * psBeginPointer, olsize_t * psEndPointer);

NETWORKAPI void NETWORKCALL clearBufferOfAdgram(adgram_t * pAdgram);

/** Returns the number of bytes that are pending to be sent
 *
 *  @param pAdgram [in] the asocket to check
 *
 *  @return number of pending bytes
 */
NETWORKAPI olsize_t NETWORKCALL getPendingBytesToSendOfAdgram(
    adgram_t * pAdgram);

/** Returns the total number of bytes that have been sent, since the last reset
 *
 *  @param pAdgram [in] the adgram to check
 *
 *  @return number of bytes sent
 */
NETWORKAPI olsize_t NETWORKCALL getTotalBytesSentOfAdgram(adgram_t * pAdgram);

/** Resets the total bytes sent counter
 *
 *  @param pAdgram [in] the adgram to reset
 */
NETWORKAPI void NETWORKCALL resetTotalBytesSentOfAdgram(adgram_t * pAdgram);

/** Sends data on an adgram
 *
 *  @param pAdgram [in] the adgram object to send data on
 *  @param pu8Buffer [in] the buffer to send
 *  @param sBuf [in] the length of the buffer to send
 *  @param memowner [in] flag indicating memory ownership.
 *  @param piaRemote [in] the remote host to send data to
 *  @param u16Port [in] the remote port
 *
 *  @return the error code
 */
NETWORKAPI u32 NETWORKCALL adSendData(
    adgram_t * pAdgram, u8 * pu8Buffer, olsize_t sBuf, ad_mem_owner_t memowner,
    ip_addr_t * piaRemote, u16 u16Port);

NETWORKAPI u32 NETWORKCALL adJoinMulticastGroup(
    adgram_t * pAdgram, ip_addr_t * piaAddr, ip_addr_t * piaMulticaseAddr);

NETWORKAPI u32 NETWORKCALL adEnableBroadcast(adgram_t * pAdgram);

/** Resumes a paused session
 *
 *  @param pAdgram [in] the asocket to resume
 */
NETWORKAPI u32 NETWORKCALL resumeAdgram(adgram_t * pAdgram);

/** resolve
 */

NETWORKAPI u32 NETWORKCALL getHostByName(
    const olchar_t * pstrName, struct hostent ** ppHostent);

#endif /*JIUFENG_NETWORK_H */

/*--------------------------------------------------------------------------*/

