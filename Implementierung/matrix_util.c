#include <stdio.h>

#include "matrix_util.h"

float mul_vectors_expanded(size_t size, float* const vector_A, float* const vector_B){
	float acc = 0.0f;
	while(size--){
		acc += vector_A[size] * vector_B[size];
	}
	return acc;
}

float mul_vectors_compressed(___dynamic_array_pair_macro_defined* vector_A, ___dynamic_array_pair_macro_defined* vector_B){
	float acc = 0.0f;
	for(uint64_t i = 0, j = 0; i < vector_A->length && j < vector_B->length;){
		// I can use the fact that the incides are increasing in both arrays
		pair cur_pair_A = ___dynamic_array_pair_macro_defined_at(vector_A, i);
		pair cur_pair_B = ___dynamic_array_pair_macro_defined_at(vector_B, j);
		if(cur_pair_A.index > cur_pair_B.index){
			j++;
			continue;
		} else if(cur_pair_A.index < cur_pair_B.index){
			i++;
			continue;
		} else {
			acc += cur_pair_A.value * cur_pair_B.value;
			i++;
			j++;
		}
	}
	return acc;
}

size_t extract_expanded_row_csc(const matrix_data* const matrix, const uint64_t index, float* const vector){
	for(uint64_t j = 0; j < matrix->dynamic_values.length; j++){
		if(___dynamic_array_uint64_t_macro_defined_at(&matrix->dynamic_row_indices, j) == index){
			size_t index; // even though the type of the index is uint64_t, if it would not fit into size_t, vector could not be allocated. Therefore I can be sure it will always fit
			for(size_t l = 0; l < matrix->no_rows+1; l++){
				if(matrix->col_ptr[l] <= j && matrix->col_ptr[l+1] > j){
					index = l;				
					break;
				}
			}
			float value = ___dynamic_array_float_macro_defined_at(&matrix->dynamic_values, j);
			vector[index] = value; // note that this array is always large enough
		}
	}
}

size_t extract_expanded_col_csc(const matrix_data* const matrix, const uint64_t index, float* const vector){
	size_t number_of_elements = 0;
	for(uint64_t j = matrix->col_ptr[index]; j < matrix->col_ptr[index+1]; j++){
		float value = ___dynamic_array_float_macro_defined_at(&matrix->dynamic_values, j);
		size_t index = ___dynamic_array_uint64_t_macro_defined_at(&matrix->dynamic_row_indices, j);
		vector[index] = value; // note that vector is always large enough
		number_of_elements++;
	}
	return number_of_elements;
}

error extract_compressed_row_csc(const matrix_data* const matrix, const uint64_t index, ___dynamic_array_pair_macro_defined* dynamic_vector){
	for(uint64_t j = 0; j < matrix->dynamic_values.length; j++){
		if(___dynamic_array_uint64_t_macro_defined_at(&matrix->dynamic_row_indices, j) == index){
			pair new_pair;
			for(uint64_t l = 0; l < matrix->no_rows+1; l++){
				if(matrix->col_ptr[l] <= j && matrix->col_ptr[l+1] > j){
					new_pair.index = l;				
					break;
				}
			}
			new_pair.value = ___dynamic_array_float_macro_defined_at(&matrix->dynamic_values, j);
			if(___dynamic_array_pair_macro_defined_add(dynamic_vector, new_pair)){
				fprintf(stderr, "Cannot add new element to dynamic_vector in function %s\n", __FUNCTION__);
				return OUT_OF_MEMORY;
			}
		}
	}
	return OK;
}

error extract_compressed_col_csc(const matrix_data* const matrix, const uint64_t index, ___dynamic_array_pair_macro_defined* dynamic_vector){
	for(uint64_t j = matrix->col_ptr[index]; j < matrix->col_ptr[index+1]; j++){
		pair new_pair = {
			.index = ___dynamic_array_uint64_t_macro_defined_at(&matrix->dynamic_row_indices, j),
			.value = ___dynamic_array_float_macro_defined_at(&matrix->dynamic_values, j)
		};
		if(___dynamic_array_pair_macro_defined_add(dynamic_vector, new_pair)){
			fprintf(stderr, "Cannot add new element to dynamic_vector in function %s\n", __FUNCTION__);
			return OUT_OF_MEMORY;
		}
	}
	return OK;
}

error expand_vector(const uint64_t length_compressed, const pair compressed[restrict length_compressed], const uint64_t length_extended, float extended[restrict length_extended]){
	if(compressed[length_compressed-1].index >= length_extended){
		return DIMENSION_MISMATCH;
	}
	for(uint64_t i = 0; i < length_compressed; i++){
		extended[compressed[i].index] = compressed[i].value;
	}
	return OK;
}

void expand_matrix(const matrix_data* const compressed, float* const * const expanded){
	for(uint64_t col = 0; col <= compressed->no_cols; col++){
		for(uint64_t i = compressed->col_ptr[col]; i <= compressed->col_ptr[col+1]; i++){
			uint64_t row = ___dynamic_array_uint64_t_macro_defined_at(&compressed->dynamic_row_indices, i);
			float value = ___dynamic_array_float_macro_defined_at(&compressed->dynamic_values, i);
			expanded[row][col] = value;
		}
	}
}

error compress_matrix(const uint64_t no_rows, const uint64_t no_cols, const float expanded[no_rows][no_cols], matrix_data* const compressed){
	compressed->no_rows = no_rows;
	compressed->no_cols = no_cols;

	//TODO
		
	return OK;
}
