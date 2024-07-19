#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include <stdint.h> // uint64_t
#include <string.h> // memccpy

#include "../error_handling/errors.h"
#include "../matrix.h"
#include "dynamic_array.h"
#include "pair.h"

/**
 * recursively frees a 2D array of size elements:
 *      free( float* array [size] )
 * note that the array can be bigger but only first size elements will be freed
 */
void nested_free(size_t size, float** vectors);

/**
 * Handles the "matrix" aka 2D dynamic arrays of values and indices. For each index *i*
 * from the set of indices present in the "matrix" accumulate all values with this
 * index from all the dynamic arrays (at most 1 per array). Place this accumulated
 * along with index *i* in the result dynamic array. The result is stored in
 * result_values and result_indices.
 * 
 * @param vector_2D_values
 * @param vector_2D_indices are part of the "matrix"
 * @param result_values
 * @param result_indices is where the result is stored
 * 	
 * @return
 *  // TODO 
 */ 
error sum_compressed_matrix
(
	DYNAMIC_ARRAY_2D_float* const		vector_2D_values,
	DYNAMIC_ARRAY_2D_uint64_t* const 	vector_2D_indices,
	DYNAMIC_ARRAY(float)* const 		result_values,
	DYNAMIC_ARRAY(uint64_t)* const 		result_indices
);

/**
 * multiply in place each value in the vector (float values) with scalar.
 */
void mul_compressed_values_vector_w_scalar(DYNAMIC_ARRAY(float)* vector, const float scalar);

/**
 * Multiplies a vector with an array of vectors. An array of rows of Matrix A and 1 column of Matrix B
 * yield 1 column of the resulting Matrix C. This function computes this column.
 * 
 * @param how_many_vectors is the number of row in the all_vectors matrix
 * @param vector_size is the number of cols in the all_vectors matrix and size of vector vector
 * @param vector is the vector containing one column form matrix B in the multiplication. Here will the resulting column be written.
 * @param all_vectors is the matrix containing all rows from matrix A in the multiplication
 */
error mul_vector_transposed_w_matrix(
	size_t how_many_vectors,
	size_t vector_size,
	float* vector, // [vector_size]
	float** all_vectors, // [how_many_vectors][vector_size]
	float* result) // [how_many_vectors]
;

/**
 * Multiplies each value with the corresponsing value from the other vector under the same index.
 * Results of the multiplication are added to one another and returned from the function
 * 
 * @param size size of both of the vectors
 * @param vector_A
 * @param vector_B
 */
float mul_vectors_expanded(size_t size, const float* vector_A, const float* vector_B);

/**
 * Multiplies each value with the corresponding value from the other vector with the same index (if present).
 * Results of the multiplication are added to one another and returned from the function
 *
 * @param values_A
 * @param indices_A together represent the first (index, value) vector
 * @param values_B
 * @param indices_B together represent the second (index, value) vector
 */
float mul_vectors_compressed
(
	DYNAMIC_ARRAY(float)* 		values_A,
	DYNAMIC_ARRAY(uint64_t)* 	indices_A,
	DYNAMIC_ARRAY(float)* 		values_B,
	DYNAMIC_ARRAY(uint64_t)* 	indices_B
);

/**
 * extracts all rows from a CSC matrix in the same way that extract_expanded_row_csc does, but runs
 * over values array only once and hence promises performance improvements at the cost of storage requirements
 */
void extract_expanded_all_rows_csc(const matrix_data* matrix, float *const * all_vectors);

/**
 * See extract_compressed_row_csc() below to inspect functionality. Difference is that
 * in this function we return fully-sized (expanded) row (vector) instead of
 * compressed one, which makes some potential optimizations possible.
 * 
 * @param vector note that size of this array is in matrix->no_cols
 * 
 * @return number of elements in the row
 */
size_t extract_expanded_row_csc(const matrix_data* matrix, uint64_t index, float* vector);

/**
 * See extract_compressed_col_csc() below to inspect functionality. Difference is that
 * in this function we return fully-sized (expanded) column (vector) instead of
 * compressed one, which makes some potential optimizations possible.
 * 
 * @param vector note that size of this array is in matrix->no_rows
 * 
 * @return number of elements in the column
 */
size_t extract_expanded_col_csc(const matrix_data* matrix, uint64_t index, float* vector);

/**
 * Extracts a row in a compressed form, from a matrix in CSC.
 * Values of row *index* lie as follows:
 *  - their values in values array under indices x for which, row_ptr[x] = i
 *  - their column-number is COL that col_ptr[COL] <= x < col_ptr[COL+1]
 * 
 * @param matrix the matrix in Compressed Sparse Column Format (CSC)
 * @param index - index of the row to be extracted
 * @param value
 * @param indices - the extracted row will be written here this is
 *   dynamically sized array of (index,value) where indices are the
 *   columns indices of the values in the matrix
 * @return OUT_OF_MEMORY in case adding new element to the dynamic_vector fails (realloc fails) OK else
 */
error extract_compressed_row_csc
(
	const matrix_data* const matrix,
	const uint64_t index,
	DYNAMIC_ARRAY(float)* values,
	DYNAMIC_ARRAY(uint64_t)* indices
);

/**
 * Extracts a column in a compressed form, from a matrix in CSC.
 * Values of column *index* lie as follows:
 *  - their values in values array under indices col_ptr[index], ..., col_ptr[index+1]
 *  - their row-number in row_indices array under the same indices
 * 
 * @param matrix the matrix in Compressed Sparse Column Format (CSC)
 * @param index index of the column to be extracted
 * @param values the values from the extracted row will be written here this is
 *   dynamically sized array of values
 * @param indices the row_indices from the extracted row will be written here this is
 *   dynamically sized array of uint64_t
 * @return
 *  OUT_OF_MEMORY in case adding new element to either the values or the indices fails (realloc fails)
 *  OK else
 */
error extract_compressed_col_csc(const matrix_data* const matrix, const uint64_t index, DYNAMIC_ARRAY(float)* values,  DYNAMIC_ARRAY(uint64_t)* indices);

/**
 * expands a matrix in Compressed Sparse Column Format (CSC) to a regular matrix
 *
 * @param matrix_data - matrix in CSC. See matrix.h
 * @param matrix - to-be-written regular matrix of appropriate size
 *
 */
void expand_matrix(const matrix_data* compressed, float *const * expanded);

/**
 * compresses a matrix back to Compressed Sparse Column Format (CSC) from a
 * regular matrix.
 *
 * @param
 * @param expanded - regular matrix of size no_rows x no_columns
 * @param compressed - to-be-written matrix in the CSC format. See matrix.h
 * 
 */
error compress_matrix(float** expanded, matrix_data* compressed);

#endif // MATRIX_UTIL_H
