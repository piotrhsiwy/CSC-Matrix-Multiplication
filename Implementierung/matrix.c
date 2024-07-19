#include "matrix.h"

#include "util/format.h"
#include "util/dynamic_array.h"
#include "util/compare.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <string.h>

__attribute__((cold)) error allocate_matrix_data(const uint64_t no_rows, const uint64_t no_cols, matrix_data* const matrix){

    matrix->no_rows = no_rows;
    matrix->no_cols = no_cols;
    matrix->mul_error = UNDEFINED;

    void* tmp = realloc(matrix->col_ptr, sizeof(*matrix->col_ptr) * (matrix->no_cols + 1));
    if(!(tmp)){
        fprintf(stderr, "Out of memory for col_ptr field in function %s\n", __FUNCTION__);
        return OUT_OF_MEMORY;
    }
    matrix->col_ptr = tmp;

    if(dynamic_array_reserve(float, 4, &matrix->values)){
        fprintf(stderr, "Cannot dynamically allocate memory for values field in function %s\n", __FUNCTION__);
        free(matrix->col_ptr);
        return OUT_OF_MEMORY;
    }

    if(dynamic_array_reserve(uint64_t, 4, &matrix->row_indices)){
        fprintf(stderr, "Cannot dynamically allocate memory for row_indices field in function %s\n", __FUNCTION__);
        free(matrix->col_ptr);
        return OUT_OF_MEMORY;
    }

    return OK;
}

