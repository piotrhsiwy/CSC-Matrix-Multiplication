#include "mul_gustavson.h"

__attribute__((hot)) void matrix_vector(const matrix_data* const matrix_A, const matrix_data* const matrix_B, matrix_data* const matrix_C){
 
    // ##################################
    // ALLOCATE THE FIELDS OF THE RESULT MATRIX
    // ##################################
    // all malloc'd pointers can be passed to free (alloc'd or null)
    matrix_C->no_rows = matrix_A->no_rows;
    matrix_C->no_cols = matrix_B->no_cols;
    matrix_C->col_ptr = malloc(sizeof(*matrix_C->col_ptr) * ( matrix_C->no_cols + 1 ));
    if(!(matrix_C->col_ptr)){
        fprintf(stderr, "Out of memory for col_ptr field in function %s\n", __FUNCTION__);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    if(dynamic_array_new(float, 4, &matrix_C->dynamic_values)){
        fprintf(stderr, "Cannot dynamically allocate memory for dynamic_values field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    if(dynamic_array_new(uint64_t, 4, &matrix_C->dynamic_row_indices)){
        fprintf(stderr, "Cannot dynamically allocate memory for dynamic_row_indices field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }

    // ##################################
    // CATCH ERRORS
    // ##################################
        // matrix size mismatch
    if(matrix_A->no_cols != matrix_B->no_rows){
        fprintf(stderr, "Matrix size mismatch\n");
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
        matrix_C->mul_error = DIMENSION_MISMATCH;
        return;
    }
        // one of the matrices has length_values == 0. That means all values in the result matrix are also 0;
    if(matrix_A->dynamic_values.length == 0 || matrix_B->dynamic_values.length == 0){
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
    DYNAMIC_ARRAY(float) vcolA;
    if(dynamic_array_new(float, 4, &vcolA)){
        fprintf(stderr, "Cannot dynamically allocate memory for colA field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    // this is dynamic-array for the col extracted from matrix B
    DYNAMIC_ARRAY(pair) colB;
    if(dynamic_array_new(pair, 4, &colB)){
        fprintf(stderr, "Cannot dynamically allocate memory for colB field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
        dynamic_array_destroy((generic_dynamic_array*)&vcolA);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }

    for(uint64_t col_index = 0; col_index < matrix_B->no_cols; col_index++){
        // extract csc column k from matrix_B
        { 
            error error = extract_compressed_col_csc(matrix_B, col_index, &colB);
            if(error != OK){
                free(matrix_C->col_ptr);
                dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                dynamic_array_destroy((generic_dynamic_array*)&vcolA);
                dynamic_array_destroy((generic_dynamic_array*)&colB);
                matrix_C->mul_error = error;
                return;
            }
        }
        // foreach pair extract the correct (pair.index-th) col from matrix_A and scalar multiply with the value (pair.value)
        // note that only values are extracted from the col, so the resulting vector is dynamic array of floats
        for(uint64_t i = 0; i < colB.length; i++){
            uint64_t row_index = dynamic_array_at(pair, &colB, i).index;
            {
                error error = extract_vcol_csc(matrix_A, row_index, &vcolA);
                if(error != OK){
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&vcolA);
                    dynamic_array_destroy((generic_dynamic_array*)&colB);
                    matrix_C->mul_error = error;
                    return;
                }
            }
            float value = vcolA.length ?
               mul_collapse_vector_expanded_w_scalar(&vcolA, dynamic_array_at(pair, &colB, i).value)
               : 0.0f;
            if(value != 0.0f){ // write only non-zero values
                if(dynamic_array_add(float, &matrix_C->dynamic_values, value)){
                    fprintf(stderr, "Cannot add new element to dynamic_values in function %s\n", __FUNCTION__);
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&vcolA);
                    dynamic_array_destroy((generic_dynamic_array*)&colB);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
                if(dynamic_array_add(uint64_t, &matrix_C->dynamic_row_indices, row_index)){
                    fprintf(stderr, "Cannot add new element to dynamic_row_indices in function %s\n", __FUNCTION__);
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&vcolA);
                    dynamic_array_destroy((generic_dynamic_array*)&colB);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
            }
            // else if value==0.0f we don't write it
            vcolA.length = 0; // reset the value col from A
        }
        colB.length = 0; // reset the compressed col from B
    }
    dynamic_array_destroy((generic_dynamic_array*)&vcolA);
    dynamic_array_destroy((generic_dynamic_array*)&colB);
    matrix_C->mul_error = OK; 
}
