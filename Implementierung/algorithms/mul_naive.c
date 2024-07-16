#include "mul_naive.h"

__attribute__((hot)) void matrix_mul_naive(const matrix_data* const a, const matrix_data* const b, matrix_data* const result){
    result->no_rows = a->no_rows;
    result->no_cols = b->no_cols;
    result->col_ptr = malloc(sizeof(*result->col_ptr) * ( result->no_cols + 1 ));
    if(!(result->col_ptr)){
        fprintf(stderr, "Out of memory for col_ptr field in function %s\n", __FUNCTION__);
        result->mul_error = OUT_OF_MEMORY;
        return;
    }
    if(dynamic_array_new(pair, 4, &result->dynamic_values_x_row_indices)){
        fprintf(stderr, "Cannot dynamically allocate memory for dynamic_values_x_row_indices field in function %s\n", __FUNCTION__);
        free(result->col_ptr);
        result->mul_error = OUT_OF_MEMORY;
        return;
    }

    float** expanded_A = malloc(( a->no_cols ) * sizeof(*expanded_A));
    if(!expanded_A){
        fprintf(stderr, "Cannot allocate memmory for field expanded_A in function %s\n", __FUNCTION__);
        free(result->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&result->dynamic_values_x_row_indices);
        result->mul_error = OUT_OF_MEMORY;
        return;
    }
    for (size_t col = 0; col < a->no_cols; col++) {
        float* temp = malloc(( a->no_rows ) * sizeof(*temp));
        if(!temp){
            fprintf(stderr, "Cannot allocate memmory for field expanded_A[%lu] in function %s\n", col, __FUNCTION__);
            free(result->col_ptr);
            dynamic_array_destroy((generic_dynamic_array*)&result->dynamic_values_x_row_indices);
            nested_free(col, expanded_A);
            result->mul_error = OUT_OF_MEMORY;
            return;
        }
        expanded_A[col] = temp;
    }
    
    float** expanded_B = malloc(( b->no_cols * b->no_rows )* sizeof(*expanded_B));
    if(!expanded_B){
        fprintf(stderr, "Cannot allocate memmory for field expanded_B in function %s\n", __FUNCTION__);
        free(result->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&result->dynamic_values_x_row_indices);
        nested_free(a->no_cols, expanded_A);
        result->mul_error = OUT_OF_MEMORY;
        return;
    }
    for (size_t col = 0; col < b->no_cols; col++) {
        float* temp = malloc(( b->no_rows ) * sizeof(*temp));
        if(!temp){
            fprintf(stderr, "Cannot allocate memmory for field expanded_B[%lu] in function %s\n", col, __FUNCTION__);
            free(result->col_ptr);
            dynamic_array_destroy((generic_dynamic_array*)&result->dynamic_values_x_row_indices);
            nested_free(a->no_cols, expanded_A);
            nested_free(col, expanded_B);
            result->mul_error = OUT_OF_MEMORY;
            return;
        }
        expanded_B[col] = temp;
    }

    float** expanded_result = malloc(( result->no_cols * result->no_rows )* sizeof(float));
    if(!expanded_result){
        fprintf(stderr, "Cannot allocate memmory for field expanded_result in function %s\n", __FUNCTION__);
        free(result->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&result->dynamic_values_x_row_indices);
        nested_free(a->no_cols, expanded_A);
        nested_free(b->no_cols, expanded_B);
        result->mul_error = OUT_OF_MEMORY;
        return;
    }
    for (size_t col = 0; col < result->no_cols; col++) {
        float* temp = malloc(( result->no_rows ) * sizeof(*temp));
        if(!temp){
            fprintf(stderr, "Cannot allocate memmory for field expanded_result[%lu] in function %s\n", col, __FUNCTION__);
            free(result->col_ptr);
            dynamic_array_destroy((generic_dynamic_array*)&result->dynamic_values_x_row_indices);
            nested_free(a->no_cols, expanded_A);
            nested_free(b->no_cols, expanded_B);
            nested_free(col, expanded_result);
            result->mul_error = OUT_OF_MEMORY;
            return;
        }
        expanded_result[col] = temp;
    }

    expand_matrix(a, expanded_A);
    expand_matrix(b, expanded_B);

    for (size_t col = 0; col < result->no_cols; col++)
    {
        for (size_t row = 0; row < result->no_rows; row++)
        {
            float acc = 0.0f;
            for (size_t k = 0; k < a->no_cols; k++)
            {
                acc += expanded_A[k][row] * expanded_B[col][k];
            }
            expanded_result[col][row] = acc;
        }
    }

    {
        error error = compress_matrix(expanded_result, result);
        if(error != OK){
            nested_free(a->no_cols, expanded_A);
            nested_free(b->no_cols, expanded_B);
            nested_free(result->no_cols, expanded_result);
            result->mul_error = error;
            return;
        }
    }

    nested_free(a->no_cols, expanded_A);
    nested_free(b->no_cols, expanded_B);
    nested_free(result->no_cols, expanded_result);
    result->mul_error = OK;
    return;
}