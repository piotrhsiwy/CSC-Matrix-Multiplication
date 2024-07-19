#include "mul_scalar.h"


__attribute__((hot)) void matrix_mul_scalar(const void *const a, const void *const b, void *const restrict c) {
    const matrix_data* const matrix_A = (const matrix_data* const) a;
    const matrix_data* const matrix_B = (const matrix_data* const) b;
    matrix_data* const matrix_C = (matrix_data* const) c;

    matrix_C->mul_error = OK; //If we don't set any errors later the multiplication was successful.

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
        memset(matrix_C->col_ptr, 0, (matrix_C->no_cols + 1) * sizeof *matrix_C->col_ptr);
        return;
    }

    // ##################################
    // EXTRACT THE ROW FROM MATRIX A AND THE COLUMN FROM MATRIX B
    // ##################################
    // indices as on this graphic: https://de.wikipedia.org/wiki/Matrizenmultiplikation#/media/Datei:Matrix_multiplication_qtl2.svg
    // I iterate over columns first (in the outer loop), because that makes it easier to write to result matrix. This is the result of using CSC.
    uint64_t* col_ptr_writer = matrix_C->col_ptr; // use this to write to col_ptr array
    *col_ptr_writer = 0; // initialize col_ptr
    col_ptr_writer++;

    // this is dynamic vector for the row extracted from matrix A
    DYNAMIC_ARRAY(float) values_A;
    if(dynamic_array_new(float, 4, &values_A)){
        fprintf(stderr, "Cannot dynamically allocate memory for values_A field in function %s\n", __FUNCTION__);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    DYNAMIC_ARRAY(uint64_t) indices_A;
    if(dynamic_array_new(uint64_t, 4, &indices_A)){
        fprintf(stderr, "Cannot dynamically allocate memory for indices_A field in function %s\n", __FUNCTION__);
        dynamic_array_destroy((generic_dynamic_array*)&values_A);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    // this is dynamic vector for the col extracted from matrix B
    DYNAMIC_ARRAY(float) values_B;
    if(dynamic_array_new(float, 4, &values_B)){
        fprintf(stderr, "Cannot dynamically allocate memory for values_B field in function %s\n", __FUNCTION__);
        dynamic_array_destroy((generic_dynamic_array*)&values_A);
        dynamic_array_destroy((generic_dynamic_array*)&indices_A);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    DYNAMIC_ARRAY(uint64_t) indices_B;
    if(dynamic_array_new(uint64_t, 4, &indices_B)){
        fprintf(stderr, "Cannot dynamically allocate memory for indices_B field in function %s\n", __FUNCTION__);
        dynamic_array_destroy((generic_dynamic_array*)&values_A);
        dynamic_array_destroy((generic_dynamic_array*)&indices_A);
        dynamic_array_destroy((generic_dynamic_array*)&values_B);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }

    /*
     * iterator names *k* and *i* as on the graphic linked above. Short indices became more
     * intuitive for us since we've been working with the graphic at all times anyway
     * */
    for(uint64_t k = 0; k < matrix_C->no_cols; k++){ // 

        // extract column from matrix B. See description of the function in matrix_util.h
        {
            error error = extract_compressed_col_csc(matrix_B, k, &values_B, &indices_B);
            if(error != OK){
                dynamic_array_destroy((generic_dynamic_array*)&values_A);
                dynamic_array_destroy((generic_dynamic_array*)&indices_A);
                dynamic_array_destroy((generic_dynamic_array*)&values_B);
                dynamic_array_destroy((generic_dynamic_array*)&indices_B);
                matrix_C->mul_error = error;
                return;
            }
        }
        if(values_B.length == 0){ // the col is empty
            continue;
        }

        // optimization idea: map all the pairs from matrix A (for all row-numbers) with one run, because for now I have to go through the whole value-length array for every row!
        // note that this comes with limitation by matrix_C->no_cols
        // this optimization  idea has been implemented in _preextract algorithm
        for(uint64_t i = 0; i < matrix_C->no_rows; i++){
            
            // extract row from matrix A. See description of the function in matrix_util.h
            {
                error error = extract_compressed_row_csc(matrix_A, i, &values_A, &indices_A);
                if(error != OK){
                    dynamic_array_destroy((generic_dynamic_array*)&values_A);
                    dynamic_array_destroy((generic_dynamic_array*)&indices_A);
                    dynamic_array_destroy((generic_dynamic_array*)&values_B);
                    dynamic_array_destroy((generic_dynamic_array*)&indices_B);
                    matrix_C->mul_error = error;
                    return;
                }
            }
            if(values_A.length == 0){
                continue;
            }

            // ##################################
            // MULTIPLY THE ROW WITH THE COLUMN AND WRITE THE RESULTS
            // ##################################
            float value = mul_vectors_compressed(&values_A, &indices_A, &values_B, &indices_B);
            if(value != 0.0f){
                if(dynamic_array_add(float, &matrix_C->values, value)){
                    fprintf(stderr, "Cannot add new element to values in function %s\n", __FUNCTION__);
                    dynamic_array_destroy((generic_dynamic_array*)&values_A);
                    dynamic_array_destroy((generic_dynamic_array*)&indices_A);
                    dynamic_array_destroy((generic_dynamic_array*)&values_B);
                    dynamic_array_destroy((generic_dynamic_array*)&indices_B);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
                if(dynamic_array_add(uint64_t, &matrix_C->row_indices, i)){
                    fprintf(stderr, "Cannot add new element to row_indices in function %s\n", __FUNCTION__);
                    dynamic_array_destroy((generic_dynamic_array*)&values_A);
                    dynamic_array_destroy((generic_dynamic_array*)&indices_A);
                    dynamic_array_destroy((generic_dynamic_array*)&values_B);
                    dynamic_array_destroy((generic_dynamic_array*)&indices_B);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
            }
            // else we don't write anything
            values_A.length = 0; // "empty" the vector holding the row
            indices_A.length = 0;
        }
        *col_ptr_writer = matrix_C->values.length;
        col_ptr_writer++;
        values_B.length = 0; // "empty" the vector holding the row
        indices_B.length = 0;
    }

    dynamic_array_destroy((generic_dynamic_array*)&values_A);
    dynamic_array_destroy((generic_dynamic_array*)&indices_A);
    dynamic_array_destroy((generic_dynamic_array*)&values_B);
    dynamic_array_destroy((generic_dynamic_array*)&indices_B);
    return;
}
