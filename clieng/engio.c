/**
 *  @file engio.c
 *
 *  @brief The clieng io module
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if defined(LINUX)
    #include <termios.h>
    #include <sys/ioctl.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */
#include "jf_basic.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "engio.h"
#include "jf_hex.h"

/* --- private data/data structure section ------------------------------------------------------ */

const static olchar_t * cls_pstrCaptionDelimit = ": ";

const static olchar_t * cls_pstrByteHexDumpBegin = "Begin Hex Dump in Byte";
const static olchar_t * cls_pstrByteHexDumpEnd = "End Hex Dump in Byte";

const static olchar_t * cls_pstrBanner = "\
====================================================================================================";

const static olchar_t * cls_pstrDivider = "\
----------------------------------------------------------------------------------------------------";

const static olchar_t * cls_pstrBannerShift4 = "\
    ================================================================================================";

const static olchar_t * cls_pstrDividerShift2 = "\
  --------------------------------------------------------------------------------------------------";

typedef struct
{
    boolean_t ici_bInitialized;
    u8 ici_u8Reserved[7];
    /* ";" */
	olchar_t ici_strSpecialDelimit[8];
	olchar_t ici_strNewLine[8];
	olchar_t ici_strBlankSpaces[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
    olchar_t ici_strOutputBuffer[MAX_OUTPUT_BUFFER_LEN];
    olsize_t ici_sNewLine;
	u8 ici_u8MoreLines;
    u8 ici_u8Reserved4[3];
    /*is special delimit on? By default it is off, blank spaces are used for
      delimit; when it is on, a semi-column is used. */
	boolean_t ici_bSpecialDelimit;
	boolean_t ici_bMoreEnabled;
	boolean_t ici_bMoreCancel;
    u8 ici_u8Reserved2[3];
    jf_filestream_t * ici_pjfOutput;
} internal_clieng_io_t;

#ifndef WINDOWS
    #define MAX_MORE_LINES    _getMaxMoreLines()
#else
    #define MAX_MORE_LINES    (23)
#endif

/** Maximum number of command in command history
 */
#define MAX_CMD_HISTORY       (64)

/** use LEFT-SPACE-LEFT to implement backspace
 */
static olchar_t str_backspace[] = {27, '[', 'D', ' ', 27, '[', 'D'};

static internal_clieng_io_t ls_iciCliengIo;

/* --- private routine section ------------------------------------------------------------------ */

#ifndef WINDOWS
u32 _getMaxMoreLines()
{
    struct winsize size;

    if (ioctl(0, TIOCGWINSZ, (olchar_t *) &size) < 0)
    {
        return 23;
    }
    else
    {
        if (size.ws_row == 0)
        {
            return 23;
        }
    }
    
    return size.ws_row - 2;
}
#endif

#ifndef WINDOWS
static u32 _switchToRawMode(olint_t nTTY)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct termios tTTYOptions;
    olint_t nRet;

    nRet = tcgetattr(nTTY, &tTTYOptions);
    if (nRet == -1)
    {
        u32Ret = JF_ERR_OPERATION_FAIL;
    }
    else
    {
        tTTYOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        nRet = tcsetattr(nTTY, TCSANOW, &tTTYOptions);
        if (nRet == -1)
        {
            u32Ret = JF_ERR_OPERATION_FAIL;
        }
    }

    return u32Ret;
}

static u32 _switchToCanonMode(olint_t nTTY)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    struct termios tTTYOptions;
    olint_t nRet;

    nRet = tcgetattr(nTTY, &tTTYOptions);
    if (nRet == -1)
    {
        u32Ret = JF_ERR_OPERATION_FAIL;
    }
    else
    {
        tTTYOptions.c_lflag |= (ICANON | ECHO | ECHOE | ISIG);
        nRet = tcsetattr(nTTY, TCSANOW, &tTTYOptions);
        if (nRet == -1)
        {
            u32Ret = JF_ERR_OPERATION_FAIL;
        }
    }

    return u32Ret;
}
#endif


static void _back_space_simple(void)
{
#ifndef WINDOWS
    putchar(str_backspace[0]);
    putchar(str_backspace[1]);
    putchar(str_backspace[2]);
    putchar(str_backspace[3]);
    putchar(str_backspace[4]);
    putchar(str_backspace[5]);
    putchar(str_backspace[6]);
#endif
}

static void _right_arrow_simple(void)
{
    putchar(27);
    putchar(91);
    putchar(67);
}

static void _left_arrow_simple(void)
{
    putchar(27);
    putchar(91);
    putchar(68);
}

static void _down_arrow_simple(void)
{
    putchar(27);
    putchar(91);
    putchar(66);
}

static void _upper_arrow_simple(void)
{
    putchar(27);
    putchar(91);
    putchar(65);
}

static void _back_space(u32 u32Position, u32 u32ScreenSize)
{
    u32 i;

    if (u32Position%u32ScreenSize != 0)
    {
        _back_space_simple();
    }
    else
    {
        _upper_arrow_simple();
        for (i=1; i<u32ScreenSize; i++)
        {
            _right_arrow_simple();
        }
        putchar(' ');
        _back_space_simple();
    }
}

