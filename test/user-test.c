/**
 *  @file user-test.c
 *
 *  @brief Test file for user function defined in jf_user common object.
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_err.h"
#include "jf_user.h"
#include "jf_option.h"

/* --- private data/data structure section ------------------------------------------------------ */

static boolean_t ls_bShowUserInfo = FALSE;

static olchar_t * ls_pstrUserName = NULL;

/* --- private routine section ------------------------------------------------------------------ */

static void _printUserTestUsage(void)
{
    ol_printf("\
Usage: user-test [-s] [-u user-name] [-h] \n\
  -s: show user information.\n\
  -u: specify the user name.\n\
  -h: show this help information.\n\
         \n");
    ol_printf("\n");
}

static u32 _parseUserTestCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;

    while ((u32Ret == JF_ERR_NO_ERROR) && ((nOpt = jf_option_get(argc, argv, "su:h?")) != -1))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUserTestUsage();
            exit(0);
        case 's':
            ls_bShowUserInfo = TRUE;
            break;
        case 'u':
            ls_pstrUserName = jf_option_getArg();
            break;
        case ':':
            u32Ret = JF_ERR_MISSING_PARAM;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _showUserInfo()
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    uid_t uid;
    gid_t gid;

    if (ls_pstrUserName == NULL)
    {
        ol_printf("Username is not specified.\n");
        u32Ret = JF_ERR_MISSING_PARAM;
    }

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_user_getUidGid(ls_pstrUserName, &uid, &gid);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Username: %s\n", ls_pstrUserName);
        ol_printf("Uid: %d, Gid: %d\n", uid, gid);
    }

    return u32Ret;
}

/* --- public routine section ------------------------------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];

    u32Ret = _parseUserTestCmdLineParam(argc, argv);
    if (u32Ret == JF_ERR_NO_ERROR)
    {
        if (ls_bShowUserInfo)
        {
            u32Ret = _showUserInfo();
        }
        else
        {
            ol_printf("No operation is specified !!!!\n\n");
            _printUserTestUsage();
        }
    }

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_readDescription(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*------------------------------------------------------------------------------------------------*/
