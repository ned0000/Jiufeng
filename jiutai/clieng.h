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
#include "olbasic.h"
#include "ollimit.h"
#include "errcode.h"

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

typedef void  clieng_cmd_t;
typedef void  clieng_cmd_set_t;

#define MAX_OUTPUT_LINE_LEN    (80)

#define MAX_COMMAND_LINE_SIZE  (280)

#define MAX_CLI_NAME_LEN       (32)

typedef u32 (* fnPrintGreeting_t)(void * pMaster);
typedef u32 (* fnPreEnterLoop_t)(void * pMaster);
typedef u32 (* fnPostExitLoop_t)(void * pMaster);

typedef struct
{
    /** Max number of characters for one command */
    olsize_t cp_sMaxCmdLine;
    /** Number of commands in command history buffer */
    olsize_t cp_sCmdHistroyBuf;
    /** cli name */
    olchar_t cp_strCliName[MAX_CLI_NAME_LEN];
    fnPrintGreeting_t cp_fnPrintGreeting;
    fnPreEnterLoop_t cp_fnPreEnterLoop;
    fnPostExitLoop_t cp_fnPostExitLoop;
    void * cp_pMaster;
    /** New line string. e.g. "\n" or "\r\n" */
    olchar_t * cp_pstrNewLine;
    u32 cp_u32MaxCmdSet;
    boolean_t cp_bEnableScriptEngine;
    u8 cp_u8Reserved[3];
    FILE * cp_fhOutput;
    olchar_t  cp_strInputCmd[MAX_COMMAND_LINE_SIZE];
} clieng_param_t;

typedef struct
{
    /* the length of cc_pstrCaption must be smaller than cc_u32Length */
    olchar_t * cc_pstrCaption;
    u32 cc_u32Len;
} clieng_caption_t;

#define CLIENG_CAP_FULL_LINE  MAX_OUTPUT_LINE_LEN
#define CLIENG_CAP_HALF_LINE  (MAX_OUTPUT_LINE_LEN / 2)

/* --- functional routines ------------------------------------------------- */

CLIENGAPI u32 CLIENGCALL initClieng(clieng_param_t * pParam);

/** Run CLI engine.
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL runClieng(void);

/** Stop CLI engine
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL stopClieng(void);

CLIENGAPI u32 CLIENGCALL finiClieng(void);

CLIENGAPI u32 CLIENGCALL cliengNewCmdSet(
    const olchar_t * pstrName, clieng_cmd_set_t ** ppCmdSet);

typedef u32 (* fnSetDefaultParam_t)(void * pMaster, void * pParam);
typedef u32 (* fnParseCmd_t)(
    void * pMaster, olint_t argc, olchar_t ** argv, void * pParam);
typedef u32 (* fnProcessCmd_t)(void * pMaster, void * pParam);

CLIENGAPI u32 CLIENGCALL cliengNewCmd(
    const olchar_t * pstrName, fnSetDefaultParam_t fnSetDefaultParam,
    void * pParam, fnParseCmd_t fnParseCmd, fnProcessCmd_t fnProcessCmd,
    clieng_cmd_t ** ppCmd);

CLIENGAPI u32 CLIENGCALL cliengAddToCmdSet(
    clieng_cmd_t * pCmd, clieng_cmd_set_t * pCmdSet);

CLIENGAPI u32 CLIENGCALL cliengOutputLine(const olchar_t * fmt, ...);

CLIENGAPI u32 CLIENGCALL cliengOutputRawLine(const olchar_t * line);

/** Output "line" with new line 
 */
CLIENGAPI u32 CLIENGCALL cliengOutputRawLine2(const olchar_t * line);

CLIENGAPI u32 CLIENGCALL cliengReportNotApplicableOpt(const olint_t nOpt);

CLIENGAPI u32 CLIENGCALL cliengReportInvalidOpt(const olint_t nOpt);

CLIENGAPI u32 CLIENGCALL cliengSetPrompt(const olchar_t * pstrPrompt);

/** Print the banner of the header bar for brief printing.
 *
 *  @param u32Len [in] the desired length of the banner in characters.
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL cliengPrintBanner(u32 u32Len);

CLIENGAPI u32 CLIENGCALL cliengPrintBannerShift4(u32 u32Len);

/** Print the devider between two manageable elements for verbose printing.
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL cliengPrintDivider(void);

CLIENGAPI u32 CLIENGCALL cliengPrintDividerShift2(void);

/** Print the hex dump of a data buffer byte by byte
 *
 *  @param pu8Buffer [in] the pointer to the data buffer
 *  @param u32Len [in] the length of the data buffer in bytes
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL cliengPrintHexDumpInByte(u8 * pu8Buffer, u32 u32Len);

/** Print the header bar according to the caption list for brief printing
 *
 *  @param pcc [in] the pointer to the caption list
 *  @param u32Count [in] the count of the captions in the caption list
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL cliengPrintHeader(
    const clieng_caption_t * pcc, u32 u32Count);

CLIENGAPI u32 CLIENGCALL cliengPrintHeaderShift4(
    const clieng_caption_t * pcc, u32 u32Count);

/** Print one attribute in a line for verbose printing
 *
 *  @param pcc [in] the pointer to the caption of the attribute
 *  @param pstrValue [in] the value of the attribute
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL cliengPrintOneFullLine(
    const clieng_caption_t * pcc, const olchar_t * pstrValue);

/** Print two attributes in a line for verbose printing
 *
 *  @param pcc [in] the pointer to the captions of the two attributes
 *  @param pstrLeft [in] the value of the attribute to be printed at the left
 *  @param pstrRight [in] the value of the attribute at the right
 *
 *  @return the error code
 */
CLIENGAPI u32 CLIENGCALL cliengPrintTwoHalfLine(
    const clieng_caption_t * pcc, const olchar_t * pstrLeft,
    const olchar_t *pstrRight);

/** Append brief column to the line
 *
 *  @param pcc [in] the pointer to the caption of the column
 *  @param pstrLine [in/out] the line to append to
 *  @param pstrColumn [in] the value of the column
 */
CLIENGAPI void CLIENGCALL cliengAppendBriefColumn(
    const clieng_caption_t * pcc, olchar_t * pstrLine,
    const olchar_t * pstrColumn);

#endif /*JIUFENG_CLIEN_H*/

/*---------------------------------------------------------------------------*/

