/**
 *  @file securesocket.c
 *
 *  @brief The secure socket
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "network.h"
#include "internalsocket.h"
#include "securesocket.h"
#include "xmalloc.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */
/** The SSL socket.*/
typedef struct secure_socket_t
{
#if defined(LINUX)
    boolean_t ss_bNegotiated;
    u8 ss_bReserved[7];
    ssl_opts_t ss_soOpts;
    SSL * ss_psSsl;
    SSL_CTX * ss_pscSslCtx;
    u8 * ss_pu8CaFile;
    u8 * ss_pu8CaDir;
    u32 ss_u32Timeout;
#elif defined(WINDOWS)
    u32 ss_u32Reserved[8];
#endif
} secure_socket_t;

/* --- private routine section---------------------------------------------- */


/* --- public routine section ---------------------------------------------- */

#define NA 65535 /* for ssl_error () */
 
/**
 * ssl_SSL_read () wraps around SSL_read and handles the 
 * following error conditions: SSL_ERROR_WANT_WRITE,
 * SSL_ERROR_WANT_READ, SSL_ERROR_WANT_CONNECT and 
 * SSL_ERROR_WANT_X509_LOOKUP.
 */
static u32 ssl_SSL_read(secure_socket_t * pss, void * pBuffer, u32 * pu32Count)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t bytes;

    bytes = SSL_read(pss->ss_psSsl, (olchar_t *)pBuffer, *pu32Count);

    while (SSL_get_error (pss->ss_psSsl, bytes) == SSL_ERROR_WANT_WRITE   ||
           SSL_get_error (pss->ss_psSsl, bytes) == SSL_ERROR_WANT_READ    ||
           SSL_get_error (pss->ss_psSsl, bytes) == SSL_ERROR_WANT_CONNECT ||
           SSL_get_error (pss->ss_psSsl, bytes) == SSL_ERROR_WANT_X509_LOOKUP)
    {
        bytes = SSL_read(pss->ss_psSsl, (olchar_t*)pBuffer, *pu32Count);
    }

    if (bytes < 1)
    {
        u32Ret = OLERR_FAIL_RECV_DATA;
    }
    else
    {
        *pu32Count = (u32)bytes;
    }

    return u32Ret;
}

/**
 * ssl_SSL_write () wraps around SSL_write and handles the 
 * following error conditions: SSL_ERROR_WANT_WRITE,
 * SSL_ERROR_WANT_READ, SSL_ERROR_WANT_CONNECT and 
 * SSL_ERROR_WANT_X509_LOOKUP.
 */
static u32 ssl_SSL_write(secure_socket_t * pss, void * pBuffer, u32 * pu32Count)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nRet;

    nRet = SSL_write(pss->ss_psSsl, (olchar_t*)pBuffer, *pu32Count);

    while (SSL_get_error (pss->ss_psSsl, nRet) == SSL_ERROR_WANT_WRITE   ||
           SSL_get_error (pss->ss_psSsl, nRet) == SSL_ERROR_WANT_READ    ||
           SSL_get_error (pss->ss_psSsl, nRet) == SSL_ERROR_WANT_CONNECT ||
           SSL_get_error (pss->ss_psSsl, nRet) == SSL_ERROR_WANT_X509_LOOKUP)
    {
        nRet = SSL_write(pss->ss_psSsl, (olchar_t *)pBuffer, *pu32Count);
    }

    if (nRet < 1)
    {
        u32Ret = OLERR_FAIL_RECV_DATA;
    }
    else
    {
        *pu32Count = (u32)nRet;
    }

    return (nRet);
}

/**
 * ssl_SSL_connect () wraps around SSL_connect and handles the 
 * following error conditions: SSL_ERROR_WANT_WRITE,
 * SSL_ERROR_WANT_READ, SSL_ERROR_WANT_CONNECT and 
 * SSL_ERROR_WANT_X509_LOOKUP.
 */
