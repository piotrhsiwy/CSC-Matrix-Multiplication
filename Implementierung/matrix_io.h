#ifndef GRA24CAPSPROJEKT_T064_MATRIX_H
#define GRA24CAPSPROJEKT_T064_MATRIX_H

// we need this because getline is not part of C-standard, see man 3 getline vs man 3 fgets
#define _POSIX_C_SOURCE 200809L

#include "error_handling/errors.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>


typedef struct {
    uint64_t no_rows;
    uint64_t no_cols;
    size_t length_values;
    float* values;
    uint64_t* row_indices;
    // len (*colPtr) = noCols + 1
    uint64_t* col_ptr;
} matrix_data;

/**
 * Frees the memory used by the arrays of the given matrix_data.
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

/*
 * Writes the matrix to passed stream in the text representation given by the requirements
 */
error write_matrix_to_fd(FILE* stream, matrix_data data);

#endif // GRA24CAPSPROJEKT_T064_MATRIX_H
