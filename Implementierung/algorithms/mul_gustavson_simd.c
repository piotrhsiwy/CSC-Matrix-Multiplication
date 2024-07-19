#include "mul_gustavson.h"

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
    if(matrix_A->values.length == 0 || matrix_B->values.length == 0){
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

    // this is dynamic-array for the values from the col extracted from matrix A
    DYNAMIC_ARRAY_2D_float colA_values_vector;
    if(dynamic_array_new(___dynamic_array_float_macro_defined, 4, &colA_values_vector)){
        fprintf(stderr, "Cannot dynamically allocate memory for colA_values_vector in function %s\n", __FUNCTION__);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    // this is dynamic-array for the indices from the col extracted from matrix A
    DYNAMIC_ARRAY_2D_uint64_t colA_indices_vector;
    if(dynamic_array_new(___dynamic_array_uint64_t_macro_defined, 4, &colA_indices_vector)){
        fprintf(stderr, "Cannot dynamically allocate memory for colA_indices_vector in function %s\n", __FUNCTION__);
        dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    // this is dynamic-array for the values of the col extracted from matrix B
    DYNAMIC_ARRAY(float) colB_values;
    if(dynamic_array_new(float, 4, &colB_values)){
        fprintf(stderr, "Cannot dynamically allocate memory for colB_values field in function %s\n", __FUNCTION__);
        dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
        dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    // this is dynamic-array for the indices of the col extracted from matrix B
    DYNAMIC_ARRAY(uint64_t) colB_indices;
    if(dynamic_array_new(uint64_t, 4, &colB_indices)){
        fprintf(stderr, "Cannot dynamically allocate memory for colB_indices field in function %s\n", __FUNCTION__);
        dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
        dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
        dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }

    for(uint64_t col_index = 0; col_index < matrix_B->no_cols; col_index++){
        // extract csc column k from matrix_B
        { 
        error error = extract_compressed_col_csc(matrix_B, col_index, &colB_values, &colB_indices);
        if(error != OK){
            dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
            dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
            dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
            dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_indices);
            matrix_C->mul_error = error;
            return;
        }
        }
        // foreach pair extract the correct (pair.index-th) col from matrix_A and scalar multiply with the value (pair.value)
        // note that only values are extracted from the col, so the resulting vector is dynamic array of floats
        for(uint64_t i = 0; i < colB_values.length; i++){
            uint64_t row_index = dynamic_array_at(uint64_t, &colB_indices, i);

            // allocate temporary buffers for the values and indices to be used when extracting cols from A (note that we extract only those cols that are required for the multiplication == under indexes where colB does not have value 0.0f), column by column
            // they have to be _new'd on every run, because we possibly (if col no empty) will have to allocate new ones for every extract column
            DYNAMIC_ARRAY(float) new_col_values = {.data = 0, .length = 0, .capacity = 0}; // to @ignore 'use of uinitialized value when reallocating in dynamic_array_new'
            if(dynamic_array_new(float, 4, &new_col_values)){
                fprintf(stderr, "Cannot dynamically allocate memory for new_col_values field in function %s\n", __FUNCTION__);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_indices);
                matrix_C->mul_error = OUT_OF_MEMORY;
                return;
            }
            DYNAMIC_ARRAY(uint64_t) new_col_indices = {.data = 0, .length = 0, .capacity = 0}; // to @ignore 'use of uinitialized value when reallocating in dynamic_array_new'
            if(dynamic_array_new(uint64_t, 4, &new_col_indices)){
                fprintf(stderr, "Cannot dynamically allocate memory for new_col_values field in function %s\n", __FUNCTION__);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_indices);
                dynamic_array_destroy((generic_dynamic_array*)&new_col_values);
                matrix_C->mul_error = OUT_OF_MEMORY;
                return;
            }

            //extract the column under index row_index from A
            {
            error error = extract_compressed_col_csc(matrix_A, row_index, &new_col_values, &new_col_indices);
            if(error != OK){
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
                dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_indices);
                dynamic_array_destroy((generic_dynamic_array*)&new_col_values);
                dynamic_array_destroy((generic_dynamic_array*)&new_col_indices);
                matrix_C->mul_error = error;
                return;
            }
            }
            if(new_col_values.length) {
                // add it to the 2D dynamic_arrays if the column is not empty. Note that new_col_values.length === new_col_indices.length
                if(dynamic_array_add(___dynamic_array_float_macro_defined, &colA_values_vector, new_col_values)){
                    fprintf(stderr, "Cannot add new element to colA_values_vector in function %s\n", __FUNCTION__);
                    dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
                    dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
                    dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
                    dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&new_col_values);
                    dynamic_array_destroy((generic_dynamic_array*)&new_col_indices);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
                if(dynamic_array_add(___dynamic_array_uint64_t_macro_defined, &colA_indices_vector, new_col_indices)){
                    fprintf(stderr, "Cannot add new element to colA_indices_vector in function %s\n", __FUNCTION__);
                    dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
                    dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
                    dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
                    dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&new_col_values);
                    dynamic_array_destroy((generic_dynamic_array*)&new_col_indices);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
                mul_compressed_values_vector_w_scalar_SIMD(&new_col_values, dynamic_array_at(float, &colB_values, i));
                dynamic_array_unbind((generic_dynamic_array*)&new_col_values);
                dynamic_array_unbind((generic_dynamic_array*)&new_col_indices);
            } else{
                /* else new_col_values and hence also _indices is empty */
                dynamic_array_destroy((generic_dynamic_array*)&new_col_values);
                dynamic_array_destroy((generic_dynamic_array*)&new_col_indices);
            }        
        }
        {
        // process the multiplied "matrix" aka 2D dynamic arrays colA_values_vector and colA_indices_vector. See matrix_util.h
        colB_values.length = 0;
        colB_indices.length = 0;
        error error = sum_compressed_matrix(&colA_values_vector, &colA_indices_vector, &colB_values, &colB_indices);
        if (error != OK) {
            dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
            dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
            dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
            dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_indices);
            matrix_C->mul_error = error;
            return;
        } 
        }
        // concatenate the resulting !col! of the result matrix into its' dynamic arrays
        dynamic_array_concat(float, &matrix_C->values, &colB_values);
        dynamic_array_concat(uint64_t, &matrix_C->row_indices, &colB_indices);

        dynamic_array_destroy_top_layer((generic_dynamic_array*)&colA_values_vector); // scratch the extracted columns from A
        dynamic_array_destroy_top_layer((generic_dynamic_array*)&colA_indices_vector);
        
        *col_ptr_writer = matrix_C->values.length;
        col_ptr_writer++;
        colB_values.length = 0; // reset the compressed col from B
    }
    dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_values_vector);
    dynamic_array_recursive_destroy((generic_dynamic_array*)&colA_indices_vector);
    dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_values);
    dynamic_array_recursive_destroy((generic_dynamic_array*)&colB_indices);
    matrix_C->mul_error = OK; 
}
