#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include <stdint.h> // uint64_t
#include "error_handling/errors.h"
#include "matrix.h"
#include "util/dynamic_array.h"
#include "util/pair.h"

/*
 * Multiplies each value with the corresponding value from the other vector with the same index (if present).
 * Results of the multiplication are added to one another and returned from the function
 *
 * @param
 *  vector_A dynamically sized array of (index,value) pairs. Pairs are sorted by index in an increasing order!
 *  vector_B as above
 * @return
 *  sum of all multiplications' results, as described above
 * */
float mul_vectors_compressed(___dynamic_array_pair_macro_defined* vector_A, ___dynamic_array_pair_macro_defined* vector_B);

/*
 * Extracts a column in a compressed form, from a matrix in CSC.
 * @param
 *  matrix - the matrix in Compressed Sparse Column Format (CSC)
 *  index - index of the column to be extracted
 *  dynamic_vector - the extracted row will be written here this is
 *   dynamically sized array of (index,value) where indices are the
 *   row indices of the values in the matrix
 * @return
 *  OUT_OF_MEMORY in case adding new element to the dynamic_vector fails (realloc fails)
 * 
 * */
error extract_compressed_col_csc(const matrix_data* const matrix, const uint64_t index, ___dynamic_array_pair_macro_defined* dynamic_vector);

/*
 * Extracts a row in a compressed form, from a matrix in CSC.
 * @param
 *  matrix - the matrix in Compressed Sparse Column Format (CSC)
 *  index - index of the row to be extracted
 *  dynamic_vector - the extracted row will be written here this is
 *   dynamically sized array of (index,value) where indices are the
 *   columns indices of the values in the matrix
 * @return
 *  OUT_OF_MEMORY in case adding new element to the dynamic_vector fails (realloc fails)
 * 
 * */
error extract_compressed_row_csc(const matrix_data* const matrix, const uint64_t index, ___dynamic_array_pair_macro_defined* dynamic_vector);

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
 *  length_compressed
 *  compressed - is the array of (i,v)-pairs. They have to be sorted by
 *   index in an increasing order.
 *  length_extended
 *  extended - is the array of values of appropriate size where
 *   value v is under index i.
 *	 
 * @return
 * 	DIMENSION_MISMATCH if extended is too small.
 * 	OK else
 * See error_handling/errors.h
 *
 * */
error expand_vector(const uint64_t length_compressed, const pair compressed[restrict length_compressed], const uint64_t length_extended, float extended[restrict length_extended]);

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