static void _right_arrow(u32 u32Position, u32 u32ScreenSize)
{
    u32 i;

    if (u32Position%u32ScreenSize != 0)
    {
        _right_arrow_simple();
    }
    else
    {
        _down_arrow_simple();
        for (i=1; i<u32ScreenSize; i++)
        {
            _left_arrow_simple();
        }
    }
}

static void _left_arrow(u32 u32Position, u32 u32ScreenSize)
{
    u32 i;

    if (u32Position%u32ScreenSize != 0)
    {
        _left_arrow_simple();
    }
    else
    {
        _upper_arrow_simple();
        for (i=1; i<u32ScreenSize; i++)
        {
            _right_arrow_simple();
        }
    }
}

static u32 _getInputPassword(olchar_t *pBuf, olsize_t * pLen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t length;
    olint_t nChar;

    length = 0;
    do
    {
        nChar = getchar();
        /* ignore the extra carriage return from windows 2000 */
        if (nChar == 13)
        {
            ;
        }
        else if (nChar == '\n')
            /* return */
        {
            putchar(nChar);
        } else if (nChar == 127 || nChar == 8)
            /* backspace */
        {
            if (length > 0)
            {
                length--;
                _back_space_simple();
            }
        }
        else
        {
            putchar('*');
            pBuf[length] = nChar;
            length++;
        }
    } while ((nChar != '\n') && (length < *pLen));
    pBuf[length] = 0;

    return u32Ret;
}

static u32 _getInputLine(
    clieng_input_type_t *pType, olchar_t * pBuf,
    olsize_t * pLen, olsize_t sPrompt)
{
#define MAX_INPUT_LEN (2 * JF_CLIENG_MAX_COMMAND_LINE_SIZE)
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t string[MAX_INPUT_LEN + 8];
    olchar_t cur_char[3];
    olint_t str_length = 0, ColNum = 0, PromptLen = 0;
    olint_t i = 0, j = 0, nRet = 0;
#ifndef WINDOWS
    struct winsize size;

    if (ioctl(0, TIOCGWINSZ, (olchar_t *) &size) < 0)
    {
        ColNum = 80;
    }
    else
    {
        ColNum = size.ws_col;
        if (ColNum == 0)
        {
            ColNum = 80;
        }
    }
#else
    ColNum = 80;
#endif

    PromptLen = sPrompt + 6;

    /* from history command */
    if (*pType == cit_navigation_cmd)
    {
        str_length = ol_strlen((olchar_t *)pBuf);
        if (str_length > MAX_INPUT_LEN)
        {
            str_length = MAX_INPUT_LEN;
        }
        for (i=0; i<str_length; i++)
        {
            putchar(pBuf[i]);
        }
        ol_strncpy(string, (olchar_t *)pBuf, str_length);
        i = str_length;
    }
    if ((str_length+PromptLen) % ColNum == 0)
    {
        putchar(' ');
        _back_space_simple();
    }

    while (nRet != TRUE)
    {
        cur_char[0] = getchar();
        /* ignore the extra carriage return from windows 2000 */
        if (cur_char[0] == 13)
        {
            ;
        }
        /* ignore tab key */
        else if (cur_char[0] == 9)
        {
            ;
        }
        else if (cur_char[0] == '\n')
            /* end of command input */
        {
            if (i < str_length)
            {
                i = str_length;
            }
#ifndef WINDOWS
            putchar(cur_char[0]);
#endif
            string[i] = '\0';

            if (*pLen < ol_strlen(string))
            {
                u32Ret = JF_ERR_CMD_TOO_LONG;
            }
            else
            {
                ol_strcpy((olchar_t *)pBuf, string);
                *pLen = ol_strlen(string);
            }
            *pType = cit_line;
            nRet = TRUE;
        }
        else if (cur_char[0] == 27)
            /* escape, function key */
        {
            cur_char[1] = getchar();
            cur_char[2] = getchar();
            if (cur_char[2] == 'A' || cur_char[2] == 'B')
            {
                /* Up and Down Arrow */
                *pType = cit_navigation_cmd;
                pBuf[0] = cur_char[2];
                pBuf[1] = '\0';
                nRet = TRUE;
                /* move cursor to end of line */
                for(j=i; j<str_length; j++)
                {
                    _right_arrow(j+PromptLen+1, ColNum);
                }
                /* back_space to begining of line */
                for(j=str_length; j>0; j--)
                {
                    _back_space(j+PromptLen, ColNum);
                }
            }
            else if (cur_char[2] == 'D')
                /* left arrow */
            {
                if (i > 0)
                {
                    _left_arrow(i+PromptLen, ColNum);
                    i --;
                }
            }
            else if (cur_char[2] == 'C')
                /* right arrow */
            {
                if (i < str_length)
                {
                    _right_arrow(i+PromptLen+1, ColNum);
                    i ++;
                }
            }
            else if (cur_char[2] == 'F' ||
                     cur_char[2] == 52 || cur_char[2] == 53)
                /* End key */
            {
                /* get rid of the extra character from some keyboard */
                if (cur_char[2] == 52 || cur_char[2] == 53)
                {
                    getchar();
                }
                for (j=i; j<str_length; j++)
                {
                    i ++;
                    _right_arrow(j+PromptLen+1, ColNum);
                }
            }
            else if (cur_char[2] == 'H' ||
                     cur_char[2] == 49 || cur_char[2] == 50)
                /* Home key */
            {
                /* get rid of the extra character from some keyboard */
                if (cur_char[2] == 49 || cur_char[2] == 50)
                {
                    getchar();
                }
                for (j=i; j>0; j--)
                {
                    i --;
                    _left_arrow(j+PromptLen, ColNum);
                }
            }
        }
        else if (cur_char[0] == 127 || cur_char[0] == 8)
            /* backspace */
        {
            if (i > 0)
            {
                _back_space(i+PromptLen, ColNum);
                i--;
                str_length--;
                /* re-display the rest of the line */
                for (j=i; j<str_length; j++)
                {
                    string[j] = string[j+1];
                    putchar(string[j]);
                }
                /* take care of the last extra character */
                putchar(' ');
                _back_space_simple();
                /* move cursor back to position */
                for (j=str_length; j>i; j--)
                {
                    _left_arrow(j+PromptLen, ColNum);
                }
            }
        }
        else
            /* insert the character */
        {
            if (str_length >= MAX_INPUT_LEN)
            {
                return JF_ERR_CMD_TOO_LONG;
            }
            if (isprint(cur_char[0]) == 0)
            {
                return JF_ERR_INVALID_COMMAND;
            }
#ifndef WINDOWS
            putchar(cur_char[0]);
#endif
            for (j=str_length; j>i; j--)
            {
                string[j] = string[j-1];
            }
            string[i] = cur_char[0];
            i++;
            str_length++;
            if (i == str_length)
            {
                if ((str_length+PromptLen) % ColNum == 0)
                {
#ifndef WINDOWS
                    putchar(' ');
                    _back_space_simple();
#endif
                }
            }
            else if (i < str_length)
            {
                /* re-display the line */
                for (j=i; j<str_length; j++)
                {
                    putchar(string[j]);
                }
                if ((str_length+PromptLen) % ColNum == 0)
                {
                    putchar(' ');
                    _back_space_simple();
                }
                /* move cursor back to position */
                for (j=str_length; j>i; j--)
                {
                    _left_arrow(j+PromptLen, ColNum);
                }
            }
        }
    } /* while loop */
    
    return u32Ret;
}

