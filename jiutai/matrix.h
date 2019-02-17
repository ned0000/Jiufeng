/**
 *  @file matrix.h
 *
 *  @brief Matrix header file, provide some functional routine for matrix
 *   library
 *
 *  @author Min Zhang
 *
 *  @note
 *
 */

#ifndef JIUFENG_MATRIX_H
#define JIUFENG_MATRIX_H

/* --- standard C lib header files ----------------------------------------- */

/* --- internal header files ----------------------------------------------- */
#include "olbasic.h"
#include "errcode.h"
#include "bases.h"

#undef MATRIXAPI
#undef MATRIXCALL
#ifdef WINDOWS
    #include "windows.h"
    #if defined(JIUFENG_MATRIX_DLL)
        #define MATRIXAPI  __declspec(dllexport)
        #define MATRIXCALL
    #else
        #define MATRIXAPI
        #define MATRIXCALL __cdecl
    #endif
#else
    #define MATRIXAPI
    #define MATRIXCALL
#endif

/* --- constant definitions ------------------------------------------------ */


/* --- data structures ----------------------------------------------------- */
typedef struct
{
    oldouble_t * m_pdbData;
    olint_t m_nRow;
    olint_t m_nCol;
} matrix_t;

/* --- functional routines ------------------------------------------------- */

MATRIXAPI u32 MATRIXCALL allocMatrix(olint_t row, olint_t col, matrix_t ** ppm);

MATRIXAPI u32 MATRIXCALL freeMatrix(matrix_t ** ppm);

MATRIXAPI void MATRIXCALL initMatrix(
    matrix_t * pm, olint_t row, olint_t col, oldouble_t * data);

MATRIXAPI void MATRIXCALL printMatrix(matrix_t * pm);

MATRIXAPI u32 MATRIXCALL addMatrix(matrix_t * pma, matrix_t * pmb);

MATRIXAPI u32 MATRIXCALL subMatrix(matrix_t * pma, matrix_t * pmb);

MATRIXAPI u32 MATRIXCALL mulMatrix(
    matrix_t * pmr, matrix_t * pma, matrix_t * pmb);

MATRIXAPI u32 MATRIXCALL transposeMatrix(matrix_t * pmt, matrix_t * pmo);

MATRIXAPI u32 MATRIXCALL getMatrixDeterminant(matrix_t * pm, oldouble_t * det);

MATRIXAPI u32 MATRIXCALL adjugateMatrix(matrix_t * pma, matrix_t * pmo);

MATRIXAPI u32 MATRIXCALL inverseMatrix(matrix_t * pmi, matrix_t * pmo);

MATRIXAPI u32 MATRIXCALL hatMatrix(matrix_t * pmh, matrix_t * pmo);

#endif /*JIUFENG_MATRIX_H*/

/*---------------------------------------------------------------------------*/