__attribute__((cold)) void destruct_matrix_data(matrix_data* const matrix)
{
    // freeing NULL doesnt do anything, no need to check for it
    free(matrix->col_ptr);
    matrix->col_ptr = NULL;
    dynamic_array_destroy((generic_dynamic_array*)&matrix->values);
    dynamic_array_destroy((generic_dynamic_array*)&matrix->row_indices);
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

    // See: CSC_format_definition.md for format constraints. This function strictly follows that definition
    #define REPORT_BAD_VALUE free(line); fprintf(stderr, "Der Wert des Felds %s in Zeile %d unter Index %lu ist inkorrekt. Lese CSC_format_definition.md\n", field, line_index, index); return INVALID_VALUE
    #define REPORT_TOO_LONG free(line); fprintf(stderr, "Die Zeile %d ist zu lang. Lese CSC_format_definition.md\n", line_index); return INVALID_LINE_LENGTH
    #define REPORT_TOO_SHORT free(line); fprintf(stderr, "Die Zeile %d ist zu kurz. Lese CSC_format_definition.md\n", line_index); return UNEXPECTED_END_OF_LINE

    typedef uint64_t (*u64reader_t)(const char *restrict , char **restrict , int );
    u64reader_t u64reader = 0;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "Simplify" //if condition is clear at compile time --> clang tidy complains
    if (UINT64_WIDTH / 8 == sizeof(unsigned long)) {
        u64reader = (u64reader_t)strtoul;
    } else {
        free(line);
        fprintf(stderr, "Keine Funktion um die Daten vom Typ uint64_t zu lesen in function %s\n", __FUNCTION__);
        return NO_U64READER_FUNC;
        // uint64_t has different width than both ul and ull?
    }
#pragma clang diagnostic pop


    /* LINE 1 */
    int line_index = 1;

    /* no_rows */
    size_t index = 0;
    char* field = "no_rows";
    token = strtok(line, ",");
    if (!token){
        REPORT_TOO_SHORT;
    }
    data->no_rows = u64reader(token, NULL, 10);
    if (token[0] == '-'){
        REPORT_BAD_VALUE;
    }
    if (data->no_rows == 0 || (data->no_rows == ULONG_MAX && errno == ERANGE || errno != 0)){
        REPORT_BAD_VALUE;
    }
    errno = 0;

    /* no_cols */
    index = 1;
    field = "no_cols";
    token = strtok(NULL, ",");
    if (!token) {
        REPORT_TOO_SHORT;
    }
    data->no_cols = u64reader(token, NULL, 10);
    if (data->no_cols == 0 || errno == ERANGE || errno != 0){
        REPORT_BAD_VALUE;
    }
    errno = 0;

    token = strtok(NULL, ",");
    if(token) {
        REPORT_TOO_LONG;
    }

    /* LINE 2 */
    line_index = 2;
    field = "values";
    index = 0;

    // allocate for values field
    if(dynamic_array_new(float, 4, &data->values)){
        fprintf(stderr, "Cannot dynamically allocate memory for dynamic_values_x_row_indices field in function %s\n", __FUNCTION__);
        return OUT_OF_MEMORY;
    }

    if (getline(&line, &line_length, stream) == -1) {
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        REPORT_TOO_SHORT;
    }

    for (token = strtok(line, ","); token; token = strtok(NULL, ","), index++) {
        const float value = strtof(token, NULL);

        if(*token == '\n') {
           // the value array is empty -> all the values in the matrix are zeros
           errno = 0;
           break;
        }

        if((value == HUGE_VALF || value == -HUGE_VALF ) && errno == ERANGE) {
            fprintf(stderr, "Warnung: Überlauf -> Wert '%s' wird gerundet.\n", token);
            errno = 0;
        } else if( value <= __FLT_MIN__ && value >= -__FLT_MIN__ && errno == ERANGE) {
            fprintf(stderr, "Warnung: Unterlauf -> Wert '%s' wird gerundet.\n", token);
            errno = 0;
        } else if(errno != 0){
            // other error
            dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
            REPORT_BAD_VALUE;
        } else if (value == 0.f) {
            // zero is not allowed 
            // TODO can we disambiguate wether it was a zero or an invalid number format?? maybe errno?? read some manpages
            dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
            REPORT_BAD_VALUE;
        }
        errno = 0;

        if(dynamic_array_add(float, &data->values, value)){
            fprintf(stderr, "Cannot add element to dynamically sized array values in function %s\n", __FUNCTION__);
            dynamic_array_destroy((generic_dynamic_array*)&data->values);
            free(line);
            return OUT_OF_MEMORY;
        }
    }

    // calculate maximum number_of values. Set it to UINT64_MAX if no_rows*no_cols would overflow
    const uint64_t MAX_VALUES_LENGTH = UINT64_MAX / data->no_cols < data->no_rows ?
        UINT64_MAX
        : data->no_cols * data->no_rows;
    // check length constraint
    if(data->values.length > MAX_VALUES_LENGTH ){
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        REPORT_TOO_LONG;
    }

    /* LINE 3 */
    line_index = 3;
    field = "row_indices";
    index = 0;

    // allocate for row_indices field
    if(dynamic_array_new(uint64_t, 4, &data->row_indices)){
        fprintf(stderr, "Cannot dynamically allocate memory for row_indices field in function %s\n", __FUNCTION__);
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        return OUT_OF_MEMORY;
    }

    if (getline(&line, &line_length, stream) == -1) {
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
        REPORT_TOO_SHORT;
    }

    for (token = strtok(line, ","); token; token = strtok(NULL, ","), index++) {
        if(*token == '\n'){
           // the row_indices array is empty -> all the values in the matrix are zeros
           errno = 0;
           break;
        }
        const uint64_t row_index = u64reader(token, NULL, 10);
        if (token[0] == '-'){
            dynamic_array_destroy((generic_dynamic_array*)&data->values);
            dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
            REPORT_BAD_VALUE;
        } else if(errno == ERANGE || row_index > data->no_rows-1 || errno != 0) {
            dynamic_array_destroy((generic_dynamic_array*)&data->values);
            dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
            REPORT_BAD_VALUE;
        }
        errno = 0;
        // check for length constraint (too long) before "at"-ing to dynamic array, else it could result in writing to unallocated block
        if(index >= MAX_VALUES_LENGTH || index >= data->row_indices.length){
            dynamic_array_destroy((generic_dynamic_array*)&data->values);
            dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
            REPORT_TOO_LONG;
        } else
            dynamic_array_add(uint64_t, &data->row_indices, row_index);
    }

    // check for length constraint (too short)
    if(index != data->row_indices.length){
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
        REPORT_TOO_SHORT;
    }

    /* LINE 4 */
    line_index = 4;
    field = "col_ptrs";
    index = 0;

    // allocate for no_cols field
    uint64_t* temp = malloc(sizeof(*temp) * (data->no_cols + 1));
    if (NULL == temp) {
        fprintf(stderr, "Cannot dynamically allocate memory for cols field in function %s\n", __FUNCTION__);
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
        return OUT_OF_MEMORY;
    }
    data->col_ptr = temp;

    if (getline(&line, &line_length, stream) == -1) {
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
        free(data->col_ptr);
        REPORT_TOO_SHORT;
    }

    // check for the "first value == 0" constraint
    token = strtok(line, ",");
    uint64_t col_ptr = u64reader(token, NULL, 10);
    if(errno != 0 || col_ptr != 0){
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
        free(data->col_ptr);
        REPORT_BAD_VALUE;
    }
    errno = 0;
    data->col_ptr[index++] = col_ptr;

    token = strtok(NULL, ",");
    for (; token; token = strtok(NULL, ",")) {
        col_ptr = u64reader(token, NULL, 10);
        if (( col_ptr == ULONG_MAX && errno == ERANGE ) || col_ptr > MAX_VALUES_LENGTH || errno != 0) {
            dynamic_array_destroy((generic_dynamic_array*)&data->values);
            dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
            free(data->col_ptr);
            REPORT_BAD_VALUE;
        }
        errno = 0;
        // check for the "(non-strictly) increasing" constraint
        if(!( col_ptr >= data->col_ptr[index-1] )){
            dynamic_array_destroy((generic_dynamic_array*)&data->values);
            dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
            free(data->col_ptr);
            REPORT_BAD_VALUE;
        }
        // check for the length constraint (too long), else it could result in a write to an unallocated block
        if(index >= data->no_cols+1){
            dynamic_array_destroy((generic_dynamic_array*)&data->values);
            dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
            free(data->col_ptr);
            REPORT_TOO_LONG;
        }
        data->col_ptr[index++] = col_ptr;
    }
    
    //check for the "last value == length of line 2" constraint
    index--;
    if(data->col_ptr[index] != data->values.length){
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
        free(data->col_ptr);
        REPORT_BAD_VALUE;
    }
    
    //check for the length constraint (too short)
    if(index != data->no_cols){
        dynamic_array_destroy((generic_dynamic_array*)&data->values);
        dynamic_array_destroy((generic_dynamic_array*)&data->row_indices);
        free(data->col_ptr);
        REPORT_TOO_SHORT;
    }

    //check for the sanity check. See CSC_format_definition.md. field/line_index are set for informative error reporting.
    line_index = 3;
    field = "row_indices(strictly increasing)";
    for(size_t i = 0; i < data->no_cols; i++){
        size_t begin = data->col_ptr[i];
        size_t end = ( data->col_ptr[i+1] == 0 ) ? 0 : data->col_ptr[i+1]-1; // begin == end == 0, then straight "-1" results in silent wrap-up and we get 0xfffffffffff in end
        for(index = begin; index < end; index++){ 
           float a = dynamic_array_at(pair, &data->dynamic_values_x_row_indices, index).index;
           float b = dynamic_array_at(pair, &data->dynamic_values_x_row_indices, index+1).index;
            if(!( b > a )){
                dynamic_array_destroy((generic_dynamic_array*)&data->dynamic_values_x_row_indices);
                free(data->col_ptr);
                index = index ? index+2 : index+1;
                // to account for i) 0-indexing ii) the fact that I'm comparing values under indices "index" and "index+1", but since prev test passed, the value under "index+1" is actually the incorrect one here
                // if index is 0, ii) does not apply, since there are no previous values that were checked, hence the ternary operator
                REPORT_BAD_VALUE;
            }
        }
    }

    free(line);
    return OK;
}