static u32 _getAnyKey(
    clieng_input_type_t * pType, olchar_t * pBuf,
    olsize_t *pLen, olsize_t sPrompt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t cur_char[10];
    olint_t nRet = FALSE;

    while (nRet != TRUE)
    {
        cur_char[0] = getchar();
        if (cur_char[0] == 0x1B)
        {
            /* escape, function key */
            cur_char[1] = getchar();
            while (cur_char[1] == 0x1B)
            {
                cur_char[1] = getchar();
            }

            *pLen = 3;
            cur_char[2] = getchar();
            if (cur_char[2] >= 0x31 && cur_char[2] <= 0x36)
            {
                *pLen = 4;
                cur_char[3] = getchar();
                if (cur_char[3] != 0x7e)
                {
                    *pLen = 5;
                    cur_char[4] = getchar();
                }
            }

            memcpy(pBuf, cur_char, *pLen);
            *pType = cit_anykey;
            nRet = TRUE;
        }
        else
        {
            /* ignore the extra carriage return from windows 2000 */
            if(cur_char[0] == 0xD)
            {
                ;
            }
            else
            {
                pBuf[0] = cur_char[0];
                pBuf[1] = '\n';
                *pLen = 1;
                *pType = cit_anykey;
                nRet = TRUE;
            }
        }
    } /* while loop */

    return u32Ret;
}

static u32 _getInput(
    clieng_input_type_t *pType, olchar_t *pBuf, olsize_t * pLen, olsize_t promptLen)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    switch (*pType)
    {
    case cit_password:
        u32Ret = _getInputPassword(pBuf, pLen);
        *pType = cit_password;
        break;
    case cit_line:
    case cit_navigation_cmd:
    case cit_unknown:
        u32Ret = _getInputLine(pType, pBuf, pLen, promptLen);
        break;
    case cit_anykey:
        u32Ret = _getAnyKey(pType, pBuf, pLen, promptLen);
        break;
    default:
        u32Ret = JF_ERR_NOT_IMPLEMENTED;
        break;
    }

    return u32Ret;
}

static u32 _postOutput(
    internal_clieng_io_t * pici, clieng_output_type_t type, const olchar_t * pBuf, u32 len)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (type != cot_text)
    {
        u32Ret = JF_ERR_NOT_IMPLEMENTED;
    }
    else
    {
        jf_filestream_printf(pici->ici_pjfOutput, "%s", pBuf);
        jf_filestream_flush(pici->ici_pjfOutput);
    }

    return u32Ret;
}

