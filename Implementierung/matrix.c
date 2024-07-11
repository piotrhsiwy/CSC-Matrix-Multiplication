#include "matrix.h"

#include "util/format.h"
#include "util/dynamic_array.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string.h>

__attribute__((cold)) void destruct_matrix_data(matrix_data* const data)
{
    // freeing NULL doesnt do anything, no need to check for it
    free(data->col_ptr);
    data->col_ptr = NULL;
    dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
}

__attribute__((cold)) error read_matrix_from_fd(FILE* const stream, matrix_data* const data)
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


    // TODO what if there is no commas and no value
    // TODO what if there is no commas and 1 value, need to separate those 2
    if(dynamic_array_new(pair, 4, &data->dynamic_values_x_row_indices)){
        fprintf(stderr, "Cannot dynamically allocate memory for dynamic_values_x_row_indices field in function %s\n", __FUNCTION__);
        free(line);
        return OUT_OF_MEMORY;
    }

    for (token = strtok(line, ","); token; token = strtok(NULL, ",")) {
        const float value = strtof(token, NULL);
        if (value == 0.f) { // We do not expect zero as a value -> either not CSC or invalid number format ; Comparing directly is fine, since '0.0F' will be returned exactly according to man(3)
            // TODO can we disambiguate wether it was a zero or an invalid number format?? maybe errno?? read some manpages
            free(line);
            return INVALID_VALUE;
        }
        // over/underflows
        if ((value == HUGE_VALF || value == HUGE_VALL) && errno == ERANGE) {
            free(line);
            return INVALID_VALUE;
        }
        pair new_pair = {.index = 0, .value = value}; // index is not yet known at this point
        if(dynamic_array_add(pair, &data->dynamic_values_x_row_indices, new_pair)){
            fprintf(stderr, "Cannot add element to dynamically sized array dynamic_values_x_row_indices in function %s\n", __FUNCTION__);
            dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
            free(line);
            return OUT_OF_MEMORY;
        }
    }

    // TODO: sanity-check on length of values? --> How?
    // is it even possible with noCols noRows
    // yes, just multiply > ceiling with UINT64_MAX > compare
    // does it make sense? i don't know but better be safe than sorry
    // data->values = values;
    // endregion

    // row_indices hat len = length_values

    // region setting row_indices

    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
        return UNEXPECTED_END_OF_FILE;
    }

    size_t loop_counter = 0;
    for (token = strtok(line, ","); token; token = strtok(NULL, ",")) {
        const uint64_t index = strtoul(token, NULL, 10);
        // TODO: check for invalid format as well
        if ((index == 0 || index == ULONG_MAX) && errno == ERANGE) {
            free(line);
            dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
            return INVALID_VALUE;
        }
        pair update_my_index = dynamic_array_at(pair, &data->dynamic_values_x_row_indices, loop_counter);
        update_my_index.index = index;
        dynamic_array_set(pair, &data->dynamic_values_x_row_indices, loop_counter, update_my_index);
    }

    // TODO: sanity-check on length of row_indices? is it equal to length_values
    // data->row_indices = ints;
    // ints = NULL;
    // endregion

    // region setting col_ptr
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
        return UNEXPECTED_END_OF_FILE;
    }

    uint64_t* cols = malloc(sizeof(*cols) * (data->no_rows + 1));
    if (NULL == cols) {
        free(line);
        dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
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
            dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
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

__attribute__((cold)) error write_matrix_to_fd(FILE* const stream, const matrix_data data)
{
    // write the noRows,NoCols
    fprintf(stream, "%lu,%lu\n", data.no_rows, data.no_cols);
    // methode, um ein Array zu schreiben lohnt sich nicht, denn es gibt keine Generics
    // TODO macros???
    // values
    for (uint64_t i = 0; i < data.dynamic_values_x_row_indices.length; ++i) {
        // TODO error Handling
        if (i == 0) {
            int ret = fprintf(stream, "%f", dynamic_array_at(pair, &data.dynamic_values_x_row_indices, (size_t)i).value);
        } else {
            int ret = fprintf(stream, ",%f", dynamic_array_at(pair, &data.dynamic_values_x_row_indices, (size_t)i).value);
        }
    }
    fprintf(stream, "\n");
    // row_indices
    for (int i = 0; i < data.dynamic_values_x_row_indices.length; ++i) {
        // TODO error handling
        if (0 != i) {
            int ret = fprintf(stream, ",");
        }

        int ret = fprintf(stream, FORMAT_UINT_64, dynamic_array_at(pair, &data.dynamic_values_x_row_indices, i).index);
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
