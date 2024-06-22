#include "matrix_mult.h"

/* 
 * Multiples two matrices A and B in Compressed Sparse Column Format and saves the result in result
 * @param
 *  a is a pointer to matrix_data containing metadata of the matrix A
 *  b is a pointer to matrix_data containing metadata of the matrix B
 *  results is a pointer to matrix_data containing metadata of the resulting matrix after the fuction is executed
 */
void matrix_mult_csc(const void* const a, const void* const b, void* const result){
    // voids are kept in the signature so that the functions fits the project's specifications
    const matrix_data* const matrix_A = a;
    const matrix_data* const matrix_B = b;
    matrix_data* const matrix_C = result;

    // CATCH ERORRS
    // size mismatch
    if(matrix_A->no_cols != matrix_B->no_rows){
        fprintf(stderr, "Matrix size mismatch\n");
        return;
    }

    // INIT THE UKNOWN-SIZE ARRAYS OF C
    // TODO maybe move whole init_empty matrix method in here (only used here?)
    // TODO alloc either statically array of proper size or use dynamic allocation
    matrix_C->length_values = matrix_A->length_values * matrix_B->length_values;
    if(!( matrix_C->values = malloc(sizeof(*matrix_C->values) * matrix_C->length_values) )){
        fprintf(stderr, "Out of memory\n");
        return;
    }
    if(!( matrix_C->values = malloc(sizeof(*matrix_C->values) * ( matrix_A->length_values * matrix_B->length_values )) )){
        fprintf(stderr, "Out of memory\n");
        return;
    }
    
    // MULTIPLY
    // indices as on this graphic: https://de.wikipedia.org/wiki/Matrizenmultiplikation#/media/Datei:Matrix_multiplication_qtl2.svg
    // I multiply columns first
    uint64_t* col_ptr_writer = matrix_C->col_ptr; // use this to write to col_ptr array
    *col_ptr_writer++ = 0; // initialize col_ptr
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
            struct pair {uint64_t index; float value;};
            struct pair to_be_multiplied_A[matrix_C->length_values]; // TODO is this known before completing the multiplication?
            uint64_t no_to_be_multiplied_A = 0;
            // TODO optimization idea: map all the pairs from matrix A (for all row-numbers) with one run
            for(uint64_t j = 0; matrix_A->length_values; i++){
                if(matrix_A->row_indices[j] == i){
                    for(uint64_t l = 0; l < matrix_A->no_rows+1; l++){
                        if(matrix_A->col_ptr[l] <= j && matrix_A->col_ptr[l+1] > j){
                            to_be_multiplied_A[no_to_be_multiplied_A].index = l;
                            break;
                        }
                    }
                    to_be_multiplied_A[no_to_be_multiplied_A].value = matrix_A->values[j];
                    no_to_be_multiplied_A++;
                }
            }
            if(!no_to_be_multiplied_A){
                continue;
            }
            for(uint64_t j = matrix_B->col_ptr[k]; j < matrix_B->col_ptr[k+1]; j++){

            }
            struct pair to_be_multiplied_B[matrix_C->length_values]; // TODO is this known before completing the multiplication?
            uint64_t no_to_be_multiplied_B = 0;
            // TODO this 'for' can be moved out of the row-loop
            for(uint64_t j = matrix_B->col_ptr[k]; j < matrix_B->col_ptr[k+1]; j++){
                to_be_multiplied_B[no_to_be_multiplied_B].index = matrix_B->row_indices[j];
                to_be_multiplied_B[no_to_be_multiplied_B].value = matrix_B->values[j];
                no_to_be_multiplied_B++;
            }
            if(!no_to_be_multiplied_B){
                continue;
            }   
            // multiply the values
            for(uint64_t i = 0, j = 0; i < no_to_be_multiplied_A && j < no_to_be_multiplied_B;){
                // I can use the fact that the incides are increasing in both arrays
                if(to_be_multiplied_A[i].index > to_be_multiplied_B[j].index){
                    j++;
                    continue;
                } else if(to_be_multiplied_A[i].index < to_be_multiplied_B[j].index){
                    i++;
                    continue;
                } else {
                    acc += to_be_multiplied_A[i].value * to_be_multiplied_B[j].value;
                }
            }
            matrix_C->values[values_index] = acc;
            matrix_C->row_indices[values_index] = i;
        }
        *col_ptr_writer++ = values_index;
    }
    return;
}
