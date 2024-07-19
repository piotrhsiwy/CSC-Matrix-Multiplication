#include "mul_gustavson_simd.h"

__attribute__((hot)) void matrix_mul_gustavson_simd(const void *const a, const void *const b, void *const restrict c){

    const matrix_data* const matrix_A = (const matrix_data* const) a;
    const matrix_data* const matrix_B = (const matrix_data* const) b;
    matrix_data* const matrix_C = (matrix_data* const) c;

    // ##################################
    // CATCH ERRORS
    // ##################################
        // matrix size mismatch
    if(matrix_A->no_cols != matrix_B->no_rows){
        fprintf(stderr, "Matrix size mismatch\n");
        matrix_C->mul_error = DIMENSION_MISMATCH;
        return;
    }
        // one of the matrices has length_values == 0. That means all values in the result matrix are also 0;
    if(matrix_A->dynamic_values_x_row_indices.length == 0 || matrix_B->dynamic_values_x_row_indices.length == 0){
        memset(matrix_C->col_ptr, 0, matrix_C->no_cols+1);
        return;
    }

    // ##################################
    // EXTRACT ROWS AND COLS FROM THE MATRICES
    // ##################################
    // indices as on this graphic: https://de.wikipedia.org/wiki/Matrizenmultiplikation#/media/Datei:Matrix_multiplication_qtl2.svg
    // I iterate over columns first (in the outer loop), because that makes it easier to write to result matrix. This is the result of using CSC.
    uint64_t* col_ptr_writer = matrix_C->col_ptr; // use this to write to col_ptr array
    *col_ptr_writer = 0; // initialize col_ptr
    col_ptr_writer++;
    uint64_t values_index = 0; // use this to write to values and row_indices arrays

    // this is dynamic-array for the values from the col extracted from matrix A
    DYNAMIC_ARRAY_2D colA_vector;
    if(dynamic_array_new(___dynamic_array_pair_macro_defined, 4, &colA_vector)){
        fprintf(stderr, "Cannot dynamically allocate memory for colA_vector in function %s\n", __FUNCTION__);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    // this is dynamic-array for the col extracted from matrix B
    DYNAMIC_ARRAY(pair) colB;
    if(dynamic_array_new(pair, 4, &colB)){
        fprintf(stderr, "Cannot dynamically allocate memory for colB field in function %s\n", __FUNCTION__);
        dynamic_array_recursive_destroy(&colA_vector);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }

    for(uint64_t col_index = 0; col_index < matrix_B->no_cols; col_index++){
        // extract csc column k from matrix_B
        { 
            error error = extract_compressed_col_csc(matrix_B, col_index, &colB);
            if(error != OK){
                dynamic_array_recursive_destroy(&colA_vector);
                dynamic_array_destroy((generic_dynamic_array*)&colB);
                matrix_C->mul_error = error;
                return;
            }
        }
        // foreach pair extract the correct (pair.index-th) col from matrix_A and scalar multiply with the value (pair.value)
        // note that only values are extracted from the col, so the resulting vector is dynamic array of floats
        for(uint64_t i = 0; i < colB.length; i++){
            uint64_t row_index = dynamic_array_at(pair, &colB, i).index;
            DYNAMIC_ARRAY(pair) new_col = {.data = 0, .length = 0, .capacity = 0}; // to @ignore 'use of uinitialized value when reallocating in dynamic_array_new'
            if(dynamic_array_new(pair, 4, &new_col)){
                fprintf(stderr, "Cannot dynamically allocate memory for new_col field in function %s\n", __FUNCTION__);
                dynamic_array_recursive_destroy(&colA_vector);
                dynamic_array_destroy((generic_dynamic_array*)&colB);
                matrix_C->mul_error = OUT_OF_MEMORY;
                return;
            }
            {
                error error = extract_compressed_col_csc(matrix_A, row_index, &new_col);
                if(error != OK){
                    dynamic_array_recursive_destroy(&colA_vector);
                    dynamic_array_destroy((generic_dynamic_array*)&colB);
                    dynamic_array_destroy((generic_dynamic_array*)&new_col);
                    matrix_C->mul_error = error;
                    return;
                }
            }
            if(new_col.length) {
                if(dynamic_array_add(___dynamic_array_pair_macro_defined, &colA_vector, new_col)){
                    fprintf(stderr, "Cannot add new element to colA_vector in function %s\n", __FUNCTION__);
                    dynamic_array_recursive_destroy(&colA_vector);
                    dynamic_array_destroy((generic_dynamic_array*)&colB);
                    dynamic_array_destroy((generic_dynamic_array*)&new_col);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
                dynamic_array_unbind((generic_dynamic_array*)&new_col);
                DYNAMIC_ARRAY(pair) colA = dynamic_array_at(___dynamic_array_pair_macro_defined, &colA_vector, colA_vector.length-1);
                mul_vector_compressed_w_scalar_SIMD(&colA, dynamic_array_at(pair, &colB, i).value);
            } else{
                dynamic_array_destroy((generic_dynamic_array*)&new_col);
                /* else new_col is empty */

            }        
        }
        {
            error error = sum_compressed_matrix(&colA_vector, &colB);
            if (error != OK) {
                dynamic_array_recursive_destroy(&colA_vector);
                dynamic_array_destroy((generic_dynamic_array*)&colB);
                matrix_C->mul_error = error;
                return;
            } 
        }
        dynamic_array_concat(pair, &matrix_C->dynamic_values_x_row_indices, &colB);

        // else if value==0.0f we don't write it
        dynamic_array_destroy_top_layer(&colA_vector); // scratch the extracted columns from A
        
        *col_ptr_writer = matrix_C->dynamic_values_x_row_indices.length;
        col_ptr_writer++;
        colB.length = 0; // reset the compressed col from B
    }
    dynamic_array_recursive_destroy(&colA_vector);
    dynamic_array_destroy((generic_dynamic_array*)&colB);
    matrix_C->mul_error = OK; 
}