static u32 _waitForMore(internal_clieng_io_t * pici)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t * more_text = "---Press ENTER to continue or CTRL-X to quit---";
    olchar_t * remove_more = "\r                                                 \r";
    olchar_t * enter_line = "\n";
    olsize_t sInput;
    olchar_t strInputKey[JF_CLIENG_MAX_OUTPUT_LINE_LEN];
    clieng_input_type_t citInputType = cit_anykey;
    
    u32Ret = _postOutput(pici, cot_text, more_text, ol_strlen(more_text));

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _getInput(&citInputType, strInputKey, &sInput, 49);

    if (u32Ret == JF_ERR_NO_ERROR)    
        u32Ret = _postOutput(
            pici, cot_text, remove_more, ol_strlen(remove_more));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (strInputKey[0] == 0x18) /* CTRL- X */
        {
            u32Ret = _postOutput(
                pici, cot_text, enter_line, ol_strlen(enter_line));
            u32Ret = JF_ERR_MORE_CANCELED;
        }
    }

    return u32Ret;
}

/** Check whether the caption list is valid for printing.
 *
 *  @param pjcc [in] the pointer to the caption list
 *  @param u32Count [in] the caption count in the caption list.
 *
 *  @return the error code
 */
static u32 _checkCaption(const jf_clieng_caption_t * pjcc, u32 u32Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Length;

    u32Length = 0;
    for (u32Index = 0; u32Index < u32Count; u32Index++)
    {
        u32Length += pjcc[u32Index].jcc_u32Len;
    }
    if (u32Length > JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1)
    {
        u32Ret = JF_ERR_LINE_TOO_LONG;
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

u32 initCliengIo(clieng_io_param_t * pcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olint_t nTTY = 0; /* the current tty ??? */
    
    assert(pcip != NULL);
    assert(! pici->ici_bInitialized);

    ol_memset(pici, 0, sizeof(internal_clieng_io_t));
    pici->ici_bSpecialDelimit = FALSE;
    pici->ici_strSpecialDelimit[0] = ';';
    pici->ici_strSpecialDelimit[1] = 0;
    ol_strcpy(pici->ici_strNewLine, pcip->cip_pstrNewLine);
    pici->ici_sNewLine = ol_strlen(pici->ici_strNewLine);
    ol_memset(pici->ici_strBlankSpaces, ' ', JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1);
    pici->ici_strBlankSpaces[JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1] = '\0';
    pici->ici_pjfOutput = (pcip->cip_pjfOutput == NULL) ? stdout : pcip->cip_pjfOutput;

    setMoreDisable();

#ifndef WINDOWS
    u32Ret = _switchToRawMode(nTTY);
#endif

    if (u32Ret == JF_ERR_NO_ERROR)
        pici->ici_bInitialized = TRUE;
    else
        finiCliengIo();

    return u32Ret;
}

u32 finiCliengIo(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
#ifndef WINDOWS
    olint_t nTTY = 0;

    _switchToCanonMode(nTTY);
#endif

    pici->ici_bInitialized = FALSE;

    return u32Ret;
}

u32 engioInput(
    clieng_input_type_t * pcitInputType, olchar_t * pstrInputBuf, olsize_t * psInputBuf,
    olsize_t sPrompt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_clieng_io_t * pici = &ls_iciCliengIo;
    
    assert(pcitInputType != NULL && pstrInputBuf != NULL && psInputBuf != NULL);

    u32Ret = _getInput(pcitInputType, pstrInputBuf, psInputBuf, sPrompt);
    
    return u32Ret;   
}

u32 voutputLine(const olchar_t * fmt, va_list ap)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olchar_t strBuffer[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];
    olsize_t sLength = 0, sRestOutput = 0;
    olchar_t * pstrLineBegin = 0;

    assert(fmt != NULL);

    sLength = ol_vsnprintf(pici->ici_strOutputBuffer, MAX_OUTPUT_BUFFER_LEN, fmt, ap);
    
    pstrLineBegin = pici->ici_strOutputBuffer;
    sRestOutput = sLength;
    if (sRestOutput == 0)
    {
        /* to prolint_t a empty line */
        sRestOutput = 1;
        pstrLineBegin[0] = ' ';
        pstrLineBegin[1] = 0;
    }
    
    while ((sRestOutput > 0) && (u32Ret == JF_ERR_NO_ERROR))
    {
        /* before output */
        if (pici->ici_bMoreEnabled)
        {
            if (pici->ici_bMoreCancel)
            {
                sRestOutput = 0;
            }
            else
            {
                if (pici->ici_u8MoreLines >= MAX_MORE_LINES)
                {
                    u32Ret = _waitForMore(pici);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        pici->ici_u8MoreLines = 0;
                    }
                    else if (u32Ret == JF_ERR_MORE_CANCELED)
                    {
                       pici->ici_bMoreCancel = TRUE;
                       sRestOutput = 0;
                       u32Ret = JF_ERR_NO_ERROR;
                    }
                }
            }
        }

        if ((sRestOutput > 0) && (u32Ret == JF_ERR_NO_ERROR))
        {
            if (sRestOutput >= JF_CLIENG_MAX_OUTPUT_LINE_LEN)
            {
                ol_memcpy(strBuffer, pstrLineBegin, JF_CLIENG_MAX_OUTPUT_LINE_LEN);
                sLength = JF_CLIENG_MAX_OUTPUT_LINE_LEN;
            }
            else
            {
                ol_memcpy(strBuffer, pstrLineBegin, sRestOutput);
                sLength = sRestOutput;
            }
            
            pstrLineBegin += sLength;
            sRestOutput -= sLength;
            strBuffer[sLength] = 0;
            sLength = sLength + pici->ici_sNewLine;
            ol_strcat(strBuffer, pici->ici_strNewLine);
            
            u32Ret = _postOutput(pici, cot_text, strBuffer, sLength);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                (pici->ici_u8MoreLines)++;
            }
        }
    }
    
    return u32Ret;
}

