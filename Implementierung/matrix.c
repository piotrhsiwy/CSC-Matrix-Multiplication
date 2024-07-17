#include "matrix.h"

#include "util/format.h"
#include "util/dynamic_array.h"
#include "util/compare.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string.h>

__attribute__((cold)) error allocate_matrix_data(const uint64_t no_rows, const uint64_t no_cols, matrix_data* const matrix){

    matrix->col_ptr = malloc(sizeof(*matrix->col_ptr) * (matrix->no_cols + 1)); //TODO: segfaultv in malloc (yes, it is really the call to malloc)
    if(!(matrix->col_ptr)){
        fprintf(stderr, "Out of memory for col_ptr field in function %s\n", __FUNCTION__);
        return OUT_OF_MEMORY;
    }

    if(dynamic_array_new(pair, 4, &matrix->dynamic_values_x_row_indices)){
        fprintf(stderr, "Cannot dynamically allocate memory for dynamic_values_x_row_indices field in function %s\n", __FUNCTION__);
        free(matrix->col_ptr);
        return OUT_OF_MEMORY;
    }

    matrix->no_rows = no_rows;
    matrix->no_cols = no_cols;
    matrix->mul_error = UNDEFINED;

    return OK;
}

__attribute__((cold)) void destruct_matrix_data(matrix_data* const matrix)
{
    // freeing NULL doesnt do anything, no need to check for it
    free(matrix->col_ptr);
    matrix->col_ptr = NULL;
    dynamic_array_destroy((generic_dynamic_array*)&matrix->dynamic_values_x_row_indices);
    matrix->no_rows = 0;
    matrix->no_cols = 0;
    matrix->mul_error = UNDEFINED;
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
        dynamic_array_at(pair, &data->dynamic_values_x_row_indices, loop_counter) = update_my_index;
        loop_counter++;
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

error fill_matrix(matrix_data *const matrix, const uint8_t filledPerTen) {
    const size_t entries = (matrix->no_cols * matrix->no_rows * filledPerTen + 5) / 10; //+5 is for proper rounding

    dynamic_array_reserve(pair, entries, &matrix->dynamic_values_x_row_indices);
    matrix->dynamic_values_x_row_indices.length = 0;

    for(size_t i = 0; i < entries; ++i) {
        const pair p = {.value=(float)rand() + 0.00001};
        dynamic_array_add(pair, &matrix->dynamic_values_x_row_indices, p); //random float bigger than 0
    }

    uint64_t cells_with_space = matrix->no_cols * matrix->no_rows;
    void* tmp = realloc(matrix->col_ptr, (matrix->no_cols + 1) * sizeof *matrix->col_ptr);
    if(!tmp) {
        return OUT_OF_MEMORY;
    }
    matrix->col_ptr = tmp;
    //Save number of free cells in each next's column-ptr
    matrix->col_ptr[0] = 0;
    for(size_t i = 0; i < matrix->no_cols; ++i) {
        matrix->col_ptr[i + 1] = matrix->no_rows;
    }

    //TODO: clustering as a factor --> might have an impact on SIMD
    for(size_t i = 0; i < entries; ++i) {
        size_t cell_offset = rand() % cells_with_space;
        cells_with_space--;

        for(size_t col = 0; col < entries; ++col) {
            if (cell_offset < matrix->col_ptr[col + 1]) {
                --matrix->col_ptr[col + 1]; //offset is inside this column -> decrease number of free cells inside
                break;
            }
            cell_offset -= matrix->col_ptr[col + 1]; //offset is behind this column -> set to offset to next column
        }
    }

    uint64_t* row_indices = alloca(matrix->no_rows * sizeof *row_indices);
    if(!row_indices) {
        return OUT_OF_MEMORY;
    }
    for(uint64_t col = 0; col < matrix->no_cols; ++col) {
        const uint64_t filled_cells = matrix->no_rows - matrix->col_ptr[col + 1];
        matrix->col_ptr[col + 1] = matrix->col_ptr[col] + filled_cells; //convert number of free cells into offset

        uint64_t free_cells = matrix->no_rows;

        // In our imagination, we create an array containing every free cell inside a column, pick one at random and remove it
        //
        // In reality we only store the cells chosen. All offsets of cells with a higher offset are therefore move one to the left.
        // To account for this, for each cell that is before the chosen one we increment the offset by one.
        //
        // (Both are equivalent.)
        for(uint64_t i = 0; i < filled_cells; ++i) {
            row_indices[i] = rand() % free_cells;
            --free_cells;

            uint64_t offset = 0;
            for(uint64_t j = 0; j < i; ++j) {
                if(row_indices[i] >= row_indices[j]) {
                    ++offset;
                }
            }
            row_indices[i] += offset;
        }

        //The values are stored in a random order -> sort them before setting them as indizes for the values
        qsort(row_indices, filled_cells, sizeof(*row_indices), cmp_u64);
        for(uint64_t i = 0; i < filled_cells; ++i) {
            dynamic_array_at(DYNAMIC_ARRAY(pair), &matrix->dynamic_values_x_row_indices, i + matrix->col_ptr[col]).index = row_indices[i];
        }
    }

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
