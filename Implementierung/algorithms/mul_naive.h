#ifndef MUL_MUL_NAIVE_H
#define MUL_MUL_NAIVE_H

#include "../util/matrix_util.h"

/**
 * This is an alterantive implementation, which has more to do with the "traditional"
 * matrix multiplication, than in the CSC format. The idea was to compare it to algos operating
 * on compressed matrices. It expands each of the input matrices into
 * a 2d array of size no_rows*no_cols and then calculates each cell of c separately
 * and writes it into expanded matrix of c. Then it compresses the matrix and writes the result 
 */
__attribute__((hot)) void matrix_mul_naive(const void*  a, const void*  b, void*  c);

#endif //MUL_MUL_NAIVE_H
