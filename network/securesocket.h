/**
 *  @file securesocket.h
 *
 *  @brief secure socket implementation file
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef NETWORK_SECURESOCKET_H
#define NETWORK_SECURESOCKET_H

/* --- standard C lib header files ----------------------------------------- */
#if defined(LINUX)
    #include <openssl/crypto.h>
    #include <openssl/x509.h>
    #include <openssl/pem.h>
    #include <openssl/ssl.h>
    #include <openssl/err.h>
#elif defined(WINDOWS)


#endif

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"

/* --- constant definitions ------------------------------------------------ */

/* --- data structures ----------------------------------------------------- */
typedef struct
{
    u32 ssp_u32Reserved[8];
} secure_socket_param_t;

/* --- functional routines ------------------------------------------------- */

/** The SSL verification level.*/
typedef enum ssl_opts_t 
{ 
    VERIFY_PEER = SSL_VERIFY_PEER, 
    VERIFY_NONE = SSL_VERIFY_NONE 
} ssl_opts_t;

/**
 * ssl_init() initializes the ssl_socket_t and must
 * be called once for each ssl_socket_t object before the other
 * functions can be used. When the ssl_socket_t object is no longer
 * needed it should be de-initialized with ssl_cleanup() to free
 * unneeded resources.
 *
 * @param ssl The ssl_socket_t object.
 */
u32 createSecureSocket(olint_t domain, olint_t type, olint_t protocol,
    socket_t ** pSocket);

/**
 * ssl_cleanup() releases unneeded resources and must be called when
 * the ssl_socket_t object is no longer needed. ssl must be initialized
 * by ssl_init() before, otherwise random errors may occur.
 *
 * @param ssl The ssl_socket_t object.
 */
u32 destroySecureSocket(socket_t ** ppSocket);

/**
 * ssl_read() reads at most bufsiz bytes from the socket and 
 * returns the number of bytes that were actually read.
 *
 * @param ssl     The ssl_socket_t object.
 * @param buf     The buffer that is to be filled.
 * @param bufsiz  The number of bytes that should be read at most.
 *
 * @returns The number of bytes actually read or -1 if an error
 * occured.
 */
u32 ssRecv(socket_t * pSocket, void * pBuffer, u32 * pu32Count);

/**
 * ssl_write() writes at most bufsiz bytes of buf to the socket and 
 * returns the number of bytes that were actually written to the socket.
 *
 * @param ssl     The ssl_socket_t object.
 * @param buf     The buffer that should be written to the socket.
 * @param bufsiz  The number of bytes that should be written to the socket.
 *
 * @returns The number of bytes actually read or -1 if an error
 * occured.
 */
u32 ssSend(socket_t * pSocket, void * pBuffer, u32 * pu32Count);

/**
 * ssl_negotiate() starts the SSL handshake.
 *
 * @param ssl     The ssl_socket_t object.
 *
 * @returns -1 on an error, otherwise 0.
 */
u32 ssNegotiate(socket_t * pSocket);

/**
 * Set some SSL specific options.
 *
 * @param ssl      The ssl_socket_t object.
 * @param opts     The SSL verification level.
 * @param ca_file  A file that contains PEM certificates. Must
 *                 be "" if opts == VERIFY_NONE.
 *                 The file  can  contain several CA certificates 
 *                 identified by
 *                 
 *                 -----BEGIN CERTIFICATE-----
 *                 
 *                 ... [CA certificate in base64 encoding] ...
 *                 
 *                 -----END CERTIFICATE-----
 *                 
 *                 sequences.  Before,  between, and after the 
 *                 certificates text is allowed which
 *                 can be used e.g. for descriptions of the 
 *                 certificates.
 *                 
 *                 Take a look in the openssl documentation to 
 *                 get more infos on that topic.
 * @param ca_dir   A directory that contains PEM certificates. Must
 *                 be "" if opts == VERIFY_NONE.
 *                 The files each contain one CA certificate.  The files 
 *                 are looked up by the CA subject name hash value, which 
 *                 must hence be available.  If more than one CA certificate 
 *                 with the same name hash value exist, the extension must be 
 *                 different (e.g. 9d66eef0.0, 9d66eef0.1 etc).  The search 
 *                 is performed in the ordering of the extension number, 
 *                 regardless of other properties of the certificates.  
 *                 Use the c_rehash utility to create the necessary links.  
 *                 
 *                 Take a look in the openssl documentation to 
 *                 get more infos on that topic.
 */
u32 enableSecureSocket(socket_t * pSocket, ssl_opts_t soOpts,
    const u8 * pu8CaFile, const u8 * pu8CaDir);

/**
 * ssl_localhost() returns the name of the localhost or 
 * the string "localhost" if the name of the localhost
 * cannot be determined for some reason.
 *
 * The resulting string is statically allocated and must not
 * be modified or freed.
 *
 * @returns The name of the localhost.
 */
//char * ssl_localhost ();

/**
 * ssl_connect() connects a socket to the specified
 * host:port and sets the timeout value. The struct ssl 
 * will be filled and is to be used with the other ssl_* 
 * functions.
 *
 * @param ssl      The struct ssl_socket_t that is to be filled.
 * @param host     The host that should be connected.
 * @param port     The port that the host is listening on.
 * @param timeout  The timeout value in seconds.
 * 
 * @returns 0 on success and -1 on any error.
 */
static u32 ssConnect(socket_t * pSocket);


#endif /*NETWORK_SECURESOCKET_H*/

/*---------------------------------------------------------------------------*/