u32 outputLine(const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
//    internal_clieng_io_t * pici = &ls_iciCliengIo;
    va_list vlParam;

    assert(fmt != NULL);

    va_start(vlParam, fmt);
    voutputLine(fmt, vlParam);
    va_end(vlParam);

    return u32Ret;
}

/** When calling output(), the output length must not exceed JF_CLIENG_MAX_OUTPUT_LINE_LEN
 *  Otherwise, it will be truncated.
 */
u32 engioOutput(const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olchar_t strBuffer[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];
    va_list vlParam; 
    olsize_t length = 0;
    
    assert(fmt != NULL);

    va_start(vlParam, fmt);
    length = ol_vsnprintf(strBuffer, JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8, fmt, vlParam);
    va_end(vlParam);
    
    strBuffer[length] = 0;
    
    if ((u32Ret == JF_ERR_NO_ERROR) && (length > 0))
    {
        /* before output */
        if (pici->ici_bMoreEnabled)
        {
            if (pici->ici_bMoreCancel)
            {
                length = 0;
            }
            else
            {
                if (pici->ici_u8MoreLines >= MAX_MORE_LINES)
                {
                    u32Ret = _waitForMore(pici);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        pici->ici_u8MoreLines = 0;
                    }
                    else if (u32Ret == JF_ERR_MORE_CANCELED)
                    {
                       pici->ici_bMoreCancel = TRUE;
                       length = 0;
                       u32Ret = JF_ERR_NO_ERROR;
                    }
                }
            }
        }

        if ((length > 0) && (u32Ret == JF_ERR_NO_ERROR))
        {
            u32Ret = _postOutput(pici, cot_text, strBuffer, length);
        }
    }
    
    return u32Ret;
}

u32 outputParagraph(const olchar_t * pstrParagraph)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olsize_t length = 0;
    olchar_t strBuffer[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];
    olchar_t * pu8MoreStringBegin = NULL;
    olchar_t * pu8MoreStringEnd = NULL;
    boolean_t bPrintLastLine = FALSE;

    assert(pstrParagraph != NULL);

    length = (u32)strlen(pstrParagraph);
        
    if (pstrParagraph[length-1] != '\n')
    {
        bPrintLastLine = TRUE;
    }
        
    if (pici->ici_bMoreEnabled)
    {
        pu8MoreStringBegin = (char*)pstrParagraph;
        pu8MoreStringEnd = (char*)pstrParagraph;
        do
        {
            pu8MoreStringEnd = strchr(pu8MoreStringBegin, '\n');

            if ((pu8MoreStringEnd == NULL) && (bPrintLastLine == FALSE))
            {
                break;
            }

            if (pici->ici_u8MoreLines >= MAX_MORE_LINES)
            {
                u32Ret = _waitForMore(pici);
                if (u32Ret == JF_ERR_NO_ERROR)
                {
                    pici->ici_u8MoreLines = 0;
                }
                else if (u32Ret == JF_ERR_MORE_CANCELED)
                {
                    pici->ici_bMoreCancel = TRUE;
                    return JF_ERR_NO_ERROR;
                }
                else
                {
                    return u32Ret;
                }
            }

            if (pu8MoreStringEnd != NULL)
            {
                length = pu8MoreStringEnd - pu8MoreStringBegin;
            }
            else
            {
                length = ol_strlen(pu8MoreStringBegin);
            }
				
            if (length > JF_CLIENG_MAX_OUTPUT_LINE_LEN)
            {
                u32Ret = JF_ERR_LINE_TOO_LONG;
            }
            else
            {
                ol_strncpy(strBuffer, pu8MoreStringBegin, length);
                strBuffer[length] = 0;
                ol_strcat(strBuffer, pici->ici_strNewLine);
                length += ol_strlen(pici->ici_strNewLine);
            
                if (strcmp("\n", pici->ici_strNewLine) == 0)
                {
                    u32Ret = _postOutput(pici, cot_text, strBuffer, length);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        pici->ici_u8MoreLines ++;
                    }
                }
                else
                {
                    u32Ret = JF_ERR_NOT_IMPLEMENTED;
                }
            
                if (pu8MoreStringEnd != NULL)
                {
                    pu8MoreStringBegin = pu8MoreStringEnd + 1;
                }
            }
        } while ((pu8MoreStringEnd != NULL) && (u32Ret == JF_ERR_NO_ERROR));
    }
    else
    {
        if (ol_strcmp("\n", pici->ici_strNewLine) == 0)
        {
            u32Ret = _postOutput(pici, cot_text, pstrParagraph, length);
        }
        else
        {
            u32Ret = JF_ERR_NOT_IMPLEMENTED;
        }
    }
    
    return u32Ret;
}

