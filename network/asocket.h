/**
 *  @file asocket.h
 *
 *  @brief header file for asocket
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef NETWORK_ASOCKET_H
#define NETWORK_ASOCKET_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_network.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** async socket
 */

/** The function is to notify upper layer there are data coming
 */
typedef u32 (* fnAsocketOnData_t)(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser);

/** The function is to notify upper layer if the connnection is established
 *  if bOK is true, connection is setup, otherwise not, upper layer SHOULD NOT
 *  call asDisconnect to close the connection, asocket will handle it by itself
 */
typedef u32 (* fnAsocketOnConnect_t)(
    jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser);

/** The function is to notify upper layer the connnection is going to be closed
 *  the access to the asocket being closed is not recommended.
 *  DO NOT use asDisconnect in this callback function as asocket can handle it
 *  by itself. The u32Status is the reason why the connection is closed
 */
typedef u32 (* fnAsocketOnDisconnect_t)(
    jf_network_asocket_t * pAsocket, u32 u32Status, void * pUser);

/** The function is to notify upper layer if the data is successfully sent or not
 */
typedef u32 (* fnAsocketOnSendData_t)(
    jf_network_asocket_t * pAsocket, u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser);

/** The parameter for creating async socket.
 */
typedef struct
{
    /**Initial buffer size.*/
    olsize_t acp_sInitialBuf;
    u32 acp_u32Reserved;
    /**Callback function for incoming data.*/
    fnAsocketOnData_t acp_fnOnData;
    /**Callback function for connect event.*/
    fnAsocketOnConnect_t acp_fnOnConnect;
    /**Callback function for disconnect event.*/
    fnAsocketOnDisconnect_t acp_fnOnDisconnect;
    /**Callback function for send data.*/
    fnAsocketOnSendData_t acp_fnOnSendData;
    /*Name of the async socket.*/
    olchar_t * acp_pstrName;
    u8 jnacp_u8Reserved[16];
} asocket_create_param_t;


/* --- functional routines ---------------------------------------------------------------------- */

/** Create a new async socket object.
 *
 *  @param pChain [in] The chain object to add the async socket.
 *  @param ppAsocket [in/out] The async socket object created.
 *  @param pacp [in] The parameter for creating the async socket.
 *
 *  @return The error code.
 */
u32 createAsocket(
    jf_network_chain_t * pChain, jf_network_asocket_t ** ppAsocket, asocket_create_param_t * pacp);

/** Destroy asocket object.
 *
 *  @note
 *  -# Connection is closed immediately. Asocket will not notify upper layer for the disconnection.
 *  -# All the pending send data are cleared immediately, callback function fnAsocketOnSendData_t is
 *   called to notify upper layer.
 *
 *  @param ppAsocket [in/out] The async socket object.
 *
 *  @return The error code.
 */
u32 destroyAsocket(jf_network_asocket_t ** ppAsocket);

/** Determine if an async socket is utilized.
 *
 *  @param pAsocket [in] The async socket to check.
 *
 *  @return The status of the async socket.
 *  @retval FALSE If async socket is utilized.
 *  @retval TRUE If async socket is free.
 */
boolean_t isAsocketFree(jf_network_asocket_t * pAsocket);

/** Return the number of bytes that are pending to be sent.
 *
 *  @param pAsocket [in] The async socket to check.
 *
 *  @return Number of pending bytes.
 */
olsize_t getTotalSendDataOfAsocket(jf_network_asocket_t * pAsocket);

/** Return the total number of bytes that have been sent, since the last reset.
 *
 *  @param pAsocket [in] The async socket to check.
 *
 *  @return Number of bytes sent.
 */
olsize_t getTotalBytesSentOfAsocket(jf_network_asocket_t * pAsocket);

/** Return the Local Interface of a connected socket.
 *
 *  @param pAsocket [in] The async socket representing the connection.
 *  @param pjiAddr [in] The local interface.
 *
 *  @return Void.
 */
void getLocalInterfaceOfAsocket(jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr);

