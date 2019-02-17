/**
 *  @file matrix-test.c
 *
 *  @brief The test file for matrix library
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
#include "matrix.h"

/* --- private data/data structure section --------------------------------- */


/* --- private routine section---------------------------------------------- */

static void _printUsage(void)
{
    ol_printf("\
Usage: matrix-test [-h] [logger options]\n\
    -h show this usage.\n\
logger options:\n\
    -T <0|1|2|3|4> the log level. 0: no log, 1: error, 2: info, 3: debug,\n\
       4: data.\n\
    -F <log file> the log file.\n\
    -S <trace file size> the size of log file. No limit if not specified.\n");

    ol_printf("\n");
}

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv, logger_param_t * plp)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "tsOT:F:S:h")) != -1) &&
           (u32Ret == OLERR_NO_ERROR))
    {
        switch (nOpt)
        {
        case '?':
        case 'h':
            _printUsage();
            exit(u32Ret);
            break;
        case 'T':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                plp->lp_u8TraceLevel = (u8)u32Value;
            else
                u32Ret = OLERR_INVALID_PARAM;
            break;
        case 'F':
            plp->lp_bLogToFile = TRUE;
            plp->lp_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                plp->lp_sLogFile = u32Value;
            else
                u32Ret = OLERR_INVALID_PARAM;
            break;
        case 'O':
            plp->lp_bLogToStdout = TRUE;
            break;
        default:
            u32Ret = OLERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testMulMatrix(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    matrix_t * pma = NULL, * pmb = NULL, * pmc = NULL;
    oldouble_t dbdata = 1.0;
    olint_t i;

    u32Ret = allocMatrix(4, 5, &pma);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(5, 3, &pmb);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(4, 3, &pmc);

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < pma->m_nRow * pma->m_nCol; i ++)
        {
            pma->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix A\n");
        printMatrix(pma);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < pmb->m_nRow * pmb->m_nCol; i ++)
        {
            pmb->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix B\n");
        printMatrix(pmb);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Matrix A * B\n");
        mulMatrix(pmc, pma, pmb);
        printMatrix(pmc);
    }


    if (pma == NULL)
        freeMatrix(&pma);
    if (pmb == NULL)
        freeMatrix(&pmb);
    if (pmc == NULL)
        freeMatrix(&pmc);

    return u32Ret;
}

static u32 _testAddSubMatrix(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    matrix_t * pma = NULL, * pmb = NULL;
    oldouble_t dbdata = 1.0;
    olint_t i;

    u32Ret = allocMatrix(3, 3, &pma);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(3, 3, &pmb);

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < pma->m_nRow * pma->m_nCol; i ++)
        {
            pma->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix A\n");
        printMatrix(pma);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < pmb->m_nRow * pmb->m_nCol; i ++)
        {
            pmb->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix B\n");
        printMatrix(pmb);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Matrix A + B\n");
        addMatrix(pma, pmb);
        printMatrix(pma);

        ol_printf("Matrix A\n");
        printMatrix(pma);

        ol_printf("Matrix B\n");
        printMatrix(pmb);

        ol_printf("Matrix A - B\n");
        subMatrix(pma, pmb);
        printMatrix(pma);
    }


    if (pma == NULL)
        freeMatrix(&pma);
    if (pmb == NULL)
        freeMatrix(&pmb);

    return u32Ret;
}

static u32 _testTransposeMatrix(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    matrix_t * pma = NULL, * pmb = NULL;
    oldouble_t dbdata = 1.0;
    olint_t i;

    u32Ret = allocMatrix(4, 5, &pma);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(5, 4, &pmb);

    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < pma->m_nRow * pma->m_nCol; i ++)
        {
            pma->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix A\n");
        printMatrix(pma);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Transpose(A)\n");
        transposeMatrix(pmb, pma);
        printMatrix(pmb);
    }

    if (pma == NULL)
        freeMatrix(&pma);
    if (pmb == NULL)
        freeMatrix(&pmb);

    return u32Ret;
}

static u32 _testInverseMatrix(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    matrix_t ma;
    matrix_t * pma = &ma, * pmb = NULL, * pmc = NULL;
//    olfloat_t madata[3 * 3] = {-3, 2, -5, -1, 0, -2, 3, -4, 1};
    oldouble_t madata[3 * 3] = {1, 4, 7, 3, 0, 5, -1, 9, 11};
    oldouble_t dbdata = 1.0;

    initMatrix(pma, 3, 3, madata);

    u32Ret = allocMatrix(3, 3, &pmb);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(3, 3, &pmc);

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Matrix A\n");
        printMatrix(pma);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        u32Ret = getMatrixDeterminant(pma, &dbdata);
        if (u32Ret == OLERR_NO_ERROR)
        {
            ol_printf("Determinant of Matrix A: %.3f\n\n", dbdata);

            ol_printf("Adjugate(A)\n");
            u32Ret = adjugateMatrix(pmb, pma);
            if (u32Ret == OLERR_NO_ERROR)
                printMatrix(pmb);
        }
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Inverse(A)\n");
        u32Ret = inverseMatrix(pmb, pma);
        if (u32Ret == OLERR_NO_ERROR)
        {
            printMatrix(pmb);

            mulMatrix(pmc, pma, pmb);
            ol_printf("A * Inverse(A)\n");
            printMatrix(pmc);

            mulMatrix(pmc, pmb, pma);
            ol_printf("Inverse(A) * A\n");
            printMatrix(pmc);
        }
    }

    if (pmb == NULL)
        freeMatrix(&pmb);
    if (pmc == NULL)
        freeMatrix(&pmc);

    return u32Ret;
}

static u32 _testHatMatrix(void)
{
    u32 u32Ret = OLERR_NO_ERROR;
    matrix_t ma;
    matrix_t * pma = &ma, * pmb = NULL;
    oldouble_t madata[11 * 5] =
        {1, 394, 848, 2027, 1548,
         1, 126, 137, 876, 1117,
         1, 144, 487, 1091, 1544,
         1, 875, 426, 1615, 1307,
         1, 629, 582, 1520, 1800,
         1, 3237, 1048, 3664, 2894,
         1, 2603, 1382, 2297, 2055,
         1, 2637, 1158, 1952, 1877,
         1, 3174, 1098, 1935, 1709,
         1, 3585, 966, 1798, 1682,
         1, 1102, 1273, 2231, 1759,
        };

    initMatrix(pma, 11, 5, madata);

    u32Ret = allocMatrix(11, 11, &pmb);

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Matrix A\n");
        printMatrix(pma);

        u32Ret = hatMatrix(pmb, pma);
    }

    if (u32Ret == OLERR_NO_ERROR)
    {
        ol_printf("Matrix B\n");
        printMatrix(pmb);
    }

    if (pmb == NULL)
        freeMatrix(&pmb);

    return u32Ret;
}

static u32 _testMatrix(void)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = _testAddSubMatrix();
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _testMulMatrix();

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _testTransposeMatrix();

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _testInverseMatrix();

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _testHatMatrix();

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olchar_t strErrMsg[300];
    logger_param_t lp;

    memset(&lp, 0, sizeof(lp));

    u32Ret = _parseCmdLineParam(argc, argv, &lp);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = _testMatrix();

    if (u32Ret != OLERR_NO_ERROR)
    {
        getErrMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