u32 switchSpecialDelimit(boolean_t bSwitchOn)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
        
    pici->ici_bSpecialDelimit = bSwitchOn;
    
    return u32Ret;
}

char * getEOL(void)
{
    olchar_t * pstrNewLine = NULL;
    internal_clieng_io_t * pici = &ls_iciCliengIo;

    pstrNewLine = pici->ici_strNewLine;
    
    return pstrNewLine;
}

char * getDelimit(const olsize_t u32StrLen, olsize_t u32TotalLen)
{
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olchar_t * pstrDelimit = NULL;
    olsize_t sDelimit;

    if (pici->ici_bSpecialDelimit)
        pstrDelimit = pici->ici_strSpecialDelimit;
    else
    {
        if (u32StrLen > u32TotalLen)
        {
            sDelimit = 1;
        }
        else
        {
            sDelimit = u32TotalLen - u32StrLen;
            if (sDelimit > JF_CLIENG_MAX_OUTPUT_LINE_LEN)
                sDelimit = JF_CLIENG_MAX_OUTPUT_LINE_LEN;
        }

        pstrDelimit = pici->ici_strBlankSpaces + JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1 -
            sDelimit;
    }
    
    return pstrDelimit;
}

void setMoreEnable(void)
{
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    
    pici->ici_bMoreEnabled = TRUE;
    pici->ici_bMoreCancel = FALSE;
    pici->ici_u8MoreLines = 0;
    
}

void setMoreDisable(void)
{
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    
    pici->ici_bMoreEnabled = FALSE;
    pici->ici_bMoreCancel = FALSE;
    pici->ici_u8MoreLines = 0;
    
}

/** Get a key from user
 *
 */
u32 getInputKey(clieng_io_key_t * pcikKey, olchar_t * pstrKey)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olsize_t sInput;
    clieng_input_type_t citInputType = cit_anykey;
    olchar_t strInputKey[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    u32Ret = engioInput(&citInputType, strInputKey, &sInput, 0);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        switch (strInputKey[0])
        {
        case 0x01:
            *pcikKey = cik_save;
            break;
        case 0x04:
            *pcikKey = cik_pgdn;
            break;
        case 0x05:
            *pcikKey = cik_help;
            break;
        case 0x08:
            *pcikKey = cik_backspace;
            break;
        case 0x09:
            *pcikKey = cik_tab;
            break;
        case 0x12:
            *pcikKey = cik_restore;
            break;
        case 0x15:
            *pcikKey = cik_pgup;
            break;
        case 0x18:
            *pcikKey = cik_return;
            break;
        case 0x0A:
        case 0x0D:
            *pcikKey = cik_enter;
            break;
        case 0x1B:
            if (strInputKey[1] == 0x1B)
            {
                *pcikKey = cik_esc;
                break;
            }
            else if (strInputKey[1] == 0x5B || strInputKey[1] == 0x4F)
            {
                /* 0x5B [ */
                switch (strInputKey[2])
                {
                case 0x31:
                    switch (strInputKey[3])
                    {
                    case 0x31:
                        *pcikKey = cik_f1;
                        break;
                    case 0x32:
                        *pcikKey = cik_f2;
                        break;
                    case 0x33:
                        *pcikKey = cik_f3;
                        break;
                    case 0x34:
                        *pcikKey = cik_f4;
                        break;
                    case 0x35:
                        *pcikKey = cik_f5;
                        break;
                    case 0x37:
                        *pcikKey = cik_f6;
                        break;
                    case 0x38:
                        *pcikKey = cik_f7;
                        break;
                    case 0x39:
                        *pcikKey = cik_f8;
                        break;
                    case 0x7e:
                        *pcikKey = cik_home;
                        break;
                    default:
                        *pcikKey = cik_unknown;
                        break;
                    }
                    break;
                case 0x32:
                    switch (strInputKey[3])
                    {
                    case 0x30:
                        *pcikKey = cik_f9;
                        break;
                    case 0x31:
                        *pcikKey = cik_f10;
                        break;
                    case 0x33:
                        *pcikKey = cik_f11;
                        break;
                    case 0x34:
                        *pcikKey = cik_f12;
                        break;
                    case 0x7e:
                        *pcikKey = cik_insert;
                        break;
                    default:
                        *pcikKey = cik_unknown;
                        break;
                    }
                    break;
                case 0x33:
                    *pcikKey = cik_delete;
                    break;
                case 0x34:
                    *pcikKey = cik_end;
                    break;
                case 0x41:
                    *pcikKey = cik_up;
                    break;
                case 0x42:
                    *pcikKey = cik_down;
                    break;
                case 0x43:
                    *pcikKey = cik_right;
                    break;
                case 0x44:
                    *pcikKey = cik_left;
                    break;
                case 0x35:
                case 0x49:
                    *pcikKey = cik_pgup;
                    break;
                case 0x36:
                case 0x47:
                    *pcikKey = cik_pgdn;
                    break;
                default:
                    *pcikKey = cik_unknown;
                    break;
                }
            }
            else
            {
                *pcikKey = cik_unknown;
            }
            break;
        case 0x20:
            *pcikKey = cik_space;
            break;
        case 0x7F:
            *pcikKey = cik_backspace;
//            *pcikKey = cik_delete;
            break;
        default:
            *pcikKey = cik_unknown;
            if (pstrKey != NULL)
                *pstrKey = strInputKey[0];
            break;
        }
    }

    return u32Ret;
}

