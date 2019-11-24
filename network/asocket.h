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

typedef struct
{
    olsize_t acp_sInitialBuf;
    u32 acp_u32Reserved;
    fnAsocketOnData_t acp_fnOnData;
    fnAsocketOnConnect_t acp_fnOnConnect;
    fnAsocketOnDisconnect_t acp_fnOnDisconnect;
    fnAsocketOnSendData_t acp_fnOnSendData;
    olchar_t * acp_pstrName;
    u8 jnacp_u8Reserved[16];
} asocket_create_param_t;


/* --- functional routines ---------------------------------------------------------------------- */

/** async socket
 */

/** Creates a new asocket object
 *
 *  @param pChain [in] the chain object to add the asocket
 *  @param ppAsocket [in/out] the asocket object created
 *  @param pacp [in] the parameter for creating the asocket
 *
 *  @return the error code
 */
u32 createAsocket(
    jf_network_chain_t * pChain, jf_network_asocket_t ** ppAsocket, asocket_create_param_t * pacp);

/** Destroy asocket object
 *
 *  @param ppAsocket [in/out] the asocket object
 *
 *  @return the error code
 */
u32 destroyAsocket(jf_network_asocket_t ** ppAsocket);

/** Determines if an asocket is utilized
 *
 *  @param pAsocket [in] the asocket to check
 *
 *  @return the status of the asocket
 *  @retval FALSE if utilized
 *  @retval TRUE if free
 */
boolean_t isAsocketFree(jf_network_asocket_t * pAsocket);

/** Returns the number of bytes that are pending to be sent
 *
 *  @param pAsocket [in] the asocket to check
 *
 *  @return number of pending bytes
 */
olsize_t getTotalSendDataOfAsocket(jf_network_asocket_t * pAsocket);

/** Returns the total number of bytes that have been sent, since the last reset
 *
 *  @param pAsocket [in] the asocket to check
 *
 *  @return number of bytes sent
 */
olsize_t getTotalBytesSentOfAsocket(jf_network_asocket_t * pAsocket);

/** Returns the Local Interface of a connected socket
 *
 *  @param pAsocket [in] the asocket to query
 *  @param pjiAddr [in] the local interface
 */
void getLocalInterfaceOfAsocket(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr);

/** Returns the Remote Interface of a connected socket
 *
 *  @param pAsocket [in] the asocket to query
 *  @param pjiAddr [out] the remote interface
 */
void getRemoteInterfaceOfAsocket(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiAddr);

/** Returns the user's tag associated with the asocket
 *
 *  @param pAsocket [in] the asocket to query
 *
 *  @return The user Tag
 */
void * getTagOfAsocket(jf_network_asocket_t * pAsocket);

/** Sets the user's tag asociated with the asocket
 *
 *  @param pAsocket [in] the asocket to save the tag to
 *  @param pTag [in] the user's tag
 */
void setTagOfAsocket(jf_network_asocket_t * pAsocket, void * pTag);

/** Disconnects an asocket
 *
 *  @param pAsocket [in] the asocket to disconnect
 */
u32 disconnectAsocket(jf_network_asocket_t * pAsocket);

/** Try to send the data and cache the data if the socket is temporarily not
 *  writable
 *
 *  @param pAsocket [in] the asocket to send data on
 *  @param pu8Buffer [in] the buffer to send
 *  @param sBuf [in] the length of the buffer to send
 *
 *  @return the error code
 */
u32 sendAsocketData(
    jf_network_asocket_t * pAsocket, u8 * pu8Buffer, olsize_t sBuf);

/** Attempts to establish a TCP connection
 *
 *  @param pAsocket [in] the asocket to initiate the connection
 *  @param pjiRemote [in] the remote interface to connect to
 *  @param u16RemotePortNumber [in] the remote port to connect to
 *  @param pUser [in] user object that will be passed to other method
 */
u32 connectAsocketTo(
    jf_network_asocket_t * pAsocket, jf_ipaddr_t * pjiRemote,
    u16 u16RemotePortNumber, void * pUser);

/** Async server socket call this function to set the client socket
 *
 *  @note No lock is required as the function is called in internal chain
 */
u32 useSocketForAsocket(
    jf_network_asocket_t * pAsocket, jf_network_socket_t * pSocket,
    jf_ipaddr_t * pjiAddr, u16 u16Port, void * pUser);


#endif /*NETWORK_ASOCKET_H*/

/*------------------------------------------------------------------------------------------------*/


