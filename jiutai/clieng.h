/**
 *  @file clieng.h
 *
 *  @brief CLI Engine header file. Define the interfaces and data structures
 *   exposed by the CLI engine
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

#ifndef JIUFENG_CLIENG_H
#define JIUFENG_CLIENG_H

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <time.h>

/* --- internal header files ----------------------------------------------- */
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"

#undef CLIENGAPI
#undef CLIENGCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_CLIENG_DLL)
        #define CLIENGAPI  __declspec(dllexport)
        #define CLIENGCALL
    #else
        #define CLIENGAPI
        #define CLIENGCALL __cdecl
    #endif
#else
    #define CLIENGAPI
    #define CLIENGCALL
#endif

/* --- constants and data structures --------------------------------------- */

typedef void  jf_clieng_cmd_t;
typedef void  jf_clieng_cmd_set_t;

#define JF_CLIENG_MAX_OUTPUT_LINE_LEN      (80)

#define JF_CLIENG_MAX_COMMAND_LINE_SIZE    (280)

#define JF_CLIENG_MAX_CLI_NAME_LEN         (32)

typedef u32 (* jf_clieng_fnPrintGreeting_t)(void * pMaster);
typedef u32 (* jf_clieng_fnPreEnterLoop_t)(void * pMaster);
typedef u32 (* jf_clieng_fnPostExitLoop_t)(void * pMaster);

typedef struct
{
    /** Max number of characters for one command */
    olsize_t jcip_sMaxCmdLine;
    /** Number of commands in command history buffer */
    olsize_t jcip_sCmdHistroyBuf;
    /** cli name */
    olchar_t jcip_strCliName[JF_CLIENG_MAX_CLI_NAME_LEN];
    jf_clieng_fnPrintGreeting_t jcip_fnPrintGreeting;
    jf_clieng_fnPreEnterLoop_t jcip_fnPreEnterLoop;
    jf_clieng_fnPostExitLoop_t jcip_fnPostExitLoop;
    void * jcip_pMaster;
    /** New line string. e.g. "\n" or "\r\n" */
    olchar_t * jcip_pstrNewLine;
    u32 jcip_u32MaxCmdSet;
    boolean_t jcip_bEnableScriptEngine;
    u8 jcip_u8Reserved[3];
    FILE * jcip_fhOutput;
    olchar_t  jcip_strInputCmd[JF_CLIENG_MAX_COMMAND_LINE_SIZE];
} jf_clieng_init_param_t;

typedef struct
{
    /* the length of cc_pstrCaption must be smaller than cc_u32Length */
    olchar_t * jcc_pstrCaption;
    u32 jcc_u32Len;
} jf_clieng_caption_t;

#define JF_CLIENG_CAP_FULL_LINE  (JF_CLIENG_MAX_OUTPUT_LINE_LEN)
#define JF_CLIENG_CAP_HALF_LINE  (JF_CLIENG_MAX_OUTPUT_LINE_LEN / 2)

typedef u32 (* jf_clieng_fnSetDefaultParam_t)(void * pMaster, void * pParam);
typedef u32 (* jf_clieng_fnParseCmd_t)(
    void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);
typedef u32 (* jf_clieng_fnProcessCmd_t)(void * pMaster, void * pParam);

/* --- functional routines ------------------------------------------------- */

CLIENGAPI u32 CLIENGCALL jf_clieng_init(jf_clieng_init_param_t * pParam);

/** Run CLI engine.
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_run(void);

/** Stop CLI engine
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_stop(void);

CLIENGAPI u32 CLIENGCALL jf_clieng_fini(void);

CLIENGAPI u32 CLIENGCALL jf_clieng_newCmdSet(
    const olchar_t * pstrName, jf_clieng_cmd_set_t ** ppCmdSet);

CLIENGAPI u32 CLIENGCALL jf_clieng_newCmd(
    const olchar_t * pstrName, jf_clieng_fnSetDefaultParam_t fnSetDefaultParam,
    jf_clieng_fnParseCmd_t fnParseCmd, jf_clieng_fnProcessCmd_t fnProcessCmd,
    void * pParam, jf_clieng_cmd_t ** ppCmd);

CLIENGAPI u32 CLIENGCALL jf_clieng_addToCmdSet(
    jf_clieng_cmd_t * pCmd, jf_clieng_cmd_set_t * pCmdSet);

CLIENGAPI u32 CLIENGCALL jf_clieng_outputLine(const olchar_t * fmt, ...);

CLIENGAPI u32 CLIENGCALL jf_clieng_outputRawLine(const olchar_t * line);

/** Output "line" with new line 
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_outputRawLine2(const olchar_t * line);

CLIENGAPI u32 CLIENGCALL jf_clieng_reportNotApplicableOpt(const olint_t nOpt);

CLIENGAPI u32 CLIENGCALL jf_clieng_reportInvalidOpt(const olint_t nOpt);

CLIENGAPI u32 CLIENGCALL jf_clieng_setPrompt(const olchar_t * pstrPrompt);

/** Print the banner of the header bar for brief printing.
 *
 *  @param u32Len [in] the desired length of the banner in characters.
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printBanner(u32 u32Len);

CLIENGAPI u32 CLIENGCALL jf_clieng_printBannerShift4(u32 u32Len);

/** Print the devider between two manageable elements for verbose printing.
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printDivider(void);

CLIENGAPI u32 CLIENGCALL jf_clieng_printDividerShift2(void);

/** Print the hex dump of a data buffer byte by byte
 *
 *  @param pu8Buffer [in] the pointer to the data buffer
 *  @param u32Len [in] the length of the data buffer in bytes
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printHexDumpInByte(
    u8 * pu8Buffer, u32 u32Len);

/** Print the header bar according to the caption list for brief printing
 *
 *  @param pjcc [in] the pointer to the caption list
 *  @param u32Count [in] the count of the captions in the caption list
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printHeader(
    const jf_clieng_caption_t * pjcc, u32 u32Count);

CLIENGAPI u32 CLIENGCALL jf_clieng_printHeaderShift4(
    const jf_clieng_caption_t * pjcc, u32 u32Count);

/** Print one attribute in a line for verbose printing
 *
 *  @param pjcc [in] the pointer to the caption of the attribute
 *  @param pstrValue [in] the value of the attribute
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printOneFullLine(
    const jf_clieng_caption_t * pjcc, const olchar_t * pstrValue);

/** Print two attributes in a line for verbose printing
 *
 *  @param pjcc [in] the pointer to the captions of the two attributes
 *  @param pstrLeft [in] the value of the attribute to be printed at the left
 *  @param pstrRight [in] the value of the attribute at the right
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL jf_clieng_printTwoHalfLine(
    const jf_clieng_caption_t * pjcc, const olchar_t * pstrLeft,
    const olchar_t *pstrRight);

/** Append brief column to the line
 *
 *  @param pjcc [in] the pointer to the caption of the column
 *  @param pstrLine [in/out] the line to append to
 *  @param pstrColumn [in] the value of the column
 */
CLIENGAPI void CLIENGCALL jf_clieng_appendBriefColumn(
    const jf_clieng_caption_t * pjcc, olchar_t * pstrLine,
    const olchar_t * pstrColumn);

#endif /*JIUFENG_CLIEN_H*/

/*---------------------------------------------------------------------------*/

