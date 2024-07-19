#include <stdio.h>
#include <stdbool.h> // bool in sum_compressed_matrix

#include "matrix_util.h"

void nested_free(size_t size, float** vectors){
    while(--size){
        free(vectors[size]);
    }
    free(vectors[0]);
    free(vectors);
}

error sum_compressed_matrix
(
	DYNAMIC_ARRAY_2D_float* const		vector_2D_values,
	DYNAMIC_ARRAY_2D_uint64_t* const 	vector_2D_indices,
	DYNAMIC_ARRAY(float)* const 		result_values,
	DYNAMIC_ARRAY(uint64_t)* const 		result_indices
){
	if(!vector_2D_values->length){
		// no vectors to sum, return empty array
		return OK;
	}

	result_values->length = 0;
	size_t index_in_vector[vector_2D_values->length];
	memset(index_in_vector, 0, vector_2D_values->length * sizeof(size_t));
	size_t how_many_have_values_left = vector_2D_values->length;

	size_t lowest_index = 0;
	while (true)
	{
		if(!how_many_have_values_left)
			break;
		float acc = 0.0f;
		for (size_t vector_index = 0; vector_index < vector_2D_values->length; vector_index++)
		{
			DYNAMIC_ARRAY(float) 	cur_values 	= dynamic_array_at(___dynamic_array_float_macro_defined		, vector_2D_values , vector_index);
			DYNAMIC_ARRAY(uint64_t) cur_indices = dynamic_array_at(___dynamic_array_uint64_t_macro_defined	, vector_2D_indices, vector_index);
			if(index_in_vector[vector_index] != cur_values.length)
			{ /* there are values left in this vector */
				float 	 cur_val = dynamic_array_at(float   , &cur_values , index_in_vector[vector_index]);
				uint64_t cur_ind = dynamic_array_at(uint64_t, &cur_indices, index_in_vector[vector_index]);
				if(cur_ind == lowest_index){
					/* belongs to the currently accumulated value */
					acc += cur_val;
					index_in_vector[vector_index]++;
					if(index_in_vector[vector_index] >= cur_values.length)
						how_many_have_values_left--;
				} else{
					/* do nothing if index is too big (it will be never be lower, since pairs are sorted by increasing indices in vectors) */
				}
			}	
		}
		if(acc != 0.0f)
		{
			if(dynamic_array_add(float, result_values, acc))
			{
				return OUT_OF_MEMORY;
			}
			if(dynamic_array_add(uint64_t, result_indices, lowest_index))
			{
				return OUT_OF_MEMORY;
			}
		}
		lowest_index++;
	}

	return OK;	
}

void mul_compressed_values_vector_w_scalar(DYNAMIC_ARRAY(float)* vector, const float scalar){
	for (size_t i = 0; i < vector->length; i++)
	{
		dynamic_array_at(float, vector, i) *= scalar;
	}
}

static void mul_matrix_horizontal(const size_t m, const size_t n, float** const A, float** const B){
	// note the order of for-loops
	for(size_t i = 0; i < m; i++){
		for(size_t j = 0; j < n; j++){
			A[i][j] *= B[i][j];
		}
	}
}

static void collapse_matrix_horizontal(const size_t m, const size_t n, float* const vector, float** const matrix){
	memset(vector, 0, m * sizeof(*vector));
	for(size_t i = 0; i < m; i++){
		for(size_t j = 0; j < n; j++){
			vector[i] += matrix[i][j];
		}
	}
}

