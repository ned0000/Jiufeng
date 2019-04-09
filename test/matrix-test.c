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
#include "jf_basic.h"
#include "jf_limit.h"
#include "jf_matrix.h"

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

static u32 _parseCmdLineParam(olint_t argc, olchar_t ** argv, jf_logger_init_param_t * pjlip)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olint_t nOpt;
    u32 u32Value;

    while (((nOpt = getopt(argc, argv, "tsOT:F:S:h")) != -1) &&
           (u32Ret == JF_ERR_NO_ERROR))
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
                pjlip->jlip_u8TraceLevel = (u8)u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        case 'F':
            pjlip->jlip_bLogToFile = TRUE;
            pjlip->jlip_pstrLogFilePath = optarg;
            break;
        case 'S':
            if (sscanf(optarg, "%d", &u32Value) == 1)
                pjlip->jlip_sLogFile = u32Value;
            else
                u32Ret = JF_ERR_INVALID_PARAM;
            break;
        case 'O':
            pjlip->jlip_bLogToStdout = TRUE;
            break;
        default:
            u32Ret = JF_ERR_INVALID_OPTION;
            break;
        }
    }

    return u32Ret;
}

static u32 _testMulMatrix(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    matrix_t * pma = NULL, * pmb = NULL, * pmc = NULL;
    oldouble_t dbdata = 1.0;
    olint_t i;

    u32Ret = jf_matrix_alloc(4, 5, &pma);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_matrix_alloc(5, 3, &pmb);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_matrix_alloc(4, 3, &pmc);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < pma->m_nRow * pma->m_nCol; i ++)
        {
            pma->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix A\n");
        jf_matrix_print(pma);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < pmb->m_nRow * pmb->m_nCol; i ++)
        {
            pmb->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix B\n");
        jf_matrix_print(pmb);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Matrix A * B\n");
        jf_matrix_mul(pmc, pma, pmb);
        jf_matrix_print(pmc);
    }


    if (pma == NULL)
        jf_matrix_free(&pma);
    if (pmb == NULL)
        jf_matrix_free(&pmb);
    if (pmc == NULL)
        jf_matrix_free(&pmc);

    return u32Ret;
}

static u32 _testAddSubMatrix(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    matrix_t * pma = NULL, * pmb = NULL;
    oldouble_t dbdata = 1.0;
    olint_t i;

    u32Ret = jf_matrix_alloc(3, 3, &pma);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_matrix_alloc(3, 3, &pmb);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < pma->m_nRow * pma->m_nCol; i ++)
        {
            pma->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix A\n");
        jf_matrix_print(pma);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < pmb->m_nRow * pmb->m_nCol; i ++)
        {
            pmb->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix B\n");
        jf_matrix_print(pmb);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Matrix A + B\n");
        jf_matrix_add(pma, pmb);
        jf_matrix_print(pma);

        ol_printf("Matrix A\n");
        jf_matrix_print(pma);

        ol_printf("Matrix B\n");
        jf_matrix_print(pmb);

        ol_printf("Matrix A - B\n");
        jf_matrix_sub(pma, pmb);
        jf_matrix_print(pma);
    }


    if (pma == NULL)
        jf_matrix_free(&pma);
    if (pmb == NULL)
        jf_matrix_free(&pmb);

    return u32Ret;
}

static u32 _testTransposeMatrix(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    matrix_t * pma = NULL, * pmb = NULL;
    oldouble_t dbdata = 1.0;
    olint_t i;

    u32Ret = jf_matrix_alloc(4, 5, &pma);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_matrix_alloc(5, 4, &pmb);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        for (i = 0; i < pma->m_nRow * pma->m_nCol; i ++)
        {
            pma->m_pdbData[i] = dbdata;
            dbdata += 1.0;
        }
        ol_printf("Matrix A\n");
        jf_matrix_print(pma);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Transpose(A)\n");
        jf_matrix_transpose(pmb, pma);
        jf_matrix_print(pmb);
    }

    if (pma == NULL)
        jf_matrix_free(&pma);
    if (pmb == NULL)
        jf_matrix_free(&pmb);

    return u32Ret;
}

static u32 _testInverseMatrix(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    matrix_t ma;
    matrix_t * pma = &ma, * pmb = NULL, * pmc = NULL;
//    olfloat_t madata[3 * 3] = {-3, 2, -5, -1, 0, -2, 3, -4, 1};
    oldouble_t madata[3 * 3] = {1, 4, 7, 3, 0, 5, -1, 9, 11};
    oldouble_t dbdata = 1.0;

    jf_matrix_init(pma, 3, 3, madata);

    u32Ret = jf_matrix_alloc(3, 3, &pmb);
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = jf_matrix_alloc(3, 3, &pmc);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Matrix A\n");
        jf_matrix_print(pma);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        u32Ret = jf_matrix_getDeterminant(pma, &dbdata);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            ol_printf("Determinant of Matrix A: %.3f\n\n", dbdata);

            ol_printf("Adjugate(A)\n");
            u32Ret = jf_matrix_adjugate(pmb, pma);
            if (u32Ret == JF_ERR_NO_ERROR)
                jf_matrix_print(pmb);
        }
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Inverse(A)\n");
        u32Ret = jf_matrix_inverse(pmb, pma);
        if (u32Ret == JF_ERR_NO_ERROR)
        {
            jf_matrix_print(pmb);

            jf_matrix_mul(pmc, pma, pmb);
            ol_printf("A * Inverse(A)\n");
            jf_matrix_print(pmc);

            jf_matrix_mul(pmc, pmb, pma);
            ol_printf("Inverse(A) * A\n");
            jf_matrix_print(pmc);
        }
    }

    if (pmb == NULL)
        jf_matrix_free(&pmb);
    if (pmc == NULL)
        jf_matrix_free(&pmc);

    return u32Ret;
}

static u32 _testHatMatrix(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
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

    jf_matrix_init(pma, 11, 5, madata);

    u32Ret = jf_matrix_alloc(11, 11, &pmb);

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Matrix A\n");
        jf_matrix_print(pma);

        u32Ret = jf_matrix_hat(pmb, pma);
    }

    if (u32Ret == JF_ERR_NO_ERROR)
    {
        ol_printf("Matrix B\n");
        jf_matrix_print(pmb);
    }

    if (pmb == NULL)
        jf_matrix_free(&pmb);

    return u32Ret;
}

static u32 _testMatrix(void)
{
    u32 u32Ret = JF_ERR_NO_ERROR;

    u32Ret = _testAddSubMatrix();
    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testMulMatrix();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testTransposeMatrix();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testInverseMatrix();

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testHatMatrix();

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */

olint_t main(olint_t argc, olchar_t ** argv)
{
    u32 u32Ret = JF_ERR_NO_ERROR;
    olchar_t strErrMsg[300];
    jf_logger_init_param_t lp;

    memset(&lp, 0, sizeof(lp));

    u32Ret = _parseCmdLineParam(argc, argv, &lp);

    if (u32Ret == JF_ERR_NO_ERROR)
        u32Ret = _testMatrix();

    if (u32Ret != JF_ERR_NO_ERROR)
    {
        jf_err_getMsg(u32Ret, strErrMsg, sizeof(strErrMsg));
        ol_printf("%s\n", strErrMsg);
    }

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


