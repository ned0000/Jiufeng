/**
 *  @file logger/log2server.c
 *
 *  @brief Implementation file for logging to server.
 *
 *  @author Min Zhang
 *  
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#if defined(LINUX)
    #include <fcntl.h>
    #include <sys/errno.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h" 
#include "jf_err.h"
#include "jf_limit.h"
#include "jf_time.h"
#include "jf_process.h"
#include "jf_thread.h"
#include "jf_mem.h"

#include "log2server.h"
#include "log2servermsg.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Define the file log location data type.
 */
typedef struct
{
    /**Address of log server.*/
    olchar_t lsll_strServerAddress[JF_LIMIT_MAX_PATH_LEN];

    /**Port of log server.*/
    u16 lsll_u16ServerPort;
    u16 lsll_u16Reserved[3];

    /**Socket of the connection to the log server.*/
    olint_t lsll_nSocket;

    /**Sequence number of the log 2 server message.*/
    u32 lsll_u32SeqNum;

    /**The name of the calling module.*/
    olchar_t lsll_strCallerName[JF_LOGGER_MAX_CALLER_NAME_LEN];

} logger_server_log_location_t;

/* --- private routine section ------------------------------------------------------------------ */

/** Set the socket to non-block mode.
 *
 *  @note
 *  -# Connection is failed and the error message is "Operation now in progress" if setting
 *   non-block mode after creating the socket. The non-block mode should be set after setting up
 *   the connection
 *
 *  @param nSocket [in/out] The socket.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
static u32 _setSocketNonBlockForLogServer(olint_t nSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t flags = 0;

#if defined(LINUX)
    flags = fcntl(nSocket, F_GETFL, 0);
    fcntl(nSocket, F_SETFL, O_NONBLOCK | flags);
#elif defined(WINDOWS)
    flags = 1;
    ioctlsocket(nSocket, FIONBIO, &flags);
#endif

    return u32Ret;
}

static u32 _createSocketForLogServer(olint_t * pnSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    /*Create socket.*/
    *pnSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (*pnSocket < 0)
        u32Ret = JF_ERR_FAIL_CREATE_SOCKET;

    return u32Ret;
}

static u32 _connectToLogServer(olchar_t * pstrAddress, u16 u16Port, olint_t * pnSocket)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nRet = 0;
    struct sockaddr_in addr;
    olint_t salen = sizeof(addr);

    /*Return if the connection is already established.*/
    if (*pnSocket >= 0)
        return u32Ret;

    /*Create socket.*/
    u32Ret = _createSocketForLogServer(pnSocket);

    /*Convert the address from numbers-and-dots notation into binary form.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_bzero(&addr, sizeof(addr));
        addr.sin_family = AF_INET;
#if defined(LINUX)
        nRet = inet_aton(pstrAddress, &addr.sin_addr);
        if (nRet == 0)
            u32Ret = JF_ERR_INVALID_IP;
#elif defined(WINDOWS)
        addr.sin_addr.s_addr = inet_addr(pstrAddress);
        if (addr.sin_addr.s_addr == INADDR_NONE)
            u32Ret = JF_ERR_INVALID_IP;
#endif
    }

    /*Make the connection.*/
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        addr.sin_port = htons(u16Port);

        nRet = connect(*pnSocket, (struct sockaddr *)&addr, salen);
        if (nRet < 0)
            u32Ret = JF_ERR_FAIL_INITIATE_CONNECTION;
    }

    /*Set the socket to nonblock mode.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _setSocketNonBlockForLogServer(*pnSocket);

    return u32Ret;
}

static u32 _sendLogToServer(olint_t nSocket, void * buf, size_t len)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t size = 0;

    /*Send data.*/
    size = send(nSocket, buf, len, 0);

    /*Error if size of sent data is not as expected.*/
    if (size != len)
        u32Ret = JF_ERR_FAIL_SEND_DATA;

    return u32Ret;
}

/** Get sequence number.
 *
 *  @note
 *  -# ToDo: Lock may be required here in the multi-thread environment.
 *
 *  @param plsll [in/out] The log location object.
 *
 *  @return The sequence number.
 */
static u32 _getLog2ServerSeqNum(logger_server_log_location_t * plsll)
{
    u32 u32Seq = 0;

    u32Seq = plsll->lsll_u32SeqNum;
    plsll->lsll_u32SeqNum ++;

    return u32Seq;
}

static u32 _getLog2ServerSaveLogMsgSource(
    const olchar_t * pstrCallerName, olchar_t * pstrSource, olsize_t sSource)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    ol_snprintf(pstrSource, sSource, "[%s:%lu]", pstrCallerName, jf_thread_getCurrentId());
    pstrSource[sSource - 1] = '\0';

    return u32Ret;
}

