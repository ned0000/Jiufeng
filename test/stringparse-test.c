/**
 *  @file stringparse-test.c
 *
 *  @brief The test file for stringparse library
 *
 *  @author Min Zhang
 *
 *  @note
 *  
 */

/* --- standard C lib header files ----------------------------------------- */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "ollimit.h"
#include "bases.h"
#include "errcode.h"
#include "stringparse.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: stringparse-test [-l] [-c error code] \n\
    [-l]: test the logger. \n\
    [-c error code]: prolint_t error message for the error coce.\n");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;

    while (((nOpt = getopt(argc, argv,
        "h?")) != -1) && (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(u32Ret);
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static void _testStringparse(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
	char * sdb = "226303136636.85";
	char * sdb2 = "2298363628138.857";
	char * sdb3 = "230189685431.55";
	oldouble_t db;

	u32Ret = getDoubleFromString(sdb, ol_strlen(sdb), &db);
	if (u32Ret == OLERR_NO_ERROR)
	{
		printf("%s, %.2f\n", sdb, db);
	}

	u32Ret = getDoubleFromString(sdb2, ol_strlen(sdb2), &db);
	if (u32Ret == OLERR_NO_ERROR)
	{
		printf("%s, %.2f\n", sdb2, db);
	}

	u32Ret = getDoubleFromString(sdb3, ol_strlen(sdb3), &db);
	if (u32Ret == OLERR_NO_ERROR)
	{
		printf("%s, %.2f\n", sdb3, db);
	}

}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _parseCmdLineParam(argc, argv);
    if (u32Ret == OLERR_NO_ERROR)
    {
		_testStringparse();
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


