/**
 *  @file adgram.h
 *
 *  @brief header file for adgram
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef NETWORK_ADGRAM_H
#define NETWORK_ADGRAM_H

/* --- standard C lib header files -------------------------------------------------------------- */

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_network.h"

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** async datagram socket
 */

/**Notify the upper layer data is received*/
typedef u32 (* fnAdgramOnData_t)(
    jf_network_adgram_t * pAdgram, u8 * pu8Buffer, olsize_t * psBeginPointer,
    olsize_t sEndPointer, void * pUser, jf_ipaddr_t * pjiRemote, u16 u16Port);

/**Notify the upper layer the data send result*/
typedef u32 (* fnAdgramOnSendData_t)(
    jf_network_adgram_t * pAdgram, u32 u32Status, u8 * pu8Buffer, olsize_t sBuf, void * pUser);

typedef struct
{
    olsize_t acp_sInitialBuf;
    u32 acp_u32Reserved;
    fnAdgramOnData_t acp_fnOnData;
    fnAdgramOnSendData_t acp_fnOnSendData;
    u8 acp_u8Reserved[16];
    void * acp_pUser;
    olchar_t * acp_pstrName;
} adgram_create_param_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** async datagram socket
 */

/** Creates a new async dgram object
 *
 *  @param pChain [in] the chain object to add the adgram object
 *  @param ppAdgram [out] the adgram object created
 *  @param pacp [in] the parameter for creating the adgram
 *
 *  @return the error code
 */
u32 createAdgram(
    jf_network_chain_t * pChain, jf_network_adgram_t ** ppAdgram, adgram_create_param_t * pacp);

/** Destroy async dgram object
 *
 *  @param ppAdgram [in/out] the async dgram object
 *
 *  @return the error code
 */
u32 destroyAdgram(jf_network_adgram_t ** ppAdgram);

/** Determines if an adgram is utilized
 *
 *  @param pAdgram [in] the asocket to check
 *
 *  @return the status of adgram
 *  @retval FALSE if utilized
 *  @retval TRUE if free
 */
boolean_t isAdgramFree(jf_network_adgram_t *pAdgram);

/** Associates an actual socket with adgram. To associate adgram with an already
 *  connected socket.
 *
 *  @param pAdgram [in] the adgram to associate with
 *  @param pSocket [in] the socket to associate
 *  @param pUser [in] the user 
 */
u32 useSocketForAdgram(
    jf_network_adgram_t * pAdgram, jf_network_socket_t * pSocket, void * pUser);

/** Returns the number of data that have been sent
 *
 *  @param pAdgram [in] the asocket to check
 *
 *  @return number of pending bytes
 */
olsize_t getTotalDataSentOfAdgram(jf_network_adgram_t * pAdgram);

/** Returns the total number of bytes that have been sent, since the last reset
 *
 *  @param pAdgram [in] the adgram to check
 *
 *  @return number of bytes sent
 */
olsize_t getTotalBytesSentOfAdgram(jf_network_adgram_t * pAdgram);

/** Sends data on an adgram
 *
 *  @param pAdgram [in] the adgram object to send data on
 *  @param pu8Buffer [in] the buffer to send
 *  @param sBuf [in] the length of the buffer to send
 *  @param pjiRemote [in] the remote host to send data to
 *  @param u16Port [in] the remote port
 *
 *  @return the error code
 */
u32 sendAdgramData(
    jf_network_adgram_t * pAdgram, u8 * pu8Buffer, olsize_t sBuf,
    jf_ipaddr_t * pjiRemote, u16 u16Port);

#endif /*NETWORK_ADGRAM_H*/

/*------------------------------------------------------------------------------------------------*/