error mul_vector_transposed_w_matrix(
	const size_t how_many_vectors,
	const size_t vector_size,
	float* const vector, // [vector_size]
	float** const all_vectors, // [how_many_vectors][vector_size]
	float* const result) // [how_many_vectors]
{
	// copied_vectors is a matrix where every row is equal to *vector*. One can think about it as single transposed and copied column from matrix B
    float** copied_vector = malloc(sizeof(*copied_vector)* how_many_vectors);
    if(!copied_vector){
        fprintf(stderr, "Cannot allocate memory for copied_vector in function %s.\n", __FUNCTION__);
        return OUT_OF_MEMORY;
    }
    for(size_t i = 0; i < how_many_vectors; i++){
        float* new_vector = malloc(sizeof(*vector) * vector_size);
        if(!new_vector){
            fprintf(stderr, "Cannot allocate memory for copied_vector[%lu] vector field in function %s.\n", i, __FUNCTION__);
            nested_free(i, copied_vector);
            return OUT_OF_MEMORY;
        }
        memmove(new_vector, vector, sizeof(*vector) * vector_size);
        copied_vector[i] = new_vector;
    }

	// multiply the created matrix with *all_vectors* "horizontally" = so matrix "x" multiplication
	mul_matrix_horizontal(how_many_vectors, vector_size, copied_vector, all_vectors);
	collapse_matrix_horizontal(how_many_vectors, vector_size, result, copied_vector);
	nested_free(how_many_vectors, copied_vector);
	return OK;
}

float mul_vectors_expanded(size_t size, const float* const vector_A, const float* const vector_B){
	float acc = 0.0f;
	for(size_t i = 0; i < size; i++){
		acc += vector_A[i] * vector_B[i];
	}
	return acc;
}

float mul_vectors_compressed
(
	DYNAMIC_ARRAY(float)* 		values_A,
	DYNAMIC_ARRAY(uint64_t)* 	indices_A,
	DYNAMIC_ARRAY(float)* 		values_B,
	DYNAMIC_ARRAY(uint64_t)* 	indices_B
){
	float acc = 0.0f;
	for(uint64_t i = 0, j = 0; i < values_A->length && j < values_B->length;){
		// I can use the fact that the incides are increasing in both arrays
		float 	 cur_val_A = dynamic_array_at(float, values_A, i);
		uint64_t cur_ind_A = dynamic_array_at(uint64_t, indices_A, i);
		float 	 cur_val_B = dynamic_array_at(float, values_B, i);
		uint64_t cur_ind_B = dynamic_array_at(uint64_t, indices_B, i);
		if(cur_ind_A > cur_ind_B){
			j++;
			continue;
		} else if(cur_ind_A < cur_ind_B){
			i++;
			continue;
		} else {
			acc += cur_val_A * cur_val_B;
			i++;
			j++;
		}
	}
	return acc;
}

void extract_expanded_all_rows_csc(const matrix_data* const matrix, float* const * const all_vectors){	
	for(uint64_t j = 0; j < matrix->values.length; j++) {
		size_t index_in_vector = 0;
		while(( index_in_vector < matrix->no_cols+1 ) && ( j >= matrix->col_ptr[index_in_vector + 1] )){
            index_in_vector++; // scroll to the correct column
        }

        size_t which_vector = dynamic_array_at(uint64_t, &matrix->row_indices, j);
        float value = dynamic_array_at(float, &matrix->values, j);
		all_vectors[which_vector][index_in_vector] = value; // note that this array is always large enough
	}
}

size_t extract_expanded_row_csc(const matrix_data* const matrix, const uint64_t index, float* const vector){
	size_t number_of_elements = 0;
	for(uint64_t j = 0; j < matrix->values.length; j++){
		if(dynamic_array_at(uint64_t, &matrix->row_indices, j) == index){
			size_t index_in_vector = 0;
			for(size_t l = 0; l < matrix->no_cols+1; l++){
				if(matrix->col_ptr[l] <= j && matrix->col_ptr[l+1] > j){
					index_in_vector = l;				
					break;
				}
			}
			float value = dynamic_array_at(float, &matrix->values, j);
			vector[index_in_vector] = value; // note that this array is always large enough
			number_of_elements++;
		}
	}
	return number_of_elements;
}

size_t extract_expanded_col_csc(const matrix_data* const matrix, const uint64_t index, float* const vector){
	size_t number_of_elements = 0;
	for(uint64_t j = matrix->col_ptr[index]; j < matrix->col_ptr[index+1]; j++){
		float value = dynamic_array_at(float, &matrix->values, j);
		uint64_t index = dynamic_array_at(uint64_t, &matrix->row_indices, j);;
		vector[index] = value; // note that vector is always large enough
		number_of_elements++;
	}
	return number_of_elements;
}

