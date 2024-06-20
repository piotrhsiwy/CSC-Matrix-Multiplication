#ifndef GRA24CAPSPROJEKT_T064_MATRIX_H
#define GRA24CAPSPROJEKT_T064_MATRIX_H

#include "util/format.h"
#include "util/dynamic_array.h"
#include "error_handling/errors.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>


typedef struct {
    uint64_t no_rows;
    uint64_t no_cols;
    size_t length_values;
    float* values;
    uint64_t* row_indices;
    // len (*colPtr) = noRows + 1
    uint64_t* col_ptr;
} matrix_data;


const char* init_empty_matrix(const uint64_t no_rows, const uint64_t no_cols, const size_t length_values, matrix_data* const data);


void destruct_matrix_data(matrix_data* const data);


IO_error read_matrix_from_fd(FILE *const stream, matrix_data *const data);


IO_error write_matrix_to_fd(FILE* stream, matrix_data data);

#endif // GRA24CAPSPROJEKT_T064_MATRIX_H