/** Return the Remote Interface of a connected socket.
 *
 *  @param pAsocket [in] The async socket to query.
 *  @param pjiAddr [out] The remote interface.
 *
 *  @return Void.
 */
void getRemoteInterfaceOfAsocket(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr);

/** Return the user's tag associated with the async socket.
 *
 *  @param pAsocket [in] The async socket to query.
 *
 *  @return The user Tag
 */
void * getTagOfAsocket(jf_network_asocket_t * pAsocket);

/** Set the user's tag asociated with the async socket.
 *
 *  @param pAsocket [in] The asocket to save the tag to.
 *  @param pTag [in] The user's tag.
 *
 *  @return Void.
 */
void setTagOfAsocket(jf_network_asocket_t * pAsocket, void * pTag);

/** Return the index associated with the async socket.
 *
 *  @param pAsocket [in] The async socket to query.
 *
 *  @return The index.
 */
u32 getIndexOfAsocket(jf_network_asocket_t * pAsocket);

/** Set the index asociated with the async socket.
 *
 *  @param pAsocket [in] The async socket to save the index to.
 *  @param u32Index [in] The index.
 *
 *  @return Void.
 */
void setIndexOfAsocket(jf_network_asocket_t * pAsocket, u32 u32Index);

/** Disconnect an async socket.
 *
 *  @note
 *  -# Connection is not closed in this function, a utimer is started to do the job. Application
 *   should wait for the disconnect event by fnAsocketOnDisconnect_t for the final result.
 *  -# Since utimer is used for disconnection, if network chain is stopped, the utimer handler can
 *   not be triggerred. In this case, use jf_network_destroyAcsocket() instead.
 *  -# The timer handler will clear all the pending send data, callback function fnAsocketOnSendData_t
 *   is called to notify upper layer.
 *
 *  @param pAsocket [in] The asocket to disconnect.
 *
 *  @return The error code.
 */
u32 disconnectAsocket(jf_network_asocket_t * pAsocket);

/** Send data to remote server.
 *
 *  @note
 *  -# The data is cloned and then send to the remote server.
 *
 *  @param pAsocket [in] The asocket to send data on.
 *  @param pu8Buffer [in] The buffer to send.
 *  @param sBuf [in] The length of the buffer to send.
 *
 *  @return The error code.
 */
u32 sendAsocketData(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf);

/** Send static data to remote server.
 *
 *  @note
 *  -# The data is not cloned and will used in async socket. Application should not touch it until
 *   fnAsocketOnSendData_t is called for the successful transimission.
 *
 *  @param pAsocket [in] The asocket to send data on.
 *  @param pu8Buffer [in] The buffer to send.
 *  @param sBuf [in] The length of the buffer to send.
 *
 *  @return The error code.
 */
u32 sendAsocketStaticData(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf);

/** Attempt to establish a TCP connection.
 *
 *  @param pAsocket [in] The asocket to initiate the connection.
 *  @param pjiRemote [in] The remote interface to connect to.
 *  @param u16RemotePortNumber [in] The remote port to connect to.
 *  @param pUser [in] User object that will be passed to other method.
 *
 *  @return The error code.
 */
u32 connectAsocketTo(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiRemote,
    u16 u16RemotePortNumber, void * pUser);

/** Async server socket call this function to set the client socket.
 *
 *  @note
 *  -# No lock is required as the function is called in internal chain.
 *
 *  @param pAsocket [in] The free async socket.
 *  @param pSocket [in] The socket representing a connection.
 *  @param pjiAddr [in] The remote address.
 *  @param u16Port [in] The remote port.
 *  @param pUser [in] User object that will be passed to other method.
 *
 *  @return The error code.
 */
u32 useSocketForAsocket(
    jf_network_asocket_t * pAsocket, jf_network_socket_t * pSocket,
    jf_ipaddr_t * pjiAddr, u16 u16Port, void * pUser);


#endif /*NETWORK_ASOCKET_H*/

/*------------------------------------------------------------------------------------------------*/


