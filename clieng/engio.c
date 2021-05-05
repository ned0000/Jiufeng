/**
 *  @file engio.c
 *
 *  @brief The clieng input/output module.
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files -------------------------------------------------------------- */

#include <ctype.h>
#if defined(LINUX)
    #include <termios.h>
    #include <sys/ioctl.h>
#endif

/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"
#include "jf_logger.h"
#include "jf_hex.h"
#include "jf_filestream.h"

#include "engio.h"

/* --- private data/data structure section ------------------------------------------------------ */

/** Delimter between caption and value.
 */
const static olchar_t * cls_pstrCaptionDelimit = ": ";

/** Begin string for hex dump.
 */
const static olchar_t * cls_pstrByteHexDumpBegin = "Begin Hex Dump in Byte";

/** End string for hex dump.
 */
const static olchar_t * cls_pstrByteHexDumpEnd = "End Hex Dump in Byte";

/** Banner string.
 */
const static olchar_t * cls_pstrBanner = "\
====================================================================================================";

/** Divider string.
 */
const static olchar_t * cls_pstrDivider = "\
----------------------------------------------------------------------------------------------------";

/** Banner string with shift 4 spaces.
 */
const static olchar_t * cls_pstrBannerShift4 = "\
    ================================================================================================";

/** Divider string with shift 2 spaces.
 */
const static olchar_t * cls_pstrDividerShift2 = "\
  --------------------------------------------------------------------------------------------------";

/** Define the internal clieng io data type.
 */
typedef struct
{
    /**Clieng io module is initialized if it's TRUE.*/
    boolean_t ici_bInitialized;
    u8 ici_u8Reserved[6];

    /**Special delimit is enabled if it's TRUE. By default it is off, blank spaces are used for
       delimit; when it is on, a semi-column is used. The delimit is used in
       jf_clieng_printTwoHalfLine() and etc.*/
    boolean_t ici_bSpecialDelimit;
    /**The default delimit is ";".*/
    olchar_t ici_strSpecialDelimit[8];

    /**New line.*/
    olchar_t ici_strNewLine[8];
    /**Size of new line.*/
    olsize_t ici_sNewLine;

    /**The line with all blank apaces.*/
    olchar_t ici_strBlankSpaces[JF_CLIENG_MAX_OUTPUT_LINE_LEN];

    /**Output buffer.*/
    olchar_t ici_strOutputBuffer[JF_CLIENG_MAX_OUTPUT_BUFFER_SIZE];

    /**Number of lines for 'more' command.*/
    u8 ici_u8MoreLines;
    u8 ici_u8Reserved2[3];
    /**'more' command is enabled.*/
    boolean_t ici_bMoreEnabled;
    /**'more' should be canceled.*/
    boolean_t ici_bMoreCancel;
    u8 ici_u8Reserved3[2];

    /**File for saving output. By default, the output is written to stdout.*/
    olchar_t * ici_pstrOutputFile;
    /**File stream for saving output.*/
    jf_filestream_t * ici_pjfOutput;
} internal_clieng_io_t;

#ifndef WINDOWS
    #define MAX_MORE_LINES                 _getMaxMoreLines()
#else
    #define MAX_MORE_LINES                (23)
#endif

/** Use LEFT-SPACE-LEFT to implement backspace.
 */
static olchar_t ls_strBackspace[] = {27, '[', 'D', ' ', 27, '[', 'D'};

