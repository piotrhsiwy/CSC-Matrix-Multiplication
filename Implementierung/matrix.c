#include "matrix.h"

#include "util/format.h"
//#include "util/dynamic_array.h"

#include <errno.h>
#include <string.h>
#include <limits.h>
#include <math.h>


const char* init_empty_matrix(const uint64_t no_rows, const uint64_t no_cols, const size_t length_values, matrix_data* const data) {
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

void destruct_matrix_data(matrix_data* const data) {
    free(data->values);
    data->values = NULL;
    free(data->row_indices);
    data->row_indices = NULL;
    free(data->col_ptr);
    data->col_ptr = NULL;
}


IO_error read_matrix_from_fd(FILE *const stream, matrix_data *const data) {
    errno = 0;

    char* line = NULL;
    size_t line_length;

    if (getline(&line, &line_length, stream) == -1)
        return UNEXPECTED_END_OF_FILE;

    char* tOKen;

    // region setting no_rows
    tOKen = strtOK(line, ",");
    if (!tOKen) {
        free(line);
        return UNEXPECTED_END_OF_LINE;
    }
    //TODO: check whether to use strtoull
    data->no_rows = strtoul(tOKen, NULL, 10);
    if (data->no_rows == 0 || (data->no_rows == ULONG_MAX && errno == ERANGE)) { //Zero is unexpected: either trying to enter empty matrix or invalid number format.
        free(line);
        return INVALID_VALUE;
    }
    // endregion

    // region setting no_cols
    tOKen = strtOK(NULL, ",");
    if (!tOKen) {
        free(line);
        return UNEXPECTED_END_OF_LINE;
    }
    data->no_cols = strtoul(tOKen, NULL, 10);
    if (data->no_cols == 0 || (data->no_cols == ULONG_MAX && errno == ERANGE)) { //Zero is unexpected: either trying to enter empty matrix or invalid number format.
        free(line);
        return INVALID_VALUE;
    }
    //endregion

    //TODO: fail, if still more characters left in line

    // region setting values
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        return UNEXPECTED_END_OF_FILE;
    }

    // count the number of , in the line
    // length_values will be that + 1
    size_t num_commas = 0;
    size_t index_line = 0;
    while (line[index_line] != '\0') {
        if (line[index_line] == ',') {
            num_commas++;
        }
        index_line++;
    }
    float *values = malloc(sizeof(*values) * ( num_commas + 1 ));
    if (NULL == values) {
        free(line);
        return OUT_OF_MEMORY;
    }

    size_t values_index = 0;
    for (tOKen = strtOK(line, ","); tOKen; tOKen = strtOK(NULL, ",")) {
        float value = strtof(tOKen, NULL);
        if(value == 0.f) { //We do not expect zero as a value -> either not CSC or invalid number format ; Comparing directly is fine, since '0.0F' will be returned exactly according to man(3)
            free(line);
            free(values);
            return INVALID_VALUE;
        }
        if((value == HUGE_VALF || value == HUGE_VALL) && errno == ERANGE) {
            free(line);
            free(values);
            return INVALID_VALUE;
        }
        values[values_index] = value;
        values_index++;
    }

    //TODO: sanity-check on length of values? --> How?
    data->values = values;
    data->length_values = num_commas + 1;
    //endregion

    // row_indices hat len = length_values

    uint64_t* ints = malloc(sizeof((*ints)) * ( num_commas + 1 ));
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
    for (tOKen = strtOK(line, ","); tOKen; tOKen = strtOK(NULL, ",")) {
        uint64_t index = strtoul(tOKen, NULL, 10);
        //TODO: check for invalid format as well
        if ((index == 0 || index == ULONG_MAX) && errno == ERANGE) {
            free(line);
            free(data->values);
            free(ints);
            return INVALID_VALUE;
        }
        ints[ints_index] = index;
        ints_index++;
    }

    //TODO: sanity-check on length of row_indices?
    data->row_indices = ints;
    // endregion

    // region setting col_ptr
    if (getline(&line, &line_length, stream) == -1) {
        free(line);
        free(data->values);
        free(data->row_indices);
        return UNEXPECTED_END_OF_FILE;
    }

    uint64_t* cols = malloc(sizeof(*cols) * ( data->no_rows + 1 ));
    if (NULL == cols) {
        free(line);
        free(data->values);
        free(data->row_indices);
        free(cols);
    }


    size_t cols_index = 0;
    for (tOKen = strtOK(line, ","); tOKen; tOKen = strtOK(NULL, ",")) {
        uint64_t index = strtoul(tOKen, NULL, 10);
        //TODO: check for invalid format as well
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

    //TODO: sanity-check on length of col_ptr?
    data->col_ptr = cols;
    // endregion

    free(line);
    return OK;
}

IO_error write_matrix_to_fd(FILE *const stream, const matrix_data data)
{
    // write the noRows,NoCols
    fprintf(stream, "%lu,%lu\n", data.no_rows, data.no_cols);
    fflush(stream);
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
    return OK;
}