u32 jf_clieng_outputLine(const olchar_t * fmt, ...)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    va_list vlParam;

    va_start(vlParam, fmt);
    voutputLine(fmt, vlParam);
    va_end(vlParam);

    return u32Ret;
}

u32 jf_clieng_outputRawLine(const olchar_t * line)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olchar_t strBuffer[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];
    olsize_t sLength = 0, sRestOutput = 0;
    olchar_t * pstrLineBegin = 0;

    ol_strncpy(pici->ici_strOutputBuffer, line, MAX_OUTPUT_BUFFER_LEN);

    sLength = ol_strlen(pici->ici_strOutputBuffer);

    pstrLineBegin = pici->ici_strOutputBuffer;
    sRestOutput = sLength;
    if (sRestOutput == 0)
    {
        /* print a empty line */
        sRestOutput = 1;
        pstrLineBegin[0] = ' ';
        pstrLineBegin[1] = 0;
    }
    
    while ((sRestOutput > 0) && (u32Ret == JF_ERR_NO_ERROR))
    {
        /* before output */
        if (pici->ici_bMoreEnabled)
        {
            if (pici->ici_bMoreCancel)
            {
                sRestOutput = 0;
            }
            else
            {
                if (pici->ici_u8MoreLines >= MAX_MORE_LINES)
                {
                    u32Ret = _waitForMore(pici);
                    if (u32Ret == JF_ERR_NO_ERROR)
                    {
                        pici->ici_u8MoreLines = 0;
                    }
                    else if (u32Ret == JF_ERR_MORE_CANCELED)
                    {
                       pici->ici_bMoreCancel = TRUE;
                       sRestOutput = 0;
                       u32Ret = JF_ERR_NO_ERROR;
                    }
                }
            }
        }

        if ((sRestOutput > 0) && (u32Ret == JF_ERR_NO_ERROR))
        {
            if (sRestOutput >= JF_CLIENG_MAX_OUTPUT_LINE_LEN)
            {
                ol_memcpy(strBuffer, pstrLineBegin, JF_CLIENG_MAX_OUTPUT_LINE_LEN);
                sLength = JF_CLIENG_MAX_OUTPUT_LINE_LEN;
            }
            else
            {
                ol_memcpy(strBuffer, pstrLineBegin, sRestOutput);
                sLength = sRestOutput;
            }
            
            pstrLineBegin += sLength;
            sRestOutput -= sLength;
            strBuffer[sLength] = 0;
            sLength = sLength + pici->ici_sNewLine;
            ol_strcat(strBuffer, pici->ici_strNewLine);
            
            u32Ret = _postOutput(pici, cot_text, strBuffer, sLength);
            if (u32Ret == JF_ERR_NO_ERROR)
            {
                pici->ici_u8MoreLines ++;
            }
        }
    }

    return u32Ret;
}

u32 jf_clieng_outputRawLine2(const olchar_t * line)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olsize_t sLength;

    sLength = ol_strlen(line);

    u32Ret = _postOutput(pici, cot_text, line, sLength);
    if (u32Ret == JF_ERR_NO_ERROR)
        _postOutput(pici, cot_text, pici->ici_strNewLine, pici->ici_sNewLine);

    return u32Ret;
}

u32 jf_clieng_reportNotApplicableOpt(const olint_t nOpt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    engioOutput("-%c: not applicable\n", (olchar_t)nOpt);

    return u32Ret;
}

u32 jf_clieng_reportInvalidOpt(const olint_t nOpt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    engioOutput("-%c: invalid option\n", (olchar_t)nOpt);

    return u32Ret;
}

u32 jf_clieng_printBanner(u32 u32Len)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const olchar_t * pstrBanner = NULL;

    if (u32Len > JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1)
        pstrBanner = cls_pstrBanner;
    else
        pstrBanner = cls_pstrBanner + (JF_CLIENG_MAX_OUTPUT_LINE_LEN - u32Len);

    u32Ret = outputLine(pstrBanner);

    return u32Ret;
}

u32 jf_clieng_printBannerShift4(u32 u32Len)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    const olchar_t * pstrBanner = NULL;

    if (u32Len > JF_CLIENG_MAX_OUTPUT_LINE_LEN)
        pstrBanner = cls_pstrBannerShift4;
    else
        pstrBanner = cls_pstrBannerShift4 + (JF_CLIENG_MAX_OUTPUT_LINE_LEN - u32Len);

    u32Ret = outputLine(pstrBanner);

    return u32Ret;
}

u32 jf_clieng_printDivider(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = outputLine(cls_pstrDivider);

    return u32Ret;
}