/** Declare the internal clieng io object.
 */
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
    putchar(ls_strBackspace[0]);
    putchar(ls_strBackspace[1]);
    putchar(ls_strBackspace[2]);
    putchar(ls_strBackspace[3]);
    putchar(ls_strBackspace[4]);
    putchar(ls_strBackspace[5]);
    putchar(ls_strBackspace[6]);
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
    olsize_t length = 0;
    olint_t nChar = 0;

    length = 0;
    do
    {
        nChar = getchar();
        /*Ignore the extra carriage return from windows 2000.*/
        if (nChar == 13)
        {
            ;
        }
        else if (nChar == '\n')
            /*Return.*/
        {
            putchar(nChar);
        } else if (nChar == 127 || nChar == 8)
            /*Backspace.*/
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
    clieng_input_type_t *pType, olchar_t * pBuf, olsize_t * pLen, olsize_t sPrompt)
{
#define MAX_INPUT_LEN (2 * JF_CLIENG_MAX_COMMAND_LINE_LEN)
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

    /*From history command.*/
    if (*pType == cit_navigation_cmd)
    {
        str_length = ol_strlen((olchar_t *)pBuf);
        if (str_length > MAX_INPUT_LEN)
        {
            str_length = MAX_INPUT_LEN;
        }
        for (i = 0; i < str_length; i++)
        {
            putchar(pBuf[i]);
        }
        ol_strncpy(string, (olchar_t *)pBuf, str_length);
        i = str_length;
    }
    if ((str_length + PromptLen) % ColNum == 0)
    {
        putchar(' ');
        _back_space_simple();
    }

    while (nRet != TRUE)
    {
        cur_char[0] = getchar();
        /*Ignore the extra carriage return from windows 2000.*/
        if (cur_char[0] == 13)
        {
            ;
        }
        /*Ignore tab key.*/
        else if (cur_char[0] == 9)
        {
            ;
        }
        else if (cur_char[0] == '\n')
            /*End of command input.*/
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
            /*Escape, function key.*/
        {
            cur_char[1] = getchar();
            cur_char[2] = getchar();
            if (cur_char[2] == 'A' || cur_char[2] == 'B')
            {
                /*Up and Down Arrow.*/
                *pType = cit_navigation_cmd;
                pBuf[0] = cur_char[2];
                pBuf[1] = '\0';
                nRet = TRUE;
                /*Move cursor to end of line.*/
                for(j=i; j<str_length; j++)
                {
                    _right_arrow(j+PromptLen+1, ColNum);
                }
                /*Back_space to begining of line.*/
                for(j=str_length; j>0; j--)
                {
                    _back_space(j+PromptLen, ColNum);
                }
            }
            else if (cur_char[2] == 'D')
                /*Left arrow.*/
            {
                if (i > 0)
                {
                    _left_arrow(i+PromptLen, ColNum);
                    i --;
                }
            }
            else if (cur_char[2] == 'C')
                /*Right arrow.*/
            {
                if (i < str_length)
                {
                    _right_arrow(i+PromptLen+1, ColNum);
                    i ++;
                }
            }
            else if (cur_char[2] == 'F' || cur_char[2] == 52 || cur_char[2] == 53)
                /*End key.*/
            {
                /*Get rid of the extra character from some keyboard.*/
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
            else if (cur_char[2] == 'H' || cur_char[2] == 49 || cur_char[2] == 50)
                /*Home key.*/
            {
                /*Get rid of the extra character from some keyboard.*/
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
            /*Backspace.*/
        {
            if (i > 0)
            {
                _back_space(i+PromptLen, ColNum);
                i--;
                str_length--;
                /*Re-display the rest of the line.*/
                for (j=i; j<str_length; j++)
                {
                    string[j] = string[j+1];
                    putchar(string[j]);
                }
                /*Take care of the last extra character.*/
                putchar(' ');
                _back_space_simple();
                /*Move cursor back to position.*/
                for (j=str_length; j>i; j--)
                {
                    _left_arrow(j+PromptLen, ColNum);
                }
            }
        }
        else /*Insert the character.*/
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
                /*Re-display the line.*/
                for (j=i; j<str_length; j++)
                {
                    putchar(string[j]);
                }
                if ((str_length+PromptLen) % ColNum == 0)
                {
                    putchar(' ');
                    _back_space_simple();
                }
                /*Move cursor back to position.*/
                for (j=str_length; j>i; j--)
                {
                    _left_arrow(j+PromptLen, ColNum);
                }
            }
        }
    } /*While loop.*/
    
    return u32Ret;
}

static u32 _getAnyKey(
    clieng_input_type_t * pType, olchar_t * pBuf, olsize_t *pLen, olsize_t sPrompt)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t cur_char[10];
    olint_t nRet = FALSE;

    while (nRet != TRUE)
    {
        cur_char[0] = getchar();

        if (cur_char[0] == 0x1B)
        {
            /*Escape, function key.*/
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

            ol_memcpy(pBuf, cur_char, *pLen);
            *pType = cit_anykey;
            nRet = TRUE;
        }
        else
        {
            /*Ignore the extra carriage return from windows 2000.*/
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
    } /*While loop.*/

    return u32Ret;
}

static u32 _getInput(
    clieng_input_type_t * pType, olchar_t * pBuf, olsize_t * pLen, olsize_t promptLen)
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
        u32Ret = _postOutput(pici, cot_text, remove_more, ol_strlen(remove_more));

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (strInputKey[0] == 0x18) /* CTRL- X */
        {
            u32Ret = _postOutput(pici, cot_text, enter_line, ol_strlen(enter_line));
            u32Ret = JF_ERR_MORE_CANCELED;
        }
    }

    return u32Ret;
}

