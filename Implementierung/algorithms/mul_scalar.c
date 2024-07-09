#include <string.h> // memset
#include "mul_scalar.h"

__attribute__((hot)) error matrix_mul_scalar(const void* const a, const void* const b, void* const result) {
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

    // INIT THE UKNOWN-SIZE ARRAYS OF MATRIX C
    // TODO alloc either statically array of proper size (impossible) or use dynamic allocation
    // a better aproximation maybe (maybe length_values * length_values)
    // i think the fastest solution would be to reallocate dynamically but on stack?
    // current solution: alloc(max{a*b, MAX})
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

    // MULTIPLY
    // indices as on this graphic: https://de.wikipedia.org/wiki/Matrizenmultiplikation#/media/Datei:Matrix_multiplication_qtl2.svg
    // I iterate over columns first
    // TODO why
    uint64_t* col_ptr_writer = matrix_C->col_ptr; // use this to write to col_ptr array
    *col_ptr_writer = 0; // initialize col_ptr
    col_ptr_writer++;
    uint64_t values_index = 0; // use this to write to values and row_indices arrays
    for(uint64_t k = 0; k < matrix_C->no_cols; k++){
        for(uint64_t i = 0; i < matrix_C->no_rows; i++){
            float acc = 0.0f;
            /*
             * to-be-multiplied values from matrix_A lie as follows:
             *  - their values in values array under indices x for which row_ptr[x] = i
             *  - their column-number is COL that col_ptr[COL] <= x < col_ptr[COL+1]
             * to-be-multiplied values from matrix_B lie as follows:
             *  - their values in values array under indices col_ptr[k], ..., col_ptr[k+1]
             *  - their row-number in row_indices array under the same indices
             * the result is =/=0 iff
             *  there is no pair of (column-number, row-number) such that column-number == row-number
             *  (or either the column or the row is empty)
             */
            pair to_be_multiplied_A[matrix_A->no_rows];
            uint64_t number_to_be_multiplied_A = 0;
            // TODO optimization idea: map all the pairs from matrix A (for all row-numbers) with one run
            for(uint64_t j = 0; j < matrix_A->length_values; j++){
                // TODO move the descs here for better understanding
                if(matrix_A->row_indices[j] == i){
                    for(uint64_t l = 0; l < matrix_A->no_rows+1; l++){
                        if(matrix_A->col_ptr[l] <= j && matrix_A->col_ptr[l+1] > j){
                            to_be_multiplied_A[number_to_be_multiplied_A].index = l;
                            break;
                        }
                    }
                    to_be_multiplied_A[number_to_be_multiplied_A].value = matrix_A->values[j];
                    number_to_be_multiplied_A++;
                }
            }
            if(!number_to_be_multiplied_A){
                continue;
            }
            pair to_be_multiplied_B[matrix_A->no_rows];
            uint64_t number_to_be_multiplied_B = 0;
            // TODO this 'for' can be moved out of the row-loop
            for(uint64_t j = matrix_B->col_ptr[k]; j < matrix_B->col_ptr[k+1]; j++){
                to_be_multiplied_B[number_to_be_multiplied_B].index = matrix_B->row_indices[j];
                to_be_multiplied_B[number_to_be_multiplied_B].value = matrix_B->values[j];
                number_to_be_multiplied_B++;
            }
            if(!number_to_be_multiplied_B){
                continue;
            }   
            // multiply the values
            // TODO here possibly a SIMD can be used (if no_rows/cols small enough) as follows
            //  reorder -> mult -> horizontal add ( operating on 32bfloats )
            // TODO are we sure we want to shadow a global here. Would be solved by different names in the outer loop anyway
            // explain
            for(uint64_t i = 0, j = 0; i < number_to_be_multiplied_A && j < number_to_be_multiplied_B;){
                // I can use the fact that the incides are increasing in both arrays
                if(to_be_multiplied_A[i].index > to_be_multiplied_B[j].index){
                    j++;
                    continue;
                } else if(to_be_multiplied_A[i].index < to_be_multiplied_B[j].index){
                    i++;
                    continue;
                } else {
                    acc += to_be_multiplied_A[i].value * to_be_multiplied_B[j].value;
                    i++;
                    j++;
                }
            }
            if(acc != 0.0f){
                matrix_C->values[values_index] = acc;
                matrix_C->row_indices[values_index] = i;
                values_index++;
            }
        }
        *col_ptr_writer = values_index;
        col_ptr_writer++;
    }
    matrix_C->length_values = values_index;
    //TODO: can't we leak memory here?
    void* temp = realloc(matrix_C->values, sizeof(*matrix_C->values) * matrix_C->length_values);
    // if realloc suceeded, save the new pointer in the struct
    if(temp){
        matrix_C->values = temp;
    }

    //TODO: can't we leak memory here?
    temp = realloc(matrix_C->row_indices, sizeof(*matrix_C->row_indices) * matrix_C->length_values);
    // if realloc suceeded, save the new pointer in the struct
    if(temp){
        matrix_C->row_indices = temp;
    }


    return OK;
}
