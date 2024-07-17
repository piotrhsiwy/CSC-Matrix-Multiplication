#include "mul_vector.h"

__attribute__((hot)) void matrix_mul_vector(const void*  a, const void*  b, void*  c){
 
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
    uint64_t values_index = 0; // use this to write to values and row_indices arrays

    // these are vectors for all the rows extracted from matrix A
    size_t vector_size = matrix_A->no_cols;
    size_t how_many_vectors = matrix_A->no_rows;
    float** all_vectors_A = malloc(sizeof(*all_vectors_A)* how_many_vectors);
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
        nested_free(how_many_vectors, all_vectors_A);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    memset(vector_B, 0, sizeof(*vector_B) * vector_size);

    // this is vector to store the expanded column of matrix C, before storing it
    float* const result = malloc(sizeof(*result) * vector_size);
    if(!result){
        fprintf(stderr, "Cannot allocate memory for result field in function %s\n", __FUNCTION__);
        nested_free(how_many_vectors, all_vectors_A);
        free(vector_B);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    memset(result, 0, sizeof(*result) * how_many_vectors);

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

        // ##################################
        // MULTIPLY ALL THE ROWS WITH THE COLUMN AND WRITE THE WHOLE
        // ##################################
        
        mul_vector_transposed_w_matrix(how_many_vectors, vector_size, vector_B, all_vectors_A, result);
        // after this call the result-matrix's column is in result[how_many_vectors]
        
        // TODO I think this loop might just nullify all the work done by vectorizing the multiplication, since we have to add one-by-one anyway
        for(size_t index_in_vector = 0; index_in_vector < how_many_vectors; index_in_vector++){
            float value = result[index_in_vector]; //TODO: fix overflow (maybe change loop condition to <?) - Execute debug_asan with -U xyz -B xyz to see issue (on 6th time called)
            if(value != 0.0f){
                pair new_pair = {.index = index_in_vector, .value = value};
                if(dynamic_array_add(pair, &matrix_C->dynamic_values_x_row_indices, new_pair)){
                    fprintf(stderr, "Cannot add new element to dynamic_values_x_row_indices in function %s\n", __FUNCTION__);
                    nested_free(how_many_vectors, all_vectors_A);
                    free(vector_B);
                    free(result);
                    matrix_C->mul_error = OUT_OF_MEMORY;
                    return;
                }
            }
        }
        memset(vector_B, 0, sizeof(*vector_B) * vector_size); // reset vector_B
        *col_ptr_writer = matrix_C->dynamic_values_x_row_indices.length;
        col_ptr_writer++;
    }

    nested_free(how_many_vectors, all_vectors_A);
    free(vector_B);
    free(result);
    matrix_C->mul_error = OK;
    return;
}
