/**
 *  @file matrix.c
 *
 *  @brief The matrix implementation file
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
#include "xmalloc.h"
#include "matrix.h"

/* --- private data/data structure section --------------------------------- */

/* --- private routine section---------------------------------------------- */
static void _clearMatrixData(matrix_t * pm)
{
    memset(pm->m_pdbData, 0, sizeof(oldouble_t) * pm->m_nRow * pm->m_nCol);
}

static olint_t _sgn(olint_t order)
{
    return order % 2 ? -1 : 1;
}

static olint_t _invertOrder(olint_t * list, olint_t n)
{
    olint_t ret = 0;
    olint_t i, j;

    for (i = 1; i < n; i ++)
        for (j = 0; j < i; j ++)
            if (list[j] > list[i])
                ret++;

    return ret;
}

static void _swap(olint_t *a, olint_t *b)
{
    olint_t m;
    m = *a;
    *a = *b;
    *b = m;
}

static oldouble_t _det(
    oldouble_t * p, olint_t n, olint_t k, olint_t * list, oldouble_t sum)
{
    olint_t order;
    oldouble_t item;
    olint_t i;

    if (k >= n)
    {
        order = _invertOrder(list, n);
        item = (oldouble_t)_sgn(order);
        for (i = 0; i < n; i ++)
        {
            item *= *(p + i * n + list[i]);
        }
        return sum + item;
    }
    else
    {
        for(i = k; i < n; i++)
        {
            _swap(&list[k], &list[i]);
            sum = _det(p, n, k + 1, list, sum);
            _swap(&list[k], &list[i]);
        }
    }

    return sum;
}


static u32 _determinant(oldouble_t * p, olint_t n, oldouble_t * dbc)
{
    u32 u32Ret = OLERR_NO_ERROR;
    oldouble_t dbret = 0.0;
    olint_t i;
    olint_t * list = NULL;

    u32Ret = xcalloc((void **)&list, sizeof(olint_t) * n);
    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < n; i++)
            list[i] = i;

        dbret = _det(p, n, 0, list, 0.0);

        *dbc = dbret;
    }

    if (list != NULL)
        xfree((void **)&list);

    return u32Ret;
}

static u32 _algebraicCofactor(
    oldouble_t *p, olint_t m, olint_t n, olint_t k, oldouble_t * dbc)
{
    u32 u32Ret = OLERR_NO_ERROR;
    oldouble_t dbret = 0.0;
    olint_t len = (k - 1) * (k - 1);
    oldouble_t * cofactor = NULL;
    olint_t i, count = 0;
    olint_t raw_len = k * k;

    u32Ret = xcalloc((void **)&cofactor, sizeof(oldouble_t) * len);
    if (u32Ret == OLERR_NO_ERROR)
    {
        for (i = 0; i < raw_len; i++)
            if (i / k != m && i % k != n)
                *(cofactor + count++) = *(p + i);

        u32Ret = _determinant(cofactor, k - 1, &dbret);
        if (u32Ret == OLERR_NO_ERROR)
        {
            if ((m + n) % 2)
                dbret = -dbret;

            *dbc = dbret;
        }
    }

    if (cofactor != NULL)
        xfree((void **)&cofactor);

    return u32Ret;
}

/* --- public routine section ---------------------------------------------- */
u32 allocMatrix(olint_t row, olint_t col, matrix_t ** ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;
    matrix_t * pm = NULL;
    olint_t size;

    size = sizeof(matrix_t) + sizeof(oldouble_t) * row * col;
    u32Ret = xcalloc((void **)&pm, size);
    if (u32Ret == OLERR_NO_ERROR)
    {
        pm->m_pdbData = (oldouble_t *)(pm + 1);
        pm->m_nRow = row;
        pm->m_nCol = col;

        *ppm = pm;
    }

    return u32Ret;
}

u32 freeMatrix(matrix_t ** ppm)
{
    u32 u32Ret = OLERR_NO_ERROR;

    u32Ret = xfree((void **)ppm);

    return u32Ret;
}

void initMatrix(matrix_t * pm, olint_t row, olint_t col, oldouble_t * data)
{
    memset(pm, 0, sizeof(*pm));
    pm->m_nRow = row;
    pm->m_nCol = col;
    pm->m_pdbData = data;
}

u32 addMatrix(matrix_t * pma, matrix_t * pmb)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;

    assert((pma != NULL) && (pmb != NULL));
    assert((pma->m_nRow == pmb->m_nRow) && (pma->m_nCol == pmb->m_nCol));

    for (i = 0; i < pma->m_nRow * pma->m_nCol; i ++)
    {
        pma->m_pdbData[i] += pmb->m_pdbData[i];
    }

    return u32Ret;
}

u32 subMatrix(matrix_t * pma, matrix_t * pmb)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;

    assert((pma != NULL) && (pmb != NULL));
    assert((pma->m_nRow == pmb->m_nRow) && (pma->m_nCol == pmb->m_nCol));

    for (i = 0; i < pma->m_nRow * pma->m_nCol; i ++)
    {
        pma->m_pdbData[i] -= pmb->m_pdbData[i];
    }

    return u32Ret;
}

