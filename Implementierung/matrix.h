#ifndef GRA24CAPSPROJEKT_T064_MATRIX_H
#define GRA24CAPSPROJEKT_T064_MATRIX_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint64_t no_rows;
    uint64_t no_cols;
    size_t length_values;
    float* values;
    uint64_t* row_index;
    // len (*colPtr) = noRows + 1
    uint64_t* col_ptr;
} matrix_data;

/**
 * Constructs a struct of type matrix_data with the arrays allocated correctly.
 *
 * Returns an error as c-string or NULL on success.
 */
char* initMatrix(const uint64_t no_rows, const uint64_t no_cols, const size_t length_values, matrix_data* const data)
{
    data->no_rows = no_rows;
    data->no_cols = no_cols;
    data->length_values = length_values;

    if (!(data->values = malloc(length_values * sizeof(*data->values))))
        return "Out of memory";

    if (!(data->row_index = malloc(length_values * sizeof(*data->row_index)))) {
        free(data->values);
        return "Out of memory";
    }

    if (!(data->col_ptr = malloc((no_rows + 1) * sizeof(*data->col_ptr)))) {
        free(data->values);
        free(data->row_index);
        return "Out of memory";
    }

    return NULL;
}

/**
 * Frees the memory used by the arrays of the given matrix_data.
 */
void destruct_matrix_data(matrix_data* const data)
{
    free(data->values);
    data->values = NULL;
    free(data->row_index);
    data->rowIndex = NULL;
    free(data->colPtr);
    data->colPtr = NULL;
}

int read_matrix_from_fd(FILE *const stream, matrix_data *const data) {

}


/*
 * Writes the matrix to passed stream in the text representation given by the requirements
 */
int write_matrix_to_fd(FILE* stream, matrix_data data)
{

    // write the noRows,NoCols
    fprintf(stream, "%lu,%lu\n", data.no_rows, data.no_cols);
    // methode, um ein Array zu schreiben lohnt sich nicht, denn es gibt keine Generics
    // TODO macros???
    // values
    for (int i = 0; i < data.length_values; ++i) {
        // TODO error Handling
        if (0 == i) {
            int ret = fprintf(stream, "%f", data.values[i]);
        } else {
            int ret = fprintf(stream, ",%f", data.values[i]);
        }
    }
    fprintf(stream, "\n");
    // row_indexes
    for (int i = 0; i < data.length_values; ++i) {
        // TODO error handling
        if (0 == i) {
            int ret = fprintf(stream, "%lu", data.row_index[i]);
        } else {
            int ret = fprintf(stream, ",%lu", data.row_index[i]);
        }
    }
    fprintf(stream, "\n");
    // col_ptr
    for (int i = 0; i < (data.no_rows + 1); ++i) {
        // TODO error handling
        if (0 == i) {
            int ret = fprintf(stream, "%lu", data.col_ptr[i]);
        } else {
            int ret = fprintf(stream, ",%lu", data.col_ptr[i]);
        }
    }
    fprintf(stream, "\n");
}

#endif // GRA24CAPSPROJEKT_T064_MATRIX_H