u32 jf_clieng_printDividerShift2(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = outputLine(cls_pstrDividerShift2);

    return u32Ret;
}

u32 jf_clieng_printHexDumpInByte(u8 * pu8Buffer, u32 u32Len)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0, u32Dumped = 0xff;
    olchar_t strLine[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];

    u32Ret = outputLine(cls_pstrByteHexDumpBegin);

    while ((u32Index < u32Len) && (u32Dumped != 0) &&
           (u32Ret == JF_ERR_NO_ERROR))
    {
        u32Dumped = jf_hex_convertByteDataToString(
            pu8Buffer, u32Len, u32Index, strLine, JF_CLIENG_MAX_OUTPUT_LINE_LEN);
        if (u32Dumped > 0)
        {
            u32Index += u32Dumped;
            u32Ret = outputLine(strLine);
        }
    }
    u32Ret = outputLine(cls_pstrByteHexDumpEnd);

    return u32Ret;
}

u32 jf_clieng_printHeader(const jf_clieng_caption_t * pjcc, u32 u32Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Len;
    olchar_t strCaption[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];
    olchar_t * pstrDelimit;

    /* check the totol length of the caption */
    u32Ret = _checkCaption(pjcc, u32Count);
    if(u32Ret == JF_ERR_NO_ERROR)
    {
        strCaption[0] = '\0';
        u32Len = 0;
        for (u32Index = 0; u32Index < u32Count; u32Index ++)
        {
            ol_strcat(strCaption, pjcc[u32Index].jcc_pstrCaption);
            u32Len += pjcc[u32Index].jcc_u32Len;
            pstrDelimit = getDelimit(ol_strlen(strCaption), u32Len);
            ol_strcat(strCaption, pstrDelimit);
        }

        jf_clieng_printBanner(u32Len);
        outputLine(strCaption);
        jf_clieng_printBanner(u32Len);
    }

    return u32Ret;
}

u32 jf_clieng_printHeaderShift4(
    const jf_clieng_caption_t * pjcc, u32 u32Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index, u32Len;
    olchar_t strCaption[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];
    olchar_t * pstrDelimit;

    /* check the totol length of the caption */
    u32Ret = _checkCaption(pjcc, u32Count);
    if(u32Ret == JF_ERR_NO_ERROR)
    {
        strCaption[0] = '\0';
        u32Len = 0;
        for (u32Index=0; u32Index<u32Count; u32Index++)
        {
            ol_strcat(strCaption, pjcc[u32Index].jcc_pstrCaption);
            u32Len += pjcc[u32Index].jcc_u32Len;
            pstrDelimit = getDelimit(strlen(strCaption), u32Len);
            ol_strcat(strCaption, pstrDelimit);
        }

        jf_clieng_printBannerShift4(u32Len);
        outputLine(strCaption);
        jf_clieng_printBannerShift4(u32Len);
    }

    return u32Ret;
}

u32 jf_clieng_printOneFullLine(
    const jf_clieng_caption_t * pjcc, const olchar_t * pstrValue)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strLine[MAX_OUTPUT_BUFFER_LEN];

    strLine[0] = '\0';

    ol_strcat(strLine, pjcc[0].jcc_pstrCaption);
    ol_strcat(strLine, cls_pstrCaptionDelimit);
    ol_strcat(strLine, pstrValue);

    /* output */
    u32Ret = outputLine("%s", strLine);

    return u32Ret;
}

u32 jf_clieng_printTwoHalfLine(
    const jf_clieng_caption_t * pjcc, const olchar_t * pstrLeft, const olchar_t *pstrRight)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Len = 0;
    olchar_t strLine[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    strLine[0] = '\0';

    /* left half */
    ol_strcat(strLine, pjcc[0].jcc_pstrCaption);
    ol_strcat(strLine, cls_pstrCaptionDelimit);
    u32Len = ol_strlen(strLine);
    strncat(strLine, pstrLeft, (pjcc[0].jcc_u32Len - 1 - u32Len));
    u32Len = ol_strlen(strLine);
    ol_strcat(strLine, getDelimit(u32Len, pjcc[0].jcc_u32Len));

    /* right half */
    ol_strcat(strLine, pjcc[1].jcc_pstrCaption);
    ol_strcat(strLine, cls_pstrCaptionDelimit);
    u32Len = pjcc[1].jcc_u32Len - ol_strlen(pjcc[1].jcc_pstrCaption) -
        ol_strlen(cls_pstrCaptionDelimit);
    strncat(strLine, pstrRight, u32Len);

    /* output */
    u32Ret = outputLine("%s", strLine);

    return u32Ret;
}

void jf_clieng_appendBriefColumn(
    const jf_clieng_caption_t * pjcc, olchar_t * pstrLine, const olchar_t * pstrColumn)
{
    u32 u32Len = ol_strlen(pstrColumn);

    if (u32Len > (pjcc->jcc_u32Len-1))
    {
        u32Len = pjcc->jcc_u32Len - 1;
    }
    strncat(pstrLine, pstrColumn, u32Len);

    ol_strcat(pstrLine, getDelimit(u32Len, pjcc->jcc_u32Len));
}

/*------------------------------------------------------------------------------------------------*/


