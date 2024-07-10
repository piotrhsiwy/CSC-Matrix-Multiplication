#include "mul_scalar.h"

//TODO add void matrix_mult_csc(void*, void*, void*) interface as per Angabe

__attribute__((hot)) error matrix_mul_scalar(const matrix_data* const a, const matrix_data* const b, matrix_data* const result) {
    //TODO: Handle already allocated memory. I think we can get away with just reusing it, since allocating is not
    //      really a part of this algorithm but rather a necessity.

    // voids are kept in the signature so that the functions fits the project's specifications
    const matrix_data* const matrix_A = a;
    const matrix_data* const matrix_B = b;
    matrix_data* const matrix_C = result;
 
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
    // EXTRACT THE ROW FROM MATRIX A AND THE COLUMN FROM MATRIX B
    // ##################################
    // indices as on this graphic: https://de.wikipedia.org/wiki/Matrizenmultiplikation#/media/Datei:Matrix_multiplication_qtl2.svg
    // I iterate over columns first (in the outer loop), because that makes it easier to write to result matrix. This is the result of using CSC.
    uint64_t* col_ptr_writer = matrix_C->col_ptr; // use this to write to col_ptr array
    *col_ptr_writer = 0; // initialize col_ptr
    col_ptr_writer++;
    uint64_t values_index = 0; // use this to write to values and row_indices arrays

    // this is dynamic vector for the row extracted from matrix A
    ___dynamic_array_pair_macro_defined dynamic_vector_A;
    if(___dynamic_array_pair_macro_defined_new(4, &dynamic_vector_A)){
        fprintf(stderr, "Cannot dynamically allocate memory for vector_A field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
        return OUT_OF_MEMORY;
    }
    // this is dynamic vector for the col extracted from matrix B
    ___dynamic_array_pair_macro_defined dynamic_vector_B;
    if(___dynamic_array_pair_macro_defined_new(4, &dynamic_vector_B)){
        fprintf(stderr, "Cannot dynamically allocate memory for vector_B field in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
        dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
        dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_A);
        return OUT_OF_MEMORY;
    }

    /*
     * iterator names *k* and *i* as on the graphic linked above. Short indices became more
     * intuitive for us since we've been working with the graphic at all times anyway
     * */
    for(uint64_t k = 0; k < matrix_C->no_cols; k++){ // 

        // extract column from matrix B. See description of the function in matrix_util.h
        {error error = extract_compressed_col_csc(matrix_B, k, &dynamic_vector_B);
        if(error != OK){
            free(matrix_C->col_ptr);
            dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
            dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
            dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_A);
            dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
            return error;
        }}
        if(dynamic_vector_B.length == 0){ // the col is empty
            continue;
        }

        // optimization idea: map all the pairs from matrix A (for all row-numbers) with one run, because for now I have to go through the whole value-length array for every row!
        // note that this comes with limitation by matrix_C->no_cols
        // this optimization  idea has been implemented in _preextract algorithm
        for(uint64_t i = 0; i < matrix_C->no_rows; i++){
            
            // extract row from matrix A. See description of the function in matrix_util.h
            {error error = extract_compressed_row_csc(matrix_A, i, &dynamic_vector_A);
            if( error != OK){
                free(matrix_C->col_ptr);
                dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_A);
                dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
                return error;
            }}
            if(dynamic_vector_A.length == 0){
                continue;
            }

            // ##################################
            // MULTIPLY THE ROW WITH THE COLUMN AND WRITE THE RESULTS
            // ##################################
            // TODO here possibly a SIMD can be used (if no_rows/cols small enough) as follows
            //  reorder -> mult -> horizontal add ( operating on 32bfloats )
            float value = mul_vectors_compressed(&dynamic_vector_A, &dynamic_vector_B);
            if(value != 0.0f){
                if(___dynamic_array_float_macro_defined_add(&matrix_C->dynamic_values, value)){
                    fprintf(stderr, "Cannot add new element to dynamic_values in function %s\n", __FUNCTION__);
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_A);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
                    return OUT_OF_MEMORY;
                }
                if(___dynamic_array_uint64_t_macro_defined_add(&matrix_C->dynamic_row_indices, i)){
                    fprintf(stderr, "Cannot add new element to dynamic_row_indices in function %s\n", __FUNCTION__);
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&matrix_C->dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_A);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
                    return OUT_OF_MEMORY;
                }
            }
            // else we don't write anything
            dynamic_vector_A.length = 0; // "empty" the vector holding the row
        }
        *col_ptr_writer = matrix_C->dynamic_values.length;
        col_ptr_writer++;
        dynamic_vector_B.length = 0; // "empty" the vector holding the column
    }

    return OK;
}
