#include <string.h> // memset
#include "mul_scalar.h"
#include "../matrix_util.h" // extracting rows, cols, multiplying vectors
#include "../util/dynamic_array.h"

//TODO add void matrix_mult_csc(void*, void*, void*) interface as per Angabe

__attribute__((hot)) error matrix_mul_scalar(const matrix_data* const a, const matrix_data* const b, matrix_data* const result) {
    //TODO: Handle already allocated memory. I think we can get away with just reusing it, since allocating is not
    //      really a part of this algorithm but rather a necessity.

    // voids are kept in the signature so that the functions fits the project's specifications
    const matrix_data* const matrix_A = a;
    const matrix_data* const matrix_B = b;
    matrix_data* const matrix_C = result;
 
    // fill the fields of the result matrix, all pointers can be passed to free (alloc'd or NULL)
    matrix_C->no_rows = matrix_A->no_rows;
    matrix_C->no_cols = matrix_B->no_cols;
    matrix_C->col_ptr = malloc(sizeof(*matrix_C->col_ptr) * ( matrix_C->no_cols + 1 ));
    if(!(matrix_C->col_ptr)){
        fprintf(stderr, "Out of memory in function %s\n", __FUNCTION__);
        return OUT_OF_MEMORY;
    }
    matrix_C->length_values = 0;
    matrix_C->values = NULL; 
    matrix_C->row_indices = NULL;

    // CATCH ERORRS
        // size mismatch
    if(matrix_A->no_cols != matrix_B->no_rows){
        fprintf(stderr, "Matrix size mismatch\n");
        return DIMENSION_MISMATCH;
    }
        // one of the matrices has length_values==0 == all zero values == all zeros in the resulting matrix
    if(matrix_A->length_values == 0 || matrix_B->length_values == 0){
        memset(matrix_C->col_ptr, 0, matrix_C->no_cols);
        return OK;
    }

    // INIT THE UKNOWN-SIZE ARRAYS OF MATRIX C: values and row_indices
    /*
    dynamic solution
    */
    ___dynamic_array_uint64_t_macro_defined dynamic_row_indices;
    {
        const char* error_string = ___dynamic_array_uint64_t_macro_defined_new(4, &dynamic_row_indices);
        if(error_string){
            fprintf(stderr, "Cannot dynamically allocate memory for row_indices field in function %s\n", __FUNCTION__);
            free(matrix_C->col_ptr);
            return OUT_OF_MEMORY;
        }
    }
    matrix_C->row_indices = dynamic_row_indices.data;
    ___dynamic_array_float_macro_defined dynamic_values;
    {
        const char* error_string = ___dynamic_array_float_macro_defined_new(4, &dynamic_values);
        if(error_string){
            fprintf(stderr, "Cannot dynamically allocate memory for values field in function %s\n", __FUNCTION__);
            free(matrix_C->col_ptr);
            dynamic_array_destroy((generic_dynamic_array*)&dynamic_row_indices);
            return OUT_OF_MEMORY;
        }
    }
    matrix_C->values = dynamic_values.data;
    /*
    // makeshift static solution 
    #define MAX_LENGTH_VALUES 1024
    matrix_C->length_values = MAX_LENGTH_VALUES;
    if( matrix_A->length_values * matrix_B->length_values > MAX_LENGTH_VALUES ){
        fprintf(stderr, "Out of memory\n");
        return OUT_OF_MEMORY;
    }
    matrix_C->values = malloc(sizeof(*matrix_C->values) * matrix_C->length_values);
    if(!(matrix_C->values)){                                                           
        fprintf(stderr, "Out of memory in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        return OUT_OF_MEMORY;
    }
    matrix_C->row_indices = malloc(sizeof(*matrix_C->row_indices) * matrix_C->length_values);
    if(!(matrix_C->values)){                                                           
        fprintf(stderr, "Out of memory in function %s\n", __FUNCTION__);
        free(matrix_C->col_ptr);
        free(matrix_C->values);
        return OUT_OF_MEMORY;
    }
    */

    // MULTIPLY
    // indices as on this graphic: https://de.wikipedia.org/wiki/Matrizenmultiplikation#/media/Datei:Matrix_multiplication_qtl2.svg
    // I iterate over columns first, because that makes it easier to write to result matrix
    uint64_t* col_ptr_writer = matrix_C->col_ptr; // use this to write to col_ptr array
    *col_ptr_writer = 0; // initialize col_ptr
    col_ptr_writer++;
    uint64_t values_index = 0; // use this to write to values and row_indices arrays
    /*
     * iterator names as on the graphic linked above, short indices are more
     * intuitive for us since we've been working with the graphic at all times anyway
     * */
    for(uint64_t k = 0; k < matrix_C->no_cols; k++){ // 
        /*
        * to-be-multiplied values from matrix_B lie as follows:
        *  - their values in values array under indices col_ptr[k], ..., col_ptr[k+1]
        *  - their row-number in row_indices array under the same indices
        * the function extract_compressed_col_csc does exactly that
        * */
        ___dynamic_array_pair_macro_defined dynamic_vector_B;
        {
            const char* error_string = ___dynamic_array_pair_macro_defined_new(4, &dynamic_vector_B);
            if(error_string){
                fprintf(stderr, "Cannot dynamically allocate memory for vector_B field in function %s\n", __FUNCTION__);
                free(matrix_C->col_ptr);
                dynamic_array_destroy((generic_dynamic_array*)&dynamic_row_indices);
                dynamic_array_destroy((generic_dynamic_array*)&dynamic_values);
                return OUT_OF_MEMORY;
            }
        }
        { 
            error error = extract_compressed_col_csc(matrix_A, k, &dynamic_vector_B);
            if( error != OK){
                free(matrix_C->col_ptr);
                dynamic_array_destroy((generic_dynamic_array*)&dynamic_row_indices);
                dynamic_array_destroy((generic_dynamic_array*)&dynamic_values);
                dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
                return error;
            }
        }
        if(dynamic_vector_B.length == 0){
            continue;
        }
        // TODO this 'for' can be moved out of the row-loop
        for(uint64_t i = 0; i < matrix_C->no_rows; i++){
            /*
             * to-be-multiplied values from matrix_A lie as follows:
             *  - their values in values array under indices x for wh, const uint64_t length_extended)ich row_ptr[x] = i
             *  - their column-number is COL that col_ptr[COL] <= x < col_ptr[COL+1]
             * the function extract_compressed_row_csc does exactly that 
             * */
            ___dynamic_array_pair_macro_defined dynamic_vector_A;
            {
                const char* error_string = ___dynamic_array_pair_macro_defined_new(4, &dynamic_vector_A);
                if(error_string){
                    fprintf(stderr, "Cannot dynamically allocate memory for vector_A field in function %s\n", __FUNCTION__);
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
                    return OUT_OF_MEMORY;
                }
            }
            // TODO optimization idea: map all the pairs from matrix A (for all row-numbers) with one run, because for now I have to go through the whole value-length array for every row!
            { 
                error error = extract_compressed_row_csc(matrix_A, i, &dynamic_vector_A);
                if( error != OK){
                    free(matrix_C->col_ptr);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_A);
                    return error;
                }
            }
            if(dynamic_vector_A.length == 0){
                continue;
            }
            // MULTIPLY THE ROW WITH THE COLUMN
            /* 
             * caveat: the result is == 0 iff
             *  there is no pair of (column-number, row-number) such that column-number == row-number
             *  (or either the column or the row is empty)
             * */
            // TODO here possibly a SIMD can be used (if no_rows/cols small enough) as follows
            //  reorder -> mult -> horizontal add ( operating on 32bfloats )
            float value = mul_vectors_compressed(&dynamic_vector_A, &dynamic_vector_B);
            if(value != 0.0f){
                if(___dynamic_array_float_macro_defined_add(&dynamic_values, value)){
                    fprintf(stderr, "Cannot add new element to dynamic_values in function %s\n", __FUNCTION__);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_A);
                    return OUT_OF_MEMORY;
                }
                if(___dynamic_array_uint64_t_macro_defined_add(&dynamic_row_indices, i)){
                    fprintf(stderr, "Cannot add new element to dynamic_values in function %s\n", __FUNCTION__);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_row_indices);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_values);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_B);
                    dynamic_array_destroy((generic_dynamic_array*)&dynamic_vector_A);
                    return OUT_OF_MEMORY;
                }
            }
            // else we don't write anything
        }
        *col_ptr_writer = dynamic_values.length;
        col_ptr_writer++;
    }
    matrix_C->length_values = dynamic_values.length;

    return OK;
}