/** Check whether the caption list is valid for printing.
 *
 *  @param pjcc [in] The pointer to the caption list.
 *  @param u32Count [in] The caption count in the caption list.
 *
 *  @return the error code
 */
static u32 _checkCaption(const jf_clieng_caption_t * pjcc, u32 u32Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0, u32Length = 0;

    /*Calculate the total length of the line.*/
    for (u32Index = 0; u32Index < u32Count; u32Index++)
    {
        u32Length += pjcc[u32Index].jcc_u32Len;
    }

    /*Check the total length.*/
    if (u32Length > JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1)
    {
        u32Ret = JF_ERR_LINE_TOO_LONG;
    }

    return u32Ret;
}

static u32 _outputLineInEngIo(internal_clieng_io_t * pici, olchar_t * pstrOutput, olsize_t sOutput)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strBuffer[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];
    olsize_t sLength = 0, sRestOutput = 0;
    olchar_t * pstrLineBegin = 0;

    pstrLineBegin = pstrOutput;
    sRestOutput = sOutput;
    if (sRestOutput == 0)
    {
        /*Print a empty line.*/
        sRestOutput = 1;
        pstrLineBegin[0] = ' ';
        pstrLineBegin[1] = '\0';
    }

    while ((sRestOutput > 0) && (u32Ret == JF_ERR_NO_ERROR))
    {
        /*More is enabled.*/
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

static u32 _openOutputForEngIo(internal_clieng_io_t * pici, clieng_io_param_t * pcip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    if (pcip->cip_pstrOutputFile == NULL)
        u32Ret = JF_ERR_INVALID_PARAM;

    if (u32Ret == JF_ERR_NO_ERROR)
        /*Use append mode, incase script engine is enabled.*/
        u32Ret = jf_filestream_open(pcip->cip_pstrOutputFile, "a", &pici->ici_pjfOutput);

    /*Save the output file name.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        pici->ici_pstrOutputFile = pcip->cip_pstrOutputFile;

    /*Use stdout incase error.*/
    if (u32Ret != JF_ERR_NO_ERROR)
    {
        pici->ici_pjfOutput = stdout;
        u32Ret = JF_ERR_NO_ERROR;
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

    JF_LOGGER_DATA((u8 *)pcip->cip_pstrNewLine, ol_strlen(pcip->cip_pstrNewLine), "newline: ");

    /*Initialize the clieng io object.*/
    ol_bzero(pici, sizeof(internal_clieng_io_t));
    pici->ici_bSpecialDelimit = FALSE;
    pici->ici_strSpecialDelimit[0] = ';';
    pici->ici_strSpecialDelimit[1] = 0;
    ol_strcpy(pici->ici_strNewLine, pcip->cip_pstrNewLine);
    pici->ici_sNewLine = ol_strlen(pici->ici_strNewLine);
    ol_memset(pici->ici_strBlankSpaces, ' ', JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1);
    pici->ici_strBlankSpaces[JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1] = '\0';

    setMoreDisable();

#ifndef WINDOWS
    u32Ret = _switchToRawMode(nTTY);
#endif

    /*Open output stream.*/
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _openOutputForEngIo(pici, pcip);

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
#endif
    JF_LOGGER_INFO("fini");

#ifndef WINDOWS
    _switchToCanonMode(nTTY);
#endif

    /*Close the output stream.*/
    if (pici->ici_pstrOutputFile != NULL)
        jf_filestream_close(&pici->ici_pjfOutput);

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
    olsize_t sLength = 0;

    assert(fmt != NULL);

    sLength = ol_vsnprintf(pici->ici_strOutputBuffer, JF_CLIENG_MAX_OUTPUT_BUFFER_SIZE, fmt, ap);

    u32Ret = _outputLineInEngIo(pici, pici->ici_strOutputBuffer, sLength);

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

olchar_t * getEOL(void)
{
    olchar_t * pstrNewLine = NULL;
    internal_clieng_io_t * pici = &ls_iciCliengIo;

    pstrNewLine = pici->ici_strNewLine;
    
    return pstrNewLine;
}

olchar_t * getDelimit(const olsize_t u32StrLen, olsize_t u32TotalLen)
{
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olchar_t * pstrDelimit = NULL;
    olsize_t sDelimit = 0;

    if (pici->ici_bSpecialDelimit)
    {
        /*Return special delimit directly if it's enabled.*/
        pstrDelimit = pici->ici_strSpecialDelimit;
    }
    else
    {
        /*Calculate the length of delimit.*/
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

        /*Only return blank space with specified length, from the end of the buffer.*/
        pstrDelimit = pici->ici_strBlankSpaces + JF_CLIENG_MAX_OUTPUT_LINE_LEN - 1 - sDelimit;
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

u32 jf_clieng_outputRawLine(olchar_t * line)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    internal_clieng_io_t * pici = &ls_iciCliengIo;
    olsize_t sLength = 0;

    sLength = ol_strlen(line);

    u32Ret = _outputLineInEngIo(pici, line, sLength);

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
        pstrBanner = cls_pstrBanner + (u32Len - JF_CLIENG_MAX_OUTPUT_LINE_LEN);
    else
        pstrBanner = cls_pstrBanner;

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

    /*Check the totol length of the caption.*/
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

u32 jf_clieng_printHeaderShift4(const jf_clieng_caption_t * pjcc, u32 u32Count)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    u32 u32Index = 0, u32Len = 0;
    olchar_t strCaption[JF_CLIENG_MAX_OUTPUT_LINE_LEN + 8];
    olchar_t * pstrDelimit = NULL;

    /*Check the totol length of the caption.*/
    u32Ret = _checkCaption(pjcc, u32Count);

    if(u32Ret == JF_ERR_NO_ERROR)
    {
        strCaption[0] = '\0';
        u32Len = 0;
        for (u32Index = 0; u32Index < u32Count; u32Index++)
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
    olchar_t strLine[JF_CLIENG_MAX_OUTPUT_BUFFER_SIZE];

    strLine[0] = '\0';

    ol_strcat(strLine, pjcc[0].jcc_pstrCaption);
    ol_strcat(strLine, cls_pstrCaptionDelimit);
    ol_strcat(strLine, pstrValue);

    /*Output line.*/
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

    /*Left half.*/
    ol_strcat(strLine, pjcc[0].jcc_pstrCaption);
    ol_strcat(strLine, cls_pstrCaptionDelimit);
    u32Len = ol_strlen(strLine);
    strncat(strLine, pstrLeft, (pjcc[0].jcc_u32Len - 1 - u32Len));
    u32Len = ol_strlen(strLine);
    ol_strcat(strLine, getDelimit(u32Len, pjcc[0].jcc_u32Len));

    /*Right half.*/
    ol_strcat(strLine, pjcc[1].jcc_pstrCaption);
    ol_strcat(strLine, cls_pstrCaptionDelimit);
    u32Len = pjcc[1].jcc_u32Len - ol_strlen(pjcc[1].jcc_pstrCaption) -
        ol_strlen(cls_pstrCaptionDelimit);
    strncat(strLine, pstrRight, u32Len);

    /*Output line.*/
    u32Ret = outputLine("%s", strLine);

    return u32Ret;
}

void jf_clieng_appendBriefColumn(
    const jf_clieng_caption_t * pjcc, olchar_t * pstrLine, const olchar_t * pstrColumn)
{
    u32 u32Len = ol_strlen(pstrColumn);

    if (u32Len > (pjcc->jcc_u32Len - 1))
    {
        /*Length of the column is larger than expected and is truncated.*/
        u32Len = pjcc->jcc_u32Len - 1;
    }
    /*Copy column to the line buffer.*/
    ol_strncat(pstrLine, pstrColumn, u32Len);

    /*Copy delimit.*/
    ol_strcat(pstrLine, getDelimit(u32Len, pjcc->jcc_u32Len));
}

/*------------------------------------------------------------------------------------------------*/