u32 ssConnect(socket_t * pSocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    secure_socket_t * pss = NULL;
    olint_t nRet;

    assert(pSocket != NULL);

    pis = (internal_socket_t *)pSocket;

    pss = (secure_socket_t *)pis->is_pPrivate;

    nRet = SSL_connect(pss->ss_psSsl);
    while (SSL_get_error(pss->ss_psSsl, nRet) == SSL_ERROR_WANT_WRITE   ||
           SSL_get_error(pss->ss_psSsl, nRet) == SSL_ERROR_WANT_READ    ||
           SSL_get_error(pss->ss_psSsl, nRet) == SSL_ERROR_WANT_CONNECT ||
           SSL_get_error(pss->ss_psSsl, nRet) == SSL_ERROR_WANT_X509_LOOKUP)
    {
        nRet = SSL_connect(pss->ss_psSsl);
    }

    if (nRet == 0)
        u32Ret = OLERR_FAIL_INITIATE_CONNECTION;

    return u32Ret;
}

/**
 * ssl_SSL_shutdown () wraps around SSL_shutdown and handles the 
 * following error conditions: SSL_ERROR_WANT_WRITE,
 * SSL_ERROR_WANT_READ, SSL_ERROR_WANT_CONNECT and 
 * SSL_ERROR_WANT_X509_LOOKUP.
 */
static u32 ssShutdown(socket_t * pSocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    secure_socket_t * pss = NULL;
    olint_t nRet;

    assert(pSocket != NULL);

    pis = (internal_socket_t *)pSocket;

    pss = (secure_socket_t *)pis->is_pPrivate;

    nRet = SSL_shutdown(pss->ss_psSsl);
    while (nRet == 0 ||
           SSL_get_error(pss->ss_psSsl, nRet) == SSL_ERROR_WANT_WRITE ||
           SSL_get_error(pss->ss_psSsl, nRet) == SSL_ERROR_WANT_READ ||
           SSL_get_error(pss->ss_psSsl, nRet) == SSL_ERROR_WANT_CONNECT ||
           SSL_get_error(pss->ss_psSsl, nRet) == SSL_ERROR_WANT_X509_LOOKUP)
    {
        nRet = SSL_shutdown(pss->ss_psSsl);
    }

    if (nRet == 0)
        u32Ret = OLERR_SECURE_SOCKET_SHUTDOWN_ERROR;

    return u32Ret;
}

/**
 * ssl_dump_info () logs some verbose information about the
 * SSL connection.
 *
 * @param ssl  The ssl_socket_t object.
 */
static void ssl_dump_info(secure_socket_t * pss)
{
    olchar_t *str = NULL;
    X509 *server_cert;

    str = (olchar_t*)SSL_get_cipher(pss->ss_psSsl);
    if (str == NULL)
        return;

    ol_printf("SSL connection using %s\n", str);

    OPENSSL_free(str);
  
    server_cert = SSL_get_peer_certificate(pss->ss_psSsl);
    if (server_cert == NULL)
        return;

    /* X509_NAME_oneline should not be used according to the manual page,
     * but i dunno any other way to get the X509 infos */
    ol_printf("server certificate - subject:\n");
    ol_printf("=============================");
    
    str = X509_NAME_oneline(X509_get_subject_name(server_cert), 0, 0);
    if (str == NULL)
    {
        X509_free(server_cert);
        return;
    }

    ol_printf("%s", str);

    OPENSSL_free (str);

    /* X509_NAME_oneline should not be used according to the manual page,
     * but i dunno any other way to get the X509 infos */
    ol_printf("server certificate - issuer:\n");
    ol_printf("============================");

    str = X509_NAME_oneline(X509_get_issuer_name(server_cert), 0, 0);
    if (str == NULL)
    {
        X509_free(server_cert);
        return;
    }

    ol_printf("%s\n", str);

    OPENSSL_free(str);
    X509_free(server_cert);
}
#if 0
/**
 * ssl_error() returns the error string corresponding to
 * to the return value to a call to ssl_SSL_connect(), 
 * ssl_SSL_read(), or ssl_SSL_write(). 
 * ret == NA may be used if no other ssl error code is available.
 * The resulting string is statically allocated and must not
 * be modified or freed.
 *
 * @param ssl  The ssl_socket_t object.
 * @param ret  The ssl error code.
 *
 * @returns The corresponding error string to ret.
 */