static u32 _logToServer(
    logger_server_log_location_t * plsll, olchar_t * pstrLog, olsize_t sLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    log_2_server_save_log_svc_t msg;
    olint_t retry = 0, count = JF_LOGGER_RETRY_COUNT;
    olsize_t sPayload = 0, sMsg = 0;

    if (sLog > JF_LOGGER_MAX_MSG_SIZE)
        sLog = JF_LOGGER_MAX_MSG_SIZE;

    ol_bzero(&msg, sizeof(msg));

    jf_time_getUtcTimeInMicroSecond(&msg.l2ssls_u64Time);

    /*Set source to the message.*/
    _getLog2ServerSaveLogMsgSource(
        plsll->lsll_strCallerName, msg.l2ssls_strSource, sizeof(msg.l2ssls_strSource));

    ol_memcpy(msg.l2ssls_strLog, pstrLog, sLog);
    /*Add '\0' to the end of the log.*/
    if (sLog < JF_LOGGER_MAX_MSG_SIZE)
    {
        /*The message size is less than the maximum message size.*/
        msg.l2ssls_strLog[sLog] = '\0';
        sLog ++;
    }
    else
    {
        /*The message size reaches maximum message size.*/
        msg.l2ssls_strLog[sLog - 1] = '\0';
    }

    /*Send the message with real size, not the size of the data type.*/
    sMsg = sizeof(msg) - sizeof(msg.l2ssls_strLog) + sLog;
    sPayload = sMsg - sizeof(msg.l2ssls_l2smhHeader);

    msg.l2ssls_l2smhHeader.l2smh_u8MsgId = L2SMI_SAVE_LOG_SVC;
    msg.l2ssls_l2smhHeader.l2smh_u16Signature = LOG_2_SERVER_MSG_SIGNATURE;
    msg.l2ssls_l2smhHeader.l2smh_u32SeqNum = _getLog2ServerSeqNum(plsll);
    msg.l2ssls_l2smhHeader.l2smh_u16PayloadSize = (u16)sPayload;

    while (retry < count)
    {
        /*Connect to log server.*/
        u32Ret = _connectToLogServer(
            plsll->lsll_strServerAddress, plsll->lsll_u16ServerPort, &plsll->lsll_nSocket);

        /*Send mesage to log server.*/
        if (u32Ret == JF_ERR_NO_ERROR)
            u32Ret = _sendLogToServer(plsll->lsll_nSocket, &msg, sMsg);

        /*Quit if no error.*/
        if (u32Ret == JF_ERR_NO_ERROR)
            break;

        /*Error handling.*/
        close(plsll->lsll_nSocket);
        plsll->lsll_nSocket = -1;

        retry ++;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 destroyServerLogLocation(jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_server_log_location_t * plsll = *ppLocation;

    /*Destroy the socket.*/
    if (plsll->lsll_nSocket >= 0)
    {
        close(plsll->lsll_nSocket);
        plsll->lsll_nSocket = -1;
    }

    jf_mem_free(ppLocation);

    return u32Ret;
}

u32 createServerLogLocation(
    create_server_log_location_param_t * pParam, jf_logger_log_location_t ** ppLocation)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_server_log_location_t * plsll = NULL;

    assert((pParam->csllp_pstrServerAddress != NULL) && (pParam->csllp_u16ServerPort != 0));

    u32Ret = jf_mem_alloc((void **)&plsll, sizeof(*plsll));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        /*Save the caller name, server address and port.*/
        ol_bzero(plsll, sizeof(*plsll));
        if (pParam->csllp_pstrCallerName != NULL)
            ol_strcpy(plsll->lsll_strCallerName, pParam->csllp_pstrCallerName);

        plsll->lsll_u16ServerPort = pParam->csllp_u16ServerPort;
        ol_strncpy(
            plsll->lsll_strServerAddress, pParam->csllp_pstrServerAddress,
            sizeof(plsll->lsll_strServerAddress) - 1);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        *ppLocation = plsll;
    else if (plsll != NULL)
        destroyServerLogLocation((void **)&plsll);

    return u32Ret;
}

u32 logToServer(jf_logger_log_location_t * pLocation, olchar_t * pstrLog, olsize_t sLog)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    logger_server_log_location_t * plsll = pLocation;

    u32Ret = _logToServer(plsll, pstrLog, sLog);

#if defined(DEBUG_LOGGER)
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        /*Failed to log to server.*/
        olchar_t strDesc[JF_ERR_MAX_DESCRIPTION_SIZE];

        jf_err_readDescription(u32Ret, strDesc, sizeof(strDesc));

        ol_fprintf(
            stderr, "Log to server \"%s:%d\" failed - %s\n",
            plsll->lsll_strServerAddress, plsll->lsll_u16ServerPort, strDesc);
    }
#endif

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