error fill_matrix(matrix_data *const matrix, const long double filledCellsRelative) {
    const size_t entries = (size_t)(matrix->no_cols * matrix->no_rows * 10 * filledCellsRelative + 5) / 10; // *10, +5, /10 is for rounding

    dynamic_array_reserve(float, entries, &matrix->values);
    matrix->values.length = 0;
    dynamic_array_reserve(uint64_t, entries, &matrix->row_indices);
    matrix->row_indices.length = 0;

    for(size_t i = 0; i < entries; ++i) {
        dynamic_array_add(float, &matrix->values, (float)rand() + 0.00001); //random float bigger than 0
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

    for(size_t i = 0; i < entries; ++i) {
        size_t cell_offset = rand() % cells_with_space;
        cells_with_space--;

        for(size_t col = 0; col < matrix->no_cols; ++col) {
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
            //TODO: directly insert into correct position (binary search insertion)
            qsort(row_indices, i, sizeof(*row_indices), cmp_u64);

            for(uint64_t j = 0; j < i; ++j) {
                if(row_indices[i] >= row_indices[j]) {
                    ++row_indices[i];
                }
            }
        }

        //The values are stored in a random order -> sort them before setting them as indizes for the values
        qsort(row_indices, filled_cells, sizeof(*row_indices), cmp_u64);
        for(uint64_t i = 0; i < filled_cells; ++i) {
            dynamic_array_at(uint64_t, &matrix->row_indices, i + matrix->col_ptr[col]) = row_indices[i];
        }
    }

    return OK;
}

__attribute__((cold)) error write_matrix_to_fd(FILE* const stream, const matrix_data data)
{
    // write the noRows,NoCols
    fprintf(stream, "%lu,%lu\n", data.no_rows, data.no_cols);

    // TODO macros???
    // values
    for (uint64_t i = 0; i < data.values.length; ++i) {
        // TODO error Handling
        if (i == 0) {
            fprintf(stream, "%f", dynamic_array_at(float, &data.values, (size_t)i));
        } else {
            fprintf(stream, ",%f", dynamic_array_at(float, &data.values, (size_t)i));
        }
    }
    fprintf(stream, "\n");
    // row_indices
    for (int i = 0; i < data.dynamic_values_x_row_indices.length; ++i) {
        // TODO error handling
        if (0 != i) {
            fprintf(stream, ",");
        }

        fprintf(stream, FORMAT_UINT_64, dynamic_array_at(uint64_t, &data.row_indices, i));
    }
    fprintf(stream, "\n");
    // col_ptr
    for (uint64_t i = 0; i < (data.no_cols + 1); ++i) {
        // TODO error handling
        if (0 != i) {
            fprintf(stream, ",");
        }

        fprintf(stream, FORMAT_UINT_64, data.col_ptr[i]);
    }
    return OK;
}
