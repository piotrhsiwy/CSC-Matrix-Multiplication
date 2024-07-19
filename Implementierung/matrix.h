#ifndef GRA24CAPSPROJEKT_T064_MATRIX_H
#define GRA24CAPSPROJEKT_T064_MATRIX_H

// we need this because getline is not part of C-standard, see man 3 getline vs man 3 fgets
#define _POSIX_C_SOURCE 200809L

#include "error_handling/errors.h"
#include "util/dynamic_array.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define EMPTY_MATRIX_INIT {.no_rows = 0, .no_cols = 0, .col_ptr = NULL, .values = {.data = NULL, .capacity = 0, .length = 0}, .row_indices = {.data = NULL, .capacity = 0, .length = 0}}


typedef struct {
    uint64_t no_rows;
    uint64_t no_cols;
    // size_t length_values; <-- this field is now in dynamic_values_x_row_indices->length
    DYNAMIC_ARRAY(float) values;
    DYNAMIC_ARRAY(uint64_t) row_indices;
    // len (*colPtr) = noCols + 1
    uint64_t* col_ptr;

    /**
     * An error that occurred while trying to multiply two matrices with this matrix as the destination.
     * (We are sadly not allowed to change the method signature, so we can't just return errors.)
     */
    error mul_error;
} matrix_data;

/**
 * Allocate storage for all the fields of the matrix and fill the no_rows and no_cols fields appropriately.
 * The mul_error field will be filled with UNDEFINED.
 * 
 * @return OUT_OF_MEMORY if one of the allocations failed due to lack of memory OK otherwise
 */
error allocate_matrix_data(const uint64_t no_rows, const uint64_t no_cols, matrix_data* const matrix);

/**
 * Free all fields of the matrix, including
 *  col_ptr - static array
 *  dynamic_values_x_row_indices - dynamic array
 * Zero the fields
 *  no_cols
 *  no_rows
 * Set the mul_erorr field to undefined
 */
void destruct_matrix_data(matrix_data* data);

/**
 * Reads a CSC-matrix from the given file.
 * if this function executed successfully
 *  uint64_t no_cols =/= 0
 *  uint64_t no_rows =/= 0
 *  values[float] does not contain 0.0f
 *  length_values == lengthof(values) == lengthof(row_indices)
 *  lengthof(col_ptr) == no_cols + 1
 * @param stream The file to read from
 * @param data The position to store the matrix data. Make sure to free any memory used by the previous instance!
 * @return The error that occurred during reading or ok if reading was successful.
 */
error read_matrix_from_fd(FILE* stream, matrix_data* data);

/**
 * Fills a matrix with random values without changing the dimensions
 * @param matrix The matrix to fill
 * @param filledCellsRelative The relative amount of filled cells
 * @return The error if one occurred
 */
error fill_matrix(matrix_data* matrix, long double filledCellsRelative);

/*
 * Writes the matrix to passed stream in the text representation given by the requirements
 */
error write_matrix_to_fd(FILE* stream, matrix_data data);

#endif // GRA24CAPSPROJEKT_T064_MATRIX_H
