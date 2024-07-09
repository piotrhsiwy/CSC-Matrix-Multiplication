#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include <stdint.h> // uint64_t
#include "error_handling/errors.h"
#include "matrix.h"

/*
 * Represents a (index,value) pair that is very often used to store either row or column of a sparse matrix
 *
 * */
typedef struct {uint64_t index; float value;} pair;

/*
 * expands a sorted array of (i,v)-pairs to an array of values, where
 * each value is under index i in the array. The former is a direct
 * result of extracting values from a matrix in the Compressed Sparse
 * Column Format (CSC). The latter can be useful e.g. when optimizing
 * matrix-algorithms for performance. Writes the resulting array to
 * extended. See picture below.
 * Memory management of both compressed and extended has to be handled
 * by the upper layer.
 *
 * @param
 *  compressed - is the array of (i,v)-pairs. They have to be sorted by
 *   index in an increasing order.
 *  length_compressed
 *  extended - is the array of values of appropriate size where
 *   value v is under index i.
 *  length_extended
 *	 
 * @return
 * 	DIMENSION_MISMATCH if extended is too small.
 * 	OK else
 * See error_handling/errors.h
 *
 * */
error expand_row(const pair* const compressed, const uint64_t length_compressed, float* const extended, const uint64_t length_extended);

/*
 * expands a matrix in Compressed Sparse Column Format (CSC) to a regular matrix
 *
 * @param
 *  matrix_data - matrix in CSC. See matrix.h
 *  matrix - to-be-written regular matrix of appropriate size
 *
 * */
void expand_matrix(const matrix_data* const compressed, float* const * const expanded);

/*
 * compresses a matrix back to Compressed Sparse Column Format (CSC) from a
 * regular matrix.
 *
 * @param
 *  no_rows
 *  no_cols
 *  expanded - regular matrix of size no_rows x no_columns
 *  compressed - to-be-written matrix in the CSC format. See matrix.h
 * 
 * */
error compress_matrix(const uint64_t no_rows, const uint64_t no_cols, const float expanded[no_rows][no_cols], matrix_data* const compressed);
#endif // MATRIX_UTIL_H
