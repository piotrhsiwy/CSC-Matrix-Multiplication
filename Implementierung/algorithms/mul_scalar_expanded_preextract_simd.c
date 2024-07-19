#include "mul_scalar_expanded_preextract_simd.h"

__attribute__((hot)) void matrix_mul_scalar_expanded_preextract_simd(const void *const a, const void *const b, void *const restrict c) {
 
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
        matrix_C->mul_error = OK;
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

    // these are vectors for all the rows extracted from matrix A
    size_t vector_size = matrix_A->no_cols;
    float** all_vectors_A = malloc(sizeof(*all_vectors_A)* matrix_A->no_rows);
    if(!all_vectors_A){
        fprintf(stderr, "Cannot allocate memory for all_vectors_A field in function %s.\nThis could be due to key limitation of this implementation. See mul_scalar_expanded.h", __FUNCTION__);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    for(size_t i = 0; i < matrix_A->no_rows; i++){
        float* vector = malloc(sizeof(*vector) * vector_size);
        if(!vector){
            fprintf(stderr, "Cannot allocate memory for all_vectors_A[%lu] vector field in function %s.\nThis could be due to key limitation of this implementation. See mul_scalar_expanded.h", i, __FUNCTION__);
            nested_free(i, all_vectors_A);
            matrix_C->mul_error = OUT_OF_MEMORY;
            return;
        }
        memset(vector, 0, sizeof(*vector) * vector_size);
        all_vectors_A[i] = vector;
    }

    // this is vector for the col extracted from matrix B
    float* vector_B = malloc(sizeof(*vector_B) * vector_size);
    if(!vector_B){
        fprintf(stderr, "Cannot allocate memory for vector_B field in function %s\n", __FUNCTION__);
        nested_free(matrix_A->no_rows, all_vectors_A);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    memset(vector_B, 0, sizeof(*vector_B) * vector_size);

    // extract all rows from matrix A. Iterates over both values and col_ptr array only once. See description of the function in matrix_util.h
    extract_expanded_all_rows_csc(matrix_A, all_vectors_A);
        
    /*
     * iterator names *i* and *k* as on the graphic linked above. Short indices became more
     * intuitive for us since we've been working with the graphic at all times
     * */
    for(uint64_t k = 0; k < matrix_C->no_cols; k++){ // 

        // extract col from matrix B. See description of the function in matrix_util.h
        if(!extract_expanded_col_csc(matrix_B, k, vector_B)){ // the col is empty
            continue;  
        }

        for(uint64_t i = 0; i < matrix_C->no_rows; i++){
            
            // ##################################
            // MULTIPLY THE ROW WITH THE COLUMN AND WRITE THE RESULTS
            // ##################################
            float value = mul_vectors_expanded_SIMD(vector_size, all_vectors_A[i], vector_B);
            if(value != 0.0f){
                pair new_pair = {.index = i, .value = value};
                if(dynamic_array_add(pair, &matrix_C->dynamic_values_x_row_indices, new_pair)){
                    fprintf(stderr, "Cannot add new element to dynamic_values_x_row_indices in function %s\n", __FUNCTION__);
                    nested_free(matrix_A->no_rows, all_vectors_A);
                    free(vector_B);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
            }
            // else we don't write anything
        }
        *col_ptr_writer = matrix_C->dynamic_values_x_row_indices.length;
        col_ptr_writer++;
        memset(vector_B, 0, sizeof(*vector_B) * vector_size); // reset vector_B
    }

    nested_free(matrix_A->no_rows, all_vectors_A);
    free(vector_B);
    matrix_C->mul_error = OK;
    return;
}