u32 mulMatrix(matrix_t * pmr, matrix_t * pma, matrix_t * pmb)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, j, k;

    assert((pmr != NULL) && (pma != NULL) && (pmb != NULL));
    assert(pma->m_nCol == pmb->m_nRow);

    _clearMatrixData(pmr);

    for (i = 0; i < pma->m_nRow; i ++)
        for (j = 0; j < pmb->m_nCol; j ++)
        {
            for (k = 0; k < pma->m_nCol; k ++)
            {
                pmr->m_pdbData[i * pmb->m_nCol + j] +=
                    pma->m_pdbData[i * pma->m_nCol + k] *
                    pmb->m_pdbData[k * pmb->m_nCol + j];
            }
        }

    return u32Ret;
}

void printMatrix(matrix_t * pm)
{
    olint_t i, j;
    oldouble_t * pdb = pm->m_pdbData;

    for (i = 0; i < pm->m_nRow; i ++)
    {
        for (j = 0; j < pm->m_nCol; j ++)
        {
            ol_printf("%.3f  ", *pdb);
            pdb ++;
        }
        ol_printf("\n");
    }

    ol_printf("\n");
}

u32 transposeMatrix(matrix_t * pmt, matrix_t * pmo)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i, j;

    assert((pmt != NULL) && (pmo != NULL));
    assert((pmt->m_nRow == pmo->m_nCol) && (pmt->m_nCol == pmo->m_nRow));

    _clearMatrixData(pmt);

    for (i = 0; i < pmt->m_nRow; i ++)
        for (j = 0; j < pmt->m_nCol; j ++)
        {
            pmt->m_pdbData[i * pmt->m_nCol + j] =
                    pmo->m_pdbData[j * pmo->m_nCol + i];

        }

    return u32Ret;
}

u32 adjugateMatrix(matrix_t * pma, matrix_t * pmo)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t len = pma->m_nRow * pma->m_nCol;
    olint_t count = 0;
    olint_t i;
    oldouble_t * dbdata;

    _clearMatrixData(pma);

    assert((pma != NULL) && (pmo != NULL));
    assert((pma->m_nRow == pma->m_nCol) && (pmo->m_nRow == pmo->m_nCol));
    assert(pma->m_nRow == pmo->m_nCol);

    for (i = 0; (i < len) && (u32Ret == OLERR_NO_ERROR); i ++)
    {
        dbdata = pma->m_pdbData + count++;
        u32Ret = _algebraicCofactor(pmo->m_pdbData,
            i % pmo->m_nRow, i / pmo->m_nRow, pmo->m_nRow, dbdata);
    }

    return u32Ret;
}

u32 inverseMatrix(matrix_t * pmi, matrix_t * pmo)
{
    u32 u32Ret = OLERR_NO_ERROR;
    olint_t i;
    oldouble_t det;

    _clearMatrixData(pmi);

    assert((pmi != NULL) && (pmo != NULL));
    assert((pmi->m_nRow == pmi->m_nCol) && (pmo->m_nRow == pmo->m_nCol));
    assert(pmi->m_nRow == pmo->m_nCol);

    u32Ret = _determinant(pmo->m_pdbData, pmo->m_nRow, &det);
    if (u32Ret != OLERR_NO_ERROR)
        return u32Ret;

    if (det == 0)
        return OLERR_MATRIX_SINGULAR;

    adjugateMatrix(pmi, pmo);

    for (i = 0; i < pmi->m_nRow * pmi->m_nCol; i++)
        pmi->m_pdbData[i] /= det;

    return u32Ret;
}

u32 getMatrixDeterminant(matrix_t * pm, oldouble_t * det)
{
    return _determinant(pm->m_pdbData, pm->m_nRow, det);
}

u32 hatMatrix(matrix_t * pmh, matrix_t * pmo)
{
    u32 u32Ret = OLERR_NO_ERROR;
    matrix_t * pmt = NULL, * pmm = NULL, * pmi = NULL;
    matrix_t * pmm2 = NULL;

    u32Ret = allocMatrix(pmo->m_nCol, pmo->m_nRow, &pmt);
    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(pmo->m_nCol, pmo->m_nCol, &pmm);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(pmo->m_nCol, pmo->m_nCol, &pmi);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = allocMatrix(pmo->m_nRow, pmo->m_nCol, &pmm2);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = transposeMatrix(pmt, pmo);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = mulMatrix(pmm, pmt, pmo);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = inverseMatrix(pmi, pmm);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = mulMatrix(pmm2, pmo, pmi);

    if (u32Ret == OLERR_NO_ERROR)
        u32Ret = mulMatrix(pmh, pmm2, pmt);

    if (pmt != NULL)
        freeMatrix(&pmt);
    if (pmm != NULL)
        freeMatrix(&pmm);
    if (pmi != NULL)
        freeMatrix(&pmi);
    if (pmm2 != NULL)
        freeMatrix(&pmm2);

    return u32Ret;
}

/*---------------------------------------------------------------------------*/


