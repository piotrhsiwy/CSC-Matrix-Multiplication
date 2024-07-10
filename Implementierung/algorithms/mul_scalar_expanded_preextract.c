#include "mul_scalar_expanded_preextract.h"

void nested_free(size_t size, float** vectors){
    while(--size){
        free(vectors[size]);
    }
    free(vectors[0]);
    free(vectors);
}

__attribute__((hot)) error matrix_mul_scalar_expanded_preextract(const matrix_data* const matrix_A, const matrix_data* const matrix_B, matrix_data* const matrix_C) {
 
    // ##################################
    // ALLOCATE THE FIELDS OF THE RESULT MATRIX
    // ##################################
    // all malloc'd pointers can be passed to free (alloc'd or null)
    matrix_C->no_rows = matrix_A->no_rows;
    matrix_C->no_cols = matrix_B->no_cols;

    matrix_C->col_ptr = malloc(sizeof(*matrix_C->col_ptr) * ( matrix_C->no_cols + 1 ));
    if(!(matrix_C->col_ptr)){
        fprintf(stderr, "Out of memory for col_ptr field in function %s\n", __FUNCTION__);
        return OUT_OF_MEMORY;
    }

    if(___dynamic_array_float_macro_defined_new(4, &matrix_C->dynamic_values)){
        fprintf(stderr, "Cannot dynamically allocate memory for dynamic_values field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        return OUT_OF_MEMORY;
    }

    if(___dynamic_array_uint64_t_macro_defined_new(4, &matrix_C->dynamic_row_indices)){
        fprintf(stderr, "Cannot dynamically allocate memory for dynamic_row_indices field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        return OUT_OF_MEMORY;
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
        return DIMENSION_MISMATCH;
    }
        // one of the matrices has length_values == 0. That means all values in the result matrix are also 0;
    if(matrix_A->dynamic_values.length == 0 || matrix_B->dynamic_values.length == 0){
        memset(matrix_C->col_ptr, 0, matrix_C->no_cols+1);
        return OK;
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

    // these are vectors for all the rows extracted from matrix A
    size_t vector_size = matrix_C-> no_rows;
    float** all_vectors_A = malloc(sizeof(*all_vectors_A)* matrix_A->no_rows);
    if(!all_vectors_A){
        fprintf(stderr, "Cannot allocate memory for all_vectors_A field in function %s.\nThis could be due to key limitation of this implementation. See mul_scalar_expanded.h", __FUNCTION__);
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
        return OUT_OF_MEMORY;
    }
    for(size_t i = 0; i < matrix_A->no_rows; i++){
        float* vector = malloc(sizeof(*vector) * vector_size);
        if(!vector){
            fprintf(stderr, "Cannot allocate memory for all_vectors_A[%lu] vector field in function %s.\nThis could be due to key limitation of this implementation. See mul_scalar_expanded.h", i, __FUNCTION__);
            free(matrix_C->col_ptr);
            dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
            dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
            nested_free(i, all_vectors_A);
            return OUT_OF_MEMORY;
        }
        memset(vector, 0, sizeof(*vector) * vector_size);
        all_vectors_A[i] = vector;
    }

    // this is vector for the col extracted from matrix B
    float* vector_B = malloc(sizeof(*vector_B) * vector_size);
    if(!vector_B){
        fprintf(stderr, "Cannot allocate memory for vector_B field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
        nested_free(matrix_A->no_rows, all_vectors_A);
        return OUT_OF_MEMORY;
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
            // TODO here possibly a SIMD can be used (if no_rows/cols small enough) as follows
            //  reorder -> mult -> horizontal add ( operating on 32bfloats )
            if(!all_vectors_A[i]){ // the row is empty
                continue;
            }
            float value = mul_vectors_expanded(vector_size, all_vectors_A[i], vector_B);
            if(value != 0.0f){
                if(___dynamic_array_float_macro_defined_add(&matrix_C->dynamic_values, value)){
                    fprintf(stderr, "Cannot add new element to dynamic_values in function %s\n", __FUNCTION__);
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                    nested_free(matrix_A->no_rows, all_vectors_A);
                    free(vector_B);
                    return OUT_OF_MEMORY;
                }
                if(___dynamic_array_uint64_t_macro_defined_add(&matrix_C->dynamic_row_indices, i)){
                    fprintf(stderr, "Cannot add new element to dynamic_row_indices in function %s\n", __FUNCTION__);
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                    nested_free(matrix_A->no_rows, all_vectors_A);
                    free(vector_B);
                    return OUT_OF_MEMORY;
                }
            }
            // else we don't write anything
        }
        *col_ptr_writer = matrix_C->dynamic_values.length;
        col_ptr_writer++;
        memset(vector_B, 0, sizeof(*vector_B) * vector_size); // reset vector_B
    }

    nested_free(matrix_A->no_rows, all_vectors_A);
    free(vector_B);
    return OK;
}