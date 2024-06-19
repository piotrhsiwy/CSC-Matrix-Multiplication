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

/**
 * Constructs a struct of type matrix_data with the arrays allocated correctly.
 *
 * Returns an error as c-string or NULL on success.
 */
char* initMatrix(const uint64_t no_rows, const uint64_t no_cols, const size_t length_values, matrix_data* const data) {
    data->no_rows = no_rows;
    data->no_cols = no_cols;
    data->length_values = length_values;

    if (!(data->values = malloc(length_values * sizeof(*data->values))))
        return "Out of memory";

    if (!(data->row_indices = malloc(length_values * sizeof(*data->row_indices)))) {
        free(data->values);
        return "Out of memory";
    }

    if (!(data->col_ptr = malloc((no_rows + 1) * sizeof(*data->col_ptr)))) {
        free(data->values);
        free(data->row_indices);
        return "Out of memory";
    }

    return NULL;
}

/**
 * Frees the memory used by the arrays of the given matrix_data.
 */
void destruct_matrix_data(matrix_data* const data) {
    free(data->values);
    data->values = NULL;
    free(data->row_indices);
    data->row_indices = NULL;
    free(data->col_ptr);
    data->col_ptr = NULL;
}

enum IO_error read_matrix_from_fd(FILE *const stream, matrix_data *const data) {
    errno = 0;

    char* line = NULL;
    size_t line_length;

    if (getline(&line, &line_length, stream) == -1)
        return unexpected_end_of_file;

    char* token;

    // region setting no_rows
    token = strtok(line, ",");
    if (!token) {
        free(line);
        return unexpected_end_of_line;
    }
    //TODO: check whether to use strtoull
    data->no_rows = strtoul(token, NULL, 10);
    if (data->no_rows == 0 || (data->no_rows == ULONG_MAX && errno == ERANGE)) { //Zero is unexpected: either trying to enter empty matrix or invalid number format.
        free(line);
        return invalid_value;
    }
    // endregion

    // region setting no_cols
    token = strtok(NULL, ",");
    if (!token) {
        free(line);
        return unexpected_end_of_line;
    }
    data->no_cols = strtoul(token, NULL, 10);
    if (data->no_cols == 0 || (data->no_cols == ULONG_MAX && errno == ERANGE)) { //Zero is unexpected: either trying to enter empty matrix or invalid number format.
        free(line);
        return invalid_value;
    }
    //endregion

    // region setting values
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        return unexpected_end_of_file;
    }

    DYNAMIC_ARRAY(float) values;
    if (dynamic_array_new(float, 4, &values)) {
        free(line);
        return out_of_memory;
    }
    for (token = strtok(line, ","); token; token = strtok(NULL, ",")) {
        float value = strtof(token, NULL);
        if(value == 0.f) { //We do not expect zero as a value -> either not CSC or invalid number format ; Comparing directly is fine, since '0.0F' will be returned exactly according to man(3)
            free(line);
            dynamic_array_destroy((generic_dynamic_array*) &values);
            return invalid_value;
        }
        if((value == HUGE_VALF || value == HUGE_VALL) && errno == ERANGE) {
            free(line);
            dynamic_array_destroy((generic_dynamic_array*) &values);
            return invalid_value;
        }
        dynamic_array_add(float, &values, value);
    }

    //TODO: sanity-check on length of values? --> How?
    data->values = values.data;
    data->length_values = values.length;
    dynamic_array_unbind((generic_dynamic_array *) &values);
    //endregion

    DYNAMIC_ARRAY(uint64_t) ints;

    // region setting row_indices
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        free(data->values);
        return unexpected_end_of_file;
    }

    if (dynamic_array_new(uint64_t, 4, &ints)) {
        free(line);
        free(data->values);
        return out_of_memory;
    }
    for (token = strtok(line, ","); token; token = strtok(NULL, ",")) {
        uint64_t index = strtoul(token, NULL, 10);
        //TODO: check why false positives are triggered...
        /*if ((index == 0 || index == ULONG_MAX) && errno == ERANGE) {
            free(line);
            free(data->values);
            dynamic_array_destroy((generic_dynamic_array*) &ints);
            return invalid_value;
        }*/
        dynamic_array_add(uint64_t, &ints, index);
    }

    //TODO: sanity-check on length of row_indices?
    data->row_indices = ints.data;
    dynamic_array_unbind((generic_dynamic_array*) &ints);
    // endregion

    // region setting col_ptr
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        free(data->values);
        free(data->row_indices);
        return unexpected_end_of_file;
    }

    if (dynamic_array_new(uint64_t, 4, &ints)) {
        free(line);
        free(data->values);
        return out_of_memory;
    }
    for (token = strtok(line, ","); token; token = strtok(NULL, ",")) {
        uint64_t index = strtoul(token, NULL, 10);
        //TODO: check why false positives are triggered...
        /*if ((index == 0 || index == ULONG_MAX) && errno == ERANGE) {
            free(line);
            free(data->values);
            free(data->row_indices);
            dynamic_array_destroy((generic_dynamic_array*) &ints);
            return invalid_value;
        }*/
        dynamic_array_add(uint64_t, &ints, index);
    }

    //TODO: sanity-check on length of col_ptr?
    data->col_ptr = ints.data;
    dynamic_array_unbind((generic_dynamic_array*) &ints);
    // endregion

    free(line);
    return ok;
}


/*
 * Writes the matrix to passed stream in the text representation given by the requirements
 */
enum IO_error write_matrix_to_fd(FILE* stream, matrix_data data)
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
    // row_indices
    for (int i = 0; i < data.length_values; ++i) {
        // TODO error handling
        if (0 != i) {
            int ret = fprintf(stream, ",");
        }

        int ret = fprintf(stream, FORMAT_UINT_64, data.row_indices[i]);
    }
    fprintf(stream, "\n");
    // col_ptr
    for (int i = 0; i < (data.no_rows + 1); ++i) {
        // TODO error handling
        if (0 != i) {
            int ret = fprintf(stream, ",");
        }

        int ret = fprintf(stream, FORMAT_UINT_64, data.col_ptr[i]);
    }
    fprintf(stream, "\n");
}

#endif // GRA24CAPSPROJEKT_T064_MATRIX_H
