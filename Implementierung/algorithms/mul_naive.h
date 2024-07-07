#ifndef MATRIX_MULT_H
#define MATRIX_MULT_H

#include "../matrix.h"

/* 
 * Multiples two matrices A and B in Compressed Sparse Column Format and saves the result in result
 * @param
 *  a is a pointer to matrix_data containing metadata of the matrix A
 *  b is a pointer to matrix_data containing metadata of the matrix B
 *  results is a pointer to matrix_data containing metadata of the resulting matrix after the fuction is executed
 */
void matrix_mul_naive(const void* a, const void* b, void* result);

#endif // MATRIX_MULT_H