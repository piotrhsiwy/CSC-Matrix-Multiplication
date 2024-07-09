#ifndef MUL_SCALAR_H
#define MUL_SCALAR_H

#include "../matrix_util.h"

/* 
 * Multiples two matrices A and B in Compressed Sparse Column Format and saves the result in result
 * @param
 *  a is a pointer to matrix_data containing metadata of the matrix A
 *  b is a pointer to matrix_data containing metadata of the matrix B
 *  results is a pointer to matrix_data containing metadata of the resulting matrix after the fuction is executed
 */
error matrix_mul_scalar(const matrix_data* const a, const matrix_data* const b, matrix_data* const result);

#endif // MUL_SCALAR_H
