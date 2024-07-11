#ifndef MUL_SCALAR_H
#define MUL_SCALAR_H

#include <string.h> // memset
#include "../util/matrix_util.h" // extracting rows, cols, multiplying vectors
#include "../util/dynamic_array.h"

/**
 * Multiples two matrices A and B in Compressed Sparse Column Format and saves the result in result
 * 
 * @param a is a pointer to matrix_data containing metadata of the matrix A
 * @param b is a pointer to matrix_data containing metadata of the matrix B
 * @param results is a pointer to matrix_data containing metadata of the resulting matrix after the fuction is executed
 */
void matrix_mul_scalar(const matrix_data* const a, const matrix_data* const b, matrix_data* const result);

#endif // MUL_SCALAR_H
