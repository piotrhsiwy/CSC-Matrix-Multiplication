#include "matrix.h"

#include "util/format.h"
// #include "util/dynamic_array.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string.h>

__attribute__((cold)) void destruct_matrix_data(matrix_data* const data)
{
    free(data->values);
    data->values = NULL;
    free(data->row_indices);
    data->row_indices = NULL;
    free(data->col_ptr);
    data->col_ptr = NULL;
}

__attribute__((cold)) IO_error read_matrix_from_fd(FILE* const stream, matrix_data* const data)
{
    errno = 0;

    char* line = NULL;
    size_t line_length;

    if (getline(&line, &line_length, stream) == -1)
        return UNEXPECTED_END_OF_FILE;

    char* token;

    // region setting no_rows
    token = strtok(line, ",");
    if (!token) {
        free(line);
        return UNEXPECTED_END_OF_LINE;
    }
    // TODO: check whether to use strtoull
    data->no_rows = strtoul(token, NULL, 10);
    if (data->no_rows == 0 || (data->no_rows == ULONG_MAX && errno == ERANGE)) { // Zero is unexpected: either trying to enter empty matrix or invalid number format.
        free(line);
        return INVALID_VALUE;
    }
    // endregion

    // region setting no_cols
    token = strtok(NULL, ",");
    if (!token) {
        free(line);
        return UNEXPECTED_END_OF_LINE;
    }
    data->no_cols = strtoul(token, NULL, 10);
    if (data->no_cols == 0 || (data->no_cols == ULONG_MAX && errno == ERANGE)) { // Zero is unexpected: either trying to enter empty matrix or invalid number format.
        free(line);
        return INVALID_VALUE;
    }
    // endregion

    // TODO: fail, if still more characters left in line

    // region setting values
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        return UNEXPECTED_END_OF_FILE;
    }

    // count the number of , in the line
    // length_values will be that + 1
    size_t num_commas = 0;
    size_t index_in_line = 0;
    // TODO do we have to handle german commas, for example 4,523
    // TODO what if there is no commas and no value
    // TODO what if there is no commas and 1 value, need to separate those 2
    while (line[index_in_line] != '\0') {
        if (line[index_in_line] == ',') {
            num_commas++;
        }
        index_in_line++;
    }
    float* values = malloc(sizeof(*values) * (num_commas + 1));
    if (NULL == values) {
        free(line);
        return OUT_OF_MEMORY;
    }

    size_t values_index = 0;
    for (token = strtok(line, ","); token; token = strtok(NULL, ",")) {
        const float value = strtof(token, NULL);
        if (value == 0.f) { // We do not expect zero as a value -> either not CSC or invalid number format ; Comparing directly is fine, since '0.0F' will be returned exactly according to man(3)
            // TODO can we disambiguate wether it was a zero or an invalid number format?? maybe errno?? read some manpages
            free(line);
            free(values);
            return INVALID_VALUE;
        }
        // over/underflows
        if ((value == HUGE_VALF || value == HUGE_VALL) && errno == ERANGE) {
            free(line);
            free(values);
            return INVALID_VALUE;
        }
        values[values_index] = value;
        values_index++;
    }

    // TODO: sanity-check on length of values? --> How?
    // is it even possible with noCols noRows
    // yes, just multiply > ceiling with UINT64_MAX > compare
    // does it make sense? i don't know but better be safe than sorry
    data->values = values;
    data->length_values = num_commas + 1;
    // endregion

    // row_indices hat len = length_values

    uint64_t* ints = malloc(sizeof((*ints)) * (num_commas + 1));
    if (NULL == ints) {
        free(line);
        free(data->values);
        return OUT_OF_MEMORY;
    }

    // region setting row_indices
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        free(data->values);
        return UNEXPECTED_END_OF_FILE;
    }

    size_t ints_index = 0;
    for (token = strtok(line, ","); token; token = strtok(NULL, ",")) {
        const uint64_t index = strtoul(token, NULL, 10);
        // TODO: check for invalid format as well
        if ((index == 0 || index == ULONG_MAX) && errno == ERANGE) {
            free(line);
            free(data->values);
            free(ints);
            return INVALID_VALUE;
        }
        ints[ints_index] = index;
        ints_index++;
    }

    // TODO: sanity-check on length of row_indices? is it equal to length_values
    data->row_indices = ints;
    ints = NULL;
    // endregion

    // region setting col_ptr
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        free(data->values);
        free(data->row_indices);
        return UNEXPECTED_END_OF_FILE;
    }

    uint64_t* cols = malloc(sizeof(*cols) * (data->no_rows + 1));
    if (NULL == cols) {
        free(line);
        free(data->values);
        free(data->row_indices);
        free(cols);
        return OUT_OF_MEMORY;
    }

    size_t cols_index = 0;
    for (token = strtok(line, ","); token; token = strtok(NULL, ",")) {
        const uint64_t index = strtoul(token, NULL, 10);
        // TODO: check for invalid format as well
        // over/underflows
        if ((index == 0 || index == ULONG_MAX) && errno == ERANGE) {
            free(line);
            free(data->values);
            free(data->row_indices);
            free(cols);
            return INVALID_VALUE;
        }
        cols[cols_index] = index;
        cols_index++;
    }

    // TODO: sanity-check on length of col_ptr? is it equal to noCols + 1
    data->col_ptr = cols;
    cols = NULL;
    // endregion

    free(line);
    return OK;
}

__attribute__((cold)) IO_error write_matrix_to_fd(FILE* const stream, const matrix_data data)
{
    // write the noRows,NoCols
    fprintf(stream, "%lu,%lu\n", data.no_rows, data.no_cols);
    // methode, um ein Array zu schreiben lohnt sich nicht, denn es gibt keine Generics
    // TODO macros???
    // values
    for (uint64_t i = 0; i < data.length_values; ++i) {
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
    for (uint64_t i = 0; i < (data.no_cols + 1); ++i) {
        // TODO error handling
        if (0 != i) {
            int ret = fprintf(stream, ",");
        }

        int ret = fprintf(stream, FORMAT_UINT_64, data.col_ptr[i]);
    }
    fprintf(stream, "\n");
    //TODO: fsync?
    return OK;
}
