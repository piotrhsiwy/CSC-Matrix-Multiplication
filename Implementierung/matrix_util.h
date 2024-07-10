#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include <stdint.h> // uint64_t
#include "error_handling/errors.h"
#include "matrix.h"
#include "util/dynamic_array.h"
#include "util/pair.h"

/**
 * Multiplies each value with the corresponsing value from the other vector under the same index.
 * Results of the multiplication are added to one another and returned from the function
 * 
 * @param size size of both of the vectors
 * @param vector_A
 * @param vector_B
 */
float mul_vectors_expanded(size_t size, float* const vector_A, float* const vector_B);

/**
 * Multiplies each value with the corresponding value from the other vector with the same index (if present).
 * Results of the multiplication are added to one another and returned from the function
 *
 * @param vector_A dynamically sized array of (index,value) pairs. Pairs are sorted by index in an increasing order!
 * @param vector_B as above
 */
float mul_vectors_compressed(___dynamic_array_pair_macro_defined* vector_A, ___dynamic_array_pair_macro_defined* vector_B);

/**
 * See extract_compressed_row_csc() below to inspect functionality. Difference is that
 * in this function we return fully-sized (expanded) row (vector) instead of
 * compressed one, which makes some potential optimizations possible.
 * 
 * @param vector note that size of this array is in matrix->no_cols
 * 
 * @return number of elements in the row
 */
size_t extract_expanded_row_csc(const matrix_data* const matrix, const uint64_t index, float* const vector);

/**
 * See extract_compressed_col_csc() below to inspect functionality. Difference is that
 * in this function we return fully-sized (expanded) column (vector) instead of
 * compressed one, which makes some potential optimizations possible.
 * 
 * @param vector note that size of this array is in matrix->no_rows
 * 
 * @return number of elements in the column
 */
size_t extract_expanded_col_csc(const matrix_data* const matrix, const uint64_t index, float* const vector);

/**
 * Extracts a row in a compressed form, from a matrix in CSC.
 * Values of row *index* lie as follows:
 *  - their values in values array under indices x for which, row_ptr[x] = i
 *  - their column-number is COL that col_ptr[COL] <= x < col_ptr[COL+1]
 * 
 * @param matrix the matrix in Compressed Sparse Column Format (CSC)
 * @param index - index of the row to be extracted
 * @param dynamic_vector - the extracted row will be written here this is
 *   dynamically sized array of (index,value) where indices are the
 *   columns indices of the values in the matrix
 * @return
 *  OUT_OF_MEMORY in case adding new element to the dynamic_vector fails (realloc fails)
 *  OK else
 * 
 */
error extract_compressed_row_csc(const matrix_data* const matrix, const uint64_t index, ___dynamic_array_pair_macro_defined* dynamic_vector);

/**
 * Extracts a column in a compressed form, from a matrix in CSC.
 * Values of column *index* lie as follows:
 *  - their values in values array under indices col_ptr[index], ..., col_ptr[index+1]
 *  - their row-number in row_indices array under the same indices
 * 
 * @param matrix the matrix in Compressed Sparse Column Format (CSC)
 * @param index index of the column to be extracted
 * @param dynamic_vector the extracted row will be written here this is
 *   dynamically sized array of (index,value) where indices are the
 *   row indices of the values in the matrix
 * @return
 *  OUT_OF_MEMORY in case adding new element to the dynamic_vector fails (realloc fails)
 *  OK else
 */
error extract_compressed_col_csc(const matrix_data* const matrix, const uint64_t index, ___dynamic_array_pair_macro_defined* dynamic_vector);

/**
 * expands a sorted array of (i,v)-pairs to an array of values, where
 * each value is under index i in the array. The former is a direct
 * result of extracting values from a matrix in the Compressed Sparse
 * Column Format (CSC). The latter can be useful e.g. when optimizing
 * matrix-algorithms for performance. Writes the resulting array to
 * extended. See picture below.
 * Memory management of both compressed and extended has to be handled
 * by the upper layer.
 *
 * @param length_compressed
 * @param compressed - is the array of (i,v)-pairs. They have to be sorted by
 *   index in an increasing order.
 * @param length_extended
 * @param extended - is the array of values of appropriate size where
 *   value v is under index i.
 *	 
 * @return
 * 	DIMENSION_MISMATCH if extended is too small.
 * 	OK else
 * See error_handling/errors.h
 *
 */
error expand_vector(const uint64_t length_compressed, const pair compressed[restrict length_compressed], const uint64_t length_extended, float extended[restrict length_extended]);

/**
 * expands a matrix in Compressed Sparse Column Format (CSC) to a regular matrix
 *
 * @param matrix_data - matrix in CSC. See matrix.h
 * @param matrix - to-be-written regular matrix of appropriate size
 *
 */
void expand_matrix(const matrix_data* const compressed, float* const * const expanded);

/**
 * compresses a matrix back to Compressed Sparse Column Format (CSC) from a
 * regular matrix.
 *
 * @param
 * @param no_rows
 * @param no_cols
 * @param expanded - regular matrix of size no_rows x no_columns
 * @param compressed - to-be-written matrix in the CSC format. See matrix.h
 * 
 */
error compress_matrix(const uint64_t no_rows, const uint64_t no_cols, const float expanded[no_rows][no_cols], matrix_data* const compressed);
#endif // MATRIX_UTIL_H