static olchar_t * ssl_error(ssl_socket_t * ssl, olint_t ret)
{
    static olchar_t buf[4096];
    
    if (SSL_get_error (pss->ss_psSsl, ret) == SSL_ERROR_SYSCALL && ERR_peek_error () == 0)
    {
        strncpy (buf, strerror (errno), 4095);
    }
    else if (ERR_peek_error () != 0)
    {
        ERR_error_string_n (ERR_peek_error(), buf, 4095);
    }
    else if (ret == 0)
    {
        strcpy (buf, "Socket is closed.");
    }
    else
    {
        strcpy (buf, "An unknown error has occured.");
    }
    
    buf[4095] = '\0';
    return (buf);
}
#endif
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
u32 ssRecv(socket_t * pSocket, void * pBuffer, u32 * pu32Count)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    secure_socket_t * pss = NULL;
    olint_t bytes;

    assert(pSocket != NULL);

    pis = (internal_socket_t *)pSocket;
    pss = (secure_socket_t *)pis->is_pPrivate;

    if (! pss->ss_bNegotiated)
        u32Ret = OLERR_SECURE_SOCKET_NOT_NEGOTIATED;

    if (pss->ss_bNegotiated)
    {
        bytes = ssl_SSL_read(pss, pBuffer, pu32Count);
    }
    else
    {
        return (isRecv(pis, pBuffer, pu32Count));
    }

    return u32Ret;
}

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
u32 ssSend(socket_t * pSocket, void * pBuffer, u32 * pu32Count)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    secure_socket_t * pss = NULL;
    olint_t bytes = 0;

    assert(pSocket != NULL);

    pis = (internal_socket_t *)pSocket;

    pss = (secure_socket_t *)pis->is_pPrivate;

    if (! pss->ss_bNegotiated)
        u32Ret = OLERR_SECURE_SOCKET_NOT_NEGOTIATED;

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = ssl_SSL_write(pss, pBuffer, pu32Count);
    }
    else
    {
        u32Ret = isSend(pis, pBuffer, pu32Count);
    }

    return (bytes);
}

/**
 * ssl_negotiate() starts the SSL handshake.
 *
 * @param ssl The ssl_socket_t object.
 *
 * @returns -1 on an error, otherwise 0.
 */