error extract_compressed_row_csc
(
	const matrix_data* const matrix,
	const uint64_t index,
	DYNAMIC_ARRAY(float)* values,
	DYNAMIC_ARRAY(uint64_t)* indices
){
	for(uint64_t j = 0; j < matrix->values.length; j++){
		if(dynamic_array_at(uint64_t, &matrix->row_indices, j) == index){
			uint64_t ind;
			for(uint64_t l = 0; l < matrix->no_cols+1; l++){
				if(matrix->col_ptr[l] <= j && matrix->col_ptr[l+1] > j){
					ind = l;
					break;
				}
			}
			float value = dynamic_array_at(float, &matrix->values, j);
			if(dynamic_array_add(float, values, value))
			{
				fprintf(stderr, "Cannot add new element to values in function %s\n", __FUNCTION__);
				return OUT_OF_MEMORY;
			}
			if(dynamic_array_add(uint64_t, indices, ind))
			{
				fprintf(stderr, "Cannot add new element to values in function %s\n", __FUNCTION__);
				return OUT_OF_MEMORY;
			}
		}
	}
	return OK;
}

error extract_compressed_col_csc(const matrix_data* const matrix, const uint64_t index, DYNAMIC_ARRAY(float)* values,  DYNAMIC_ARRAY(uint64_t)* indices){
	for(uint64_t j = matrix->col_ptr[index]; j < matrix->col_ptr[index+1]; j++){
		float value = dynamic_array_at(float, &matrix->values, j);
		uint64_t row_index = dynamic_array_at(uint64_t, &matrix->row_indices, j);
		if(dynamic_array_add(float, values, value))
		{
			fprintf(stderr, "Cannot add new element to values in function %s\n", __FUNCTION__);
			return OUT_OF_MEMORY;
		}
		if(dynamic_array_add(uint64_t, indices, row_index))
		{
			fprintf(stderr, "Cannot add new element to indices in function %s\n", __FUNCTION__);
			return OUT_OF_MEMORY;
	}
	}
	return OK;
}

__attribute__((unused)) error expand_vector(const uint64_t length_compressed, const pair compressed[restrict length_compressed], const uint64_t length_extended, float extended[restrict length_extended]){
	if(compressed[length_compressed-1].index >= length_extended){
		return DIMENSION_MISMATCH;
	}
	for(uint64_t i = 0; i < length_compressed; i++){
		extended[compressed[i].index] = compressed[i].value;
	}
	return OK;
}

void expand_matrix(const matrix_data* const compressed, float* const * const expanded){
	for(uint64_t col = 0; col < compressed->no_cols; col++){
		extract_expanded_col_csc(compressed, col, expanded[col]);
	}
}

static error compress_vector
(
	const uint64_t count,
	const float vector[count],
	DYNAMIC_ARRAY(float)* const values,
	DYNAMIC_ARRAY(uint64_t)* const indices
){
	for (uint64_t i = 0; i < count; i++)
	{
		float value = vector[i];
		uint64_t index = i;
		if(value != 0){
			if(dynamic_array_add(float, values, value))
			{
				fprintf(stderr, "Cannot add an element to dynamic array values in function %s\n", __FUNCTION__);
				return OUT_OF_MEMORY;
			}
			if(dynamic_array_add(uint64_t, indices, index))
			{
				fprintf(stderr, "Cannot add an element to dynamic array values in function %s\n", __FUNCTION__);
				return OUT_OF_MEMORY;
			}
		}
	}
	return OK;
}

error compress_matrix(float** const expanded, matrix_data* const compressed){
	uint64_t* col_ptr_writer = compressed->col_ptr;
	*col_ptr_writer++ = 0;
	for(uint64_t col = 0; col < compressed->no_cols; col++)
	{
		{
			error error = compress_vector(compressed->no_rows, expanded[col], &compressed->values , &compressed->row_indices);
			if(error != OK){
				return error;
			}
		}
		*col_ptr_writer++ = compressed->values.length;
	}
	return OK;
}