#include <stdio.h>

#include "matrix_util.h"

void nested_free(size_t size, float** vectors){
    while(--size){
        free(vectors[size]);
    }
    free(vectors[0]);
    free(vectors);
}

// TODO this can be done with SIMD faster?
// TODO should we return any erorrs here?
void collapse_matrix_horizontal(const size_t m, const size_t n, float* const vector, float** const matrix){
	// note the order of for-loops
	memset(vector, 0, n * sizeof(*vector));
	for(size_t i = 0; i < m; i++){
		for(size_t j = 0; j < n; j++){
			vector[i] += matrix[i][j];
		}
	}
}

// TODO this can be done with SIMD faster?
// TODO should we return any erorrs here?
void mul_matrix_horizontal(const size_t m, const size_t n, float** const A, float** const B){
	// note the order of for-loops
	for(size_t i = 0; i < m; i++){
		for(size_t j = 0; j < n; j++){
			A[i][j] *= B[i][j];
		}
	}
}

error mul_vector_transposed_w_matrix(
	const size_t how_many_vectors,
	const size_t vector_size,
	float* const vector,
	float** const all_vectors)
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
        memmove(new_vector, vector, sizeof(*vector) * vector_size); // TODO should/can we use memcpy here?
        copied_vector[i] = new_vector;
    }

	// multiply the created matrix with *all_vectors* "horizontally" = so matrix "x" multiplication
	mul_matrix_horizontal(how_many_vectors, vector_size, copied_vector, all_vectors);
	collapse_matrix_horizontal(how_many_vectors, vector_size, vector, copied_vector);
	nested_free(how_many_vectors, copied_vector);
	return OK;
}

float mul_vectors_expanded(size_t size, float* const vector_A, float* const vector_B){
	float acc = 0.0f;
	for(size_t i = 0; i < size; i++){
		acc += vector_A[i] * vector_B[i];
	}
	return acc;
}

float mul_vectors_compressed(DYNAMIC_ARRAY(pair)* vector_A, DYNAMIC_ARRAY(pair)* vector_B){
	float acc = 0.0f;
	for(uint64_t i = 0, j = 0; i < vector_A->length && j < vector_B->length;){
		// I can use the fact that the incides are increasing in both arrays
		pair cur_pair_A = dynamic_array_at(pair, vector_A, i);
		pair cur_pair_B = dynamic_array_at(pair, vector_B, j);
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

void extract_expanded_all_rows_csc(const matrix_data* const matrix, float* const * const all_vectors){	
	// TODO it is impossible to build in the "is-the-row-empty?"-check here, unless I use some other bool field or sth
	for(uint64_t j = 0; j < matrix->dynamic_values.length; j++){
		size_t which_vector;
		size_t index_in_vector = 0;
		which_vector = dynamic_array_at(uint64_t, &matrix->dynamic_row_indices, j);
		while(j >= matrix->col_ptr[index_in_vector+1]){
			index_in_vector++; // scroll to the corect column
		}
		float value = dynamic_array_at(float, &matrix->dynamic_values, j);
		all_vectors[which_vector][index_in_vector] = value; // note that this array is always large enough
	}
}

size_t extract_expanded_row_csc(const matrix_data* const matrix, const uint64_t index, float* const vector){
	size_t number_of_elements = 0;
	for(uint64_t j = 0; j < matrix->dynamic_values.length; j++){
		if(dynamic_array_at(uint64_t, &matrix->dynamic_row_indices, j) == index){
			size_t index_in_vector = 0;
			for(size_t l = 0; l < matrix->no_rows+1; l++){
				if(matrix->col_ptr[l] <= j && matrix->col_ptr[l+1] > j){
					index_in_vector = l;				
					break;
				}
			}
			float value = dynamic_array_at(float, &matrix->dynamic_values, j);
			vector[index_in_vector] = value; // note that this array is always large enough
			number_of_elements++;
		}
	}
	return number_of_elements;
}

size_t extract_expanded_col_csc(const matrix_data* const matrix, const uint64_t index, float* const vector){
	size_t number_of_elements = 0;
	for(uint64_t j = matrix->col_ptr[index]; j < matrix->col_ptr[index+1]; j++){
		float value = dynamic_array_at(float, &matrix->dynamic_values, j);
		size_t index = dynamic_array_at(uint64_t, &matrix->dynamic_row_indices, j);
		vector[index] = value; // note that vector is always large enough
		number_of_elements++;
	}
	return number_of_elements;
}

error extract_compressed_row_csc(const matrix_data* const matrix, const uint64_t index, DYNAMIC_ARRAY(pair)* dynamic_vector){
	for(uint64_t j = 0; j < matrix->dynamic_values.length; j++){
		if(dynamic_array_at(uint64_t, &matrix->dynamic_row_indices, j) == index){
			pair new_pair;
			for(uint64_t l = 0; l < matrix->no_rows+1; l++){
				if(matrix->col_ptr[l] <= j && matrix->col_ptr[l+1] > j){
					new_pair.index = l;				
					break;
				}
			}
			new_pair.value = dynamic_array_at(float, &matrix->dynamic_values, j);
			if(dynamic_array_add(pair, dynamic_vector, new_pair)){
				fprintf(stderr, "Cannot add new element to dynamic_vector in function %s\n", __FUNCTION__);
				return OUT_OF_MEMORY;
			}
		}
	}
	return OK;
}

error extract_compressed_col_csc(const matrix_data* const matrix, const uint64_t index, DYNAMIC_ARRAY(pair)* dynamic_vector){
	for(uint64_t j = matrix->col_ptr[index]; j < matrix->col_ptr[index+1]; j++){
		pair new_pair = {
			.index = dynamic_array_at(uint64_t, &matrix->dynamic_row_indices, j),
			.value = dynamic_array_at(float, &matrix->dynamic_values, j)
		};
		if(dynamic_array_add(pair, dynamic_vector, new_pair)){
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
			uint64_t row = dynamic_array_at(uint64_t, &compressed->dynamic_row_indices, i);
			float value = dynamic_array_at(float, &compressed->dynamic_values, i);
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
