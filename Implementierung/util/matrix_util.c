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

// TODO this can be done with SIMD faster?
// TODO should we return any erorrs here?
void collapse_matrix_horizontal(const size_t m, const size_t n, float* const vector, float** const matrix){
	// note the order of for-loops
	memset(vector, 0, n * sizeof(*vector));
	for(size_t i = 0; i < m; i++){
		for(size_t j = 0; j < n; j++){
			vector[j] += matrix[i][j];
		}
	}
}

static void swap(const size_t i, const size_t j, pair arr[]){
	pair temp = arr[i];
	arr[i] = arr[j];
	arr[j] = temp;
}

// TODO optimize this
static void sort(const size_t count, pair arr[count]){
	for (size_t i = 0; i < count; i++)
	{
		pair min = arr[i];
		size_t min_index = i;
		for (size_t j = i+1; j < count; j++)
		{
			if(arr[j].index < min.index)
			{
				min = arr[j];
				min_index = j;
			}
		}
		if(i != min_index){
			swap(i, min_index, arr);
		}
	}
}

error sum_compressed_matrix(DYNAMIC_ARRAY_2D* const vector_2d, DYNAMIC_ARRAY(pair)* const result){
	if(!vector_2d->length){
		// no vectors to sum, return empty array
		return OK;
	}

	result->length = 0;
	size_t index_in_vector[vector_2d->length];
	memset(index_in_vector, 0, vector_2d->length * sizeof(size_t));
	size_t how_many_have_values_left = vector_2d->length;

	size_t lowest_index = 0;
	while (true)
	{
		if(!how_many_have_values_left)
			break;
		float acc = 0.0f;
		for (size_t vector_index = 0; vector_index < vector_2d->length; vector_index++)
		{
			DYNAMIC_ARRAY(pair) cur_vector = dynamic_array_at(___dynamic_array_pair_macro_defined, vector_2d, vector_index);
			if(index_in_vector[vector_index] != cur_vector.length)
			{ /* there are values left in this vector */
				pair cur_pair = dynamic_array_at(pair, &cur_vector, index_in_vector[vector_index]);
				if(cur_pair.index == lowest_index){
					/* belongs to the currently accumulated value */
					acc += cur_pair.value;
					index_in_vector[vector_index]++;
					if(index_in_vector[vector_index] >= cur_vector.length)
						how_many_have_values_left--;
				} else{
					/* do nothing if index is too big (it will be never be lower, since pairs are sorted by increasing indices in vectors) */
				}
			}	
		}
		if(acc != 0.0f)
		{
			pair new_pair = {.index = lowest_index, .value = acc};
			if(dynamic_array_add(pair, result, new_pair))
				return OUT_OF_MEMORY;
		}
		lowest_index++;
	}

	return OK;	
}
// alternatively scatter to an extended vector and then add up

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

void mul_vector_compressed_w_scalar(DYNAMIC_ARRAY(pair)* vector, const float scalar){
	for (size_t i = 0; i < vector->length; i++) {
		pair cur_pair = dynamic_array_at(pair, vector, i);
		cur_pair.value = cur_pair.value * scalar;
		dynamic_array_at(pair, vector, i) = cur_pair;
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
	for(uint64_t j = 0; j < matrix->dynamic_values_x_row_indices.length; j++) {
		size_t index_in_vector = 0;
		while(( index_in_vector < matrix->no_cols+1 ) && ( j >= matrix->col_ptr[index_in_vector + 1] )){
            index_in_vector++; // scroll to the correct column
        }

        size_t which_vector = dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, j).index;
        float value = dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, j).value;
		all_vectors[which_vector][index_in_vector] = value; // note that this array is always large enough
	}
}

size_t extract_expanded_row_csc(const matrix_data* const matrix, const uint64_t index, float* const vector){
	size_t number_of_elements = 0;
	for(uint64_t j = 0; j < matrix->dynamic_values_x_row_indices.length; j++){
		if(dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, j).index == index){
			size_t index_in_vector = 0;
			for(size_t l = 0; l < matrix->no_rows+1; l++){
				if(matrix->col_ptr[l] <= j && matrix->col_ptr[l+1] > j){
					index_in_vector = l;				
					break;
				}
			}
			float value = dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, j).value;
			vector[index_in_vector] = value; // note that this array is always large enough
			number_of_elements++;
		}
	}
	return number_of_elements;
}

size_t extract_expanded_col_csc(const matrix_data* const matrix, const uint64_t index, float* const vector){
	size_t number_of_elements = 0;
	for(uint64_t j = matrix->col_ptr[index]; j < matrix->col_ptr[index+1]; j++){
		pair pair = dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, j);
		float value = pair.value;
		size_t index = pair.index;
		vector[index] = value; // note that vector is always large enough
		number_of_elements++;
	}
	return number_of_elements;
}

error extract_compressed_row_csc(const matrix_data* const matrix, const uint64_t index, DYNAMIC_ARRAY(pair)* dynamic_vector){
	for(uint64_t j = 0; j < matrix->dynamic_values_x_row_indices.length; j++){
		if(dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, j).index == index){
			pair new_pair;
			for(uint64_t l = 0; l < matrix->no_rows+1; l++){
				if(matrix->col_ptr[l] <= j && matrix->col_ptr[l+1] > j){
					new_pair.index = l;				
					break;
				}
			}
			new_pair.value = dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, j).value;
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
		pair new_pair = dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, j);
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
	for(uint64_t col = 0; col < compressed->no_cols; col++){
		extract_expanded_col_csc(compressed, col, expanded[col]);
	}
}

static error compress_vector(const uint64_t count, const float vector[count], DYNAMIC_ARRAY(pair)* const compressed){
	for (uint64_t i = 0; i < count; i++)
	{
		pair new_pair = {.index = i, .value = vector[i]};
		if(new_pair.value && dynamic_array_add(pair, compressed, new_pair))
		{
			fprintf(stderr, "Cannot add an element to dynamic array compressed in function %s\n", __FUNCTION__);
			return OUT_OF_MEMORY;
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
			error error = compress_vector(compressed->no_rows, expanded[col], &compressed->dynamic_values_x_row_indices);
			if(error != OK){
				return error;
			}
		}
		*col_ptr_writer++ = compressed->dynamic_values_x_row_indices.length;
	}
	return OK;
}
