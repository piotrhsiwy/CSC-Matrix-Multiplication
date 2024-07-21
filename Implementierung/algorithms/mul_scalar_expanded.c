#include "mul_scalar_expanded.h"

__attribute__((hot)) void matrix_mul_scalar_expanded(const void *const a, const void *const b, void *const restrict c) {

    const matrix_data* const matrix_A = (const matrix_data* const) a;
    const matrix_data* const matrix_B = (const matrix_data* const) b;
    matrix_data* const matrix_C = (matrix_data* const) c;

    matrix_C->mul_error = OK; //If we don't set an error late the multiplication was successful.

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
    // EXTRACT THE ROW FROM MATRIX A AND THE COLUMN FROM MATRIX B
    // ##################################
    // indices as on this graphic: https://de.wikipedia.org/wiki/Matrizenmultiplikation#/media/Datei:Matrix_multiplication_qtl2.svg
    // I iterate over columns first (in the outer loop), because that makes it easier to write to result matrix. This is the result of using CSC.
    uint64_t* col_ptr_writer = matrix_C->col_ptr; // use this to write to col_ptr array
    *col_ptr_writer = 0; // initialize col_ptr
    col_ptr_writer++;

    // this is vector for the row extracted from matrix A
    size_t vector_size = matrix_A->no_cols;
    float* vector_A = malloc(sizeof(*vector_A) * vector_size);
    if(!vector_A){
        fprintf(stderr, "Cannot allocate memory for vector_A field in function %s.\nThis could be due to key limitation of this implementation. See mul_scalar_expanded.h", __func__);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    memset(vector_A, 0, sizeof(*vector_A) * vector_size);
    // this is vector for the col extracted from matrix B
    float* vector_B = malloc(sizeof(*vector_B) * vector_size);
    if(!vector_B){
        fprintf(stderr, "Cannot allocate memory for vector_B field in function %s\n", __func__);
        free(vector_A);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    memset(vector_B, 0, sizeof(*vector_B) * vector_size);

    /*
     * iterator names *k* and *i* as on the graphic linked above. Short indices became more
     * intuitive for us since we've been working with the graphic at all times anyway
     * */
    for(uint64_t k = 0; k < matrix_C->no_cols; k++){ // 

        // extract column from matrix B. See description of the function in matrix_util.h
        if(!extract_expanded_col_csc(matrix_B, k, vector_B)){ // the col is empty
            continue;  
        }

        for(uint64_t i = 0; i < matrix_C->no_rows; i++){
            
            // extract row from matrix A. See description of the function in matrix_util.h
            if(!extract_expanded_row_csc(matrix_A, i, vector_A)){ // the row is empty
                continue;  
            }

            // ##################################
            // MULTIPLY THE ROW WITH THE COLUMN AND WRITE THE RESULTS
            // ##################################
            float value = mul_vectors_expanded(vector_size, vector_A, vector_B);
            if(value != 0.0f){
                if(dynamic_array_add(float, &matrix_C->values, value)){
                    fprintf(stderr, "Cannot add new element to values in function %s\n", __func__);
                    free(vector_A);
                    free(vector_B);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
                if(dynamic_array_add(uint64_t, &matrix_C->row_indices, i)){
                    fprintf(stderr, "Cannot add new element to row_indices in function %s\n", __func__);
                    free(vector_A);
                    free(vector_B);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
            }
            // else we don't write anything
            memset(vector_A, 0, sizeof(*vector_A) * vector_size); // reset vector_B
        }
        *col_ptr_writer = matrix_C->values.length;
        col_ptr_writer++;
        memset(vector_B, 0, sizeof(*vector_B) * vector_size); // reset vector_A
    }

    free(vector_A);
    free(vector_B);
}