u32 ssNegotiate(socket_t * pSocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    secure_socket_t * pss = NULL;
    olint_t nRet;

    assert(pSocket != NULL);

    pis = (internal_socket_t *)pSocket;
    pss = (secure_socket_t *)pis->is_pPrivate;

    pss->ss_pscSslCtx = SSL_CTX_new(SSLv23_client_method());
    if (pss->ss_pscSslCtx == NULL) 
    { 
        u32Ret = OLERR_CREATE_SECURE_SOCKET_ERROR;
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        pss->ss_psSsl = SSL_new(pss->ss_pscSslCtx);
        if (pss->ss_psSsl == NULL) 
        {
            u32Ret = OLERR_CREATE_SECURE_SOCKET_ERROR;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
//        SSL_CTX_set_timeout(pss->ss_pscSslCtx, timeout);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pss->ss_soOpts != VERIFY_NONE)
        {
            /* check accessability of pss->ss_pu8CaFile */
            if (pss->ss_pu8CaFile != NULL)
            {
                if (access(pss->ss_pu8CaFile, F_OK) != 0)
                {
                    u32Ret = OLERR_FILE_NOT_FOUND;
                }
                else if (access(pss->ss_pu8CaFile, R_OK) != 0)
                {
                    u32Ret = OLERR_FILE_NOT_FOUND;
                }
            }

            /* check accessability of pss->ss_pu8CaDir */
            if ((u32Ret == OLERR_NO_ERROR) && (pss->ss_pu8CaDir != NULL))
            {
                if (access(pss->ss_pu8CaDir, F_OK) != 0)
                {
                    u32Ret = OLERR_FILE_NOT_FOUND;
                }
                else if (access(pss->ss_pu8CaDir, R_OK) != 0)
                {
                    u32Ret = OLERR_FILE_NOT_FOUND;
                }
            }
        }
    } 

    if (u32Ret == OLERR_NO_ERROR)
    {
        nRet = SSL_CTX_load_verify_locations(pss->ss_pscSslCtx, 
            pss->ss_pu8CaFile != NULL ? pss->ss_pu8CaFile : NULL,
            pss->ss_pu8CaDir != NULL ? pss->ss_pu8CaDir : NULL);
        if (nRet != 1) 
        { 
            u32Ret = OLERR_SECURE_SOCKET_NEGOTIATE_ERROR;
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        SSL_set_verify(pss->ss_psSsl, pss->ss_soOpts, NULL);
        SSL_set_fd(pss->ss_psSsl, pis->is_isSocket);
    
        u32Ret = ssConnect(pss->ss_psSsl);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        if (pss->ss_soOpts != VERIFY_NONE)
        {
            X509 * cert = SSL_get_peer_certificate(pss->ss_psSsl);
            if (cert == NULL)
            {
                u32Ret = OLERR_SECURE_SOCKET_NEGOTIATE_ERROR;
            }
            else if (SSL_get_verify_result(pss->ss_psSsl) != X509_V_OK)
            {
                X509_free (cert);
                u32Ret = OLERR_SECURE_SOCKET_NEGOTIATE_ERROR;
            }
            X509_free(cert);
        }

        ssl_dump_info(pss);

        pss->ss_bNegotiated = TRUE;
    }

    return u32Ret;
}

/**
 * Set some SSL specific options.
 *
 * @param ssl      The ssl_socket_t object.
 * @param opts     The SSL verification level.
 * @param ss_pu8CaFile  A file that contains PEM certificates. Must
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
 * @param ss_pu8CaDir   A directory that contains PEM certificates. Must
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
    const u8 * pu8CaFile, const u8 * pu8CaDir)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_socket_t * pis = (internal_socket_t *)pSocket;
    secure_socket_t * pss = (secure_socket_t *)pis->is_pPrivate;

    assert(pSocket != NULL);
    assert(! pis->is_bSecure);

    u32Ret = xcalloc((void **)&(pis->is_pPrivate), sizeof(secure_socket_t));
    if (u32Ret == OLERR_NO_ERROR)
    {
        pis->is_bSecure = TRUE;
        pss->ss_soOpts = soOpts;

        if (pss->ss_pu8CaFile != NULL)
            u32Ret = dupString(&(pss->ss_pu8CaFile), pu8CaFile);

        if (u32Ret == OLERR_NO_ERROR)
        {
            if (pss->ss_pu8CaDir != NULL)
                u32Ret = dupString(&(pss->ss_pu8CaDir), pu8CaDir);
        }
    }

    return u32Ret;
}

/** destroy the secure socket
 *
 *  @param ssl  The socket that should be closed.
 *
 *  @returns 0 on success and -1 on any error.
 */
u32 destroySecureSocket(socket_t ** ppSocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    secure_socket_t * pss = NULL;

    assert((ppSocket != NULL) && (*ppSocket != NULL));

    pis = (internal_socket_t *)*ppSocket;

    pss = (secure_socket_t *)pis->is_pPrivate;

#if defined(LINUX)
    if (pss != NULL)
    {
        if (pss->ss_psSsl != NULL)
        {
            ssShutdown(pss->ss_psSsl);

            SSL_clear(pss->ss_psSsl);
            SSL_free(pss->ss_psSsl);
            pss->ss_psSsl = NULL;
        }

        if (pss->ss_pscSslCtx != NULL)
        {
            SSL_CTX_free(pss->ss_pscSslCtx);
            pss->ss_pscSslCtx = NULL;
        }

        if (pss->ss_pu8CaFile != NULL)
            freeString(&(pss->ss_pu8CaFile));

        if (pss->ss_pu8CaDir != NULL)
            freeString(&(pss->ss_pu8CaDir));

        xfree((void **)&pss);
    }
#elif defined(WINDOWS)

#endif

    freeIsocket((internal_socket_t **)ppSocket);

    return u32Ret;
}

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
    socket_t ** pSocket)
{
    u32 u32Ret = OLERR_NO_ERROR;
    internal_socket_t * pis = NULL;
    secure_socket_t * pss;

    u32Ret = createIsocket(domain, type, protocol, &pis);
    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = xcalloc((void **)&(pis->is_pPrivate), sizeof(secure_socket_t));
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        pis->is_bSecure = TRUE;
        pss = (secure_socket_t *)pis->is_pPrivate;

        pss->ss_soOpts = VERIFY_PEER;

        SSLeay_add_ssl_algorithms();
        ERR_load_crypto_strings();
        SSL_load_error_strings();
    }

    if (u32Ret == OLERR_NO_ERROR)
        *pSocket = pis;
    else if (pis != NULL)
        destroySecureSocket((socket_t **)&pis);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


