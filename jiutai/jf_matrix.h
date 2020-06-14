/**
 *  @file jf_matrix.h
 *
 *  @brief Matrix header file which define the interface of matrix library.
 *
 *  @author Min Zhang
 *
 *  @note
 *  -# Routines declared in this file are included in jf_matrix library.
 *
 */

#ifndef JIUFENG_MATRIX_H
#define JIUFENG_MATRIX_H

/* --- standard C lib header files -------------------------------------------------------------- */


/* --- internal header files -------------------------------------------------------------------- */

#include "jf_basic.h"
#include "jf_err.h"

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

/* --- constant definitions --------------------------------------------------------------------- */


/* --- data structures -------------------------------------------------------------------------- */

/** Define the matrix data type.
 */
typedef struct
{
    /*The matrix data.*/
    oldouble_t * m_pdbData;
    /*Number of row in the matrix.*/
    olint_t m_nRow;
    /*Number of column in the matrix.*/
    olint_t m_nCol;
} matrix_t;

/* --- functional routines ---------------------------------------------------------------------- */

/** Allocate a matrix.
 *
 *  @param row [in] Number of row in the matrix.
 *  @param col [in] Number of column in the matrix.
 *  @param ppm [out] The matrix to be allocated.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_alloc(olint_t row, olint_t col, matrix_t ** ppm);

/** Free the matrix.
 *
 *  @param ppm [in/out] The matrix to be freed.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_free(matrix_t ** ppm);

/** Initialize a matrix.
 *
 *  @param pm [in] The matrix to be initialized.
 *  @param row [in] Number of row in the matrix.
 *  @param col [in] Number of column in the matrix.
 *  @param data [out] The data of the matrix.
 *
 *  @return Void.
 */
MATRIXAPI void MATRIXCALL jf_matrix_init(
    matrix_t * pm, olint_t row, olint_t col, oldouble_t * data);

/** Print a matrix.
 *
 *  @param pm [in] The matrix to be printed.
 *
 *  @return Void.
 */
MATRIXAPI void MATRIXCALL jf_matrix_print(matrix_t * pm);

/** Add 2 matrixs.
 *
 *  @note
 *  -# Number of row and column must be the same for the 2 matrixs.
 *  -# The result is saved to matrix b.
 *
 *  @param pma [in] The matrix a.
 *  @param pmb [in/out] The matrix b.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_add(matrix_t * pma, matrix_t * pmb);

/** Subtract 2 matrixs.
 *
 *  @note
 *  -# Number of row and column must be the same for the 2 matrixs.
 *  -# The result is saved to matrix b.
 *
 *  @param pma [in] The matrix a.
 *  @param pmb [in/out] The matrix b.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_sub(matrix_t * pma, matrix_t * pmb);

/** Multiply 2 matrixs.
 *
 *  @note
 *  -# Number of column in matrix a must be the same with number of the row in matrix b.
 *
 *  @param pmr [out] The matrix for the result.
 *  @param pma [in] The matrix a.
 *  @param pmb [in] The matrix b.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_mul(matrix_t * pmr, matrix_t * pma, matrix_t * pmb);

/** Transpose a matrix.
 *
 *  @note
 *  -# Number of column in transposed matrix must be the same with number of the row in original
 *   matrix.
 *  -# Number of row in transposed matrix must be the same with number of the column in original
 *   matrix.
 *
 *  @param pmt [out] The transposed matrix.
 *  @param pmo [in] The original matrix.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_transpose(matrix_t * pmt, matrix_t * pmo);

/** Get determinant of a matrix.
 *
 *  @param pm [in] The matrix.
 *  @param det [out] The determinant.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_getDeterminant(matrix_t * pm, oldouble_t * det);

/** Get the adjugate matrix of a matrix.
 *
 *  @note
 *  -# Number of column must be the same with number of the row in adjugate matrix.
 *  -# Number of column must be the same with number of the row in original matrix.
 *  -# Number of row in adjugate matrix must be the same with number of the column in original
 *   matrix.
 *
 *  @param pma [out] The adjugate matrix.
 *  @param pmo [in] The original matrix.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_adjugate(matrix_t * pma, matrix_t * pmo);

/** Get the inverse matrix of a matrix.
 *
 *  @note
 *  -# Number of column must be the same with number of the row in inverse matrix.
 *  -# Number of column must be the same with number of the row in original matrix.
 *  -# Number of row in inverse matrix must be the same with number of the column in original
 *   matrix.
 *
 *  @param pmi [out] The inverse matrix.
 *  @param pmo [in] The original matrix.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_inverse(matrix_t * pmi, matrix_t * pmo);

/** Get the hat matrix of a matrix.
 *
 *  @param pmh [out] The hat matrix.
 *  @param pmo [in] The original matrix.
 *
 *  @return The error code.
 *  @retval JF_ERR_NO_ERROR Success.
 */
MATRIXAPI u32 MATRIXCALL jf_matrix_hat(matrix_t * pmh, matrix_t * pmo);

#endif /*JIUFENG_MATRIX_H*/

/*------------------------------------------------------------------------------------------------*/

