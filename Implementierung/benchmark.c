#include "benchmark.h"
#include "util/signal_io.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#define SEC_IN_NANOS 1000000000l


error execute_benchmark(const matrix_data* a, const matrix_data* b, matrix_data *const result, const uint32_t iterations, const matrix_multiplier algorithm, struct timespec *const duration) {
    algorithm(a, b, result); //We don't want to benchmark the memory allocator --> preallocate

    struct timespec start;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        fprintf(stderr, "failed to get time. errno is: %d\n", errno);
        errno = 0;
        return COULD_NOT_FETCH_TIME;
    }

    for(uint32_t i = 0; i < iterations; ++i) {
        algorithm(a, b, result);
        if(result->mul_error != OK) {
            return result->mul_error;
        }
    }

    if (clock_gettime(CLOCK_MONOTONIC, duration) == -1) {
        fprintf(stderr, "failed to get time. errno is: %d\n", errno);
        errno = 0;
        return COULD_NOT_FETCH_TIME;
    }

    duration->tv_sec -= start.tv_sec;
    duration->tv_nsec -= start.tv_nsec;

    if(duration->tv_nsec < 0l) {
        duration->tv_nsec += SEC_IN_NANOS;
        duration->tv_sec -= 1l;
    }

    return OK;
}

volatile bool continue_unattended_benchmark;
void interrupt_handler(const int sig) {
    if(sig != SIGINT) {
        return;
    }

    print_sig("\nCtrl-C erkannt.");
    while(true) {
        print_sig("\nTesten fortführen/Nach diesem Run beenden/Sofort beenden? [c/r/q] ");

        switch(getchar_sig()) {
            case 'c':
            case 'C':
                print_sig("Fahre fort...\n");
                break;

            case 'r':
            case 'R':
                print_sig("Warte auf letzten Run...\n");
                continue_unattended_benchmark = false;
                break;

            case 'q':
            case 'Q':
                print_sig("Bist Du Dir sicher?");
                while(true) {
                    print_sig("\nDas kann zu Datenverlust führen. [j/n] ");

                    switch (getchar_sig()) {
                        case 'j':
                        case 'J':
                            print_sig("Beende sofort...\n");
                            exit(EXIT_SUCCESS); //cleanup of fds or memory impossible

                        case 'n':
                        case 'N':
                            print_sig("Fahre fort...\n");
                            break;

                        default:
                            continue;
                    }
                    break;
                }
                break;

            default:
                continue;
        }
        break;
    }
}

//TODO: move to util
int cmp_u64(const void *const aptr, const void *const bptr) {
    uint64_t a = *(uint64_t *)aptr;
    uint64_t b = *(uint64_t *)bptr;
    return a > b ? 1 : (a < b ? -1 : 0);
}
//TODO: move to matrix.h/c?
error fill_matrix(matrix_data *const matrix, const uint8_t filledPerTen) {
    const size_t entries = (matrix->no_cols * matrix->no_rows * filledPerTen + 5) / 10; //+5 is for proper rounding

    dynamic_array_reserve(pair, entries, &matrix->dynamic_values_x_row_indices);
    matrix->dynamic_values_x_row_indices.length = 0;

    uint64_t cells_with_space = matrix->no_cols * matrix->no_rows;
    void* tmp = realloc(matrix->col_ptr, (matrix->no_cols + 1) * sizeof *matrix->col_ptr);
    if(!tmp) {
        return OUT_OF_MEMORY;
    }
    matrix->col_ptr = tmp;
    for(size_t i = 0; i < entries; ++i) {
        dynamic_array_at(pair, &matrix->dynamic_values_x_row_indices, i).value = (float)rand() + 0.00001; //random float bigger than 0

        matrix->col_ptr[i + 1] = matrix->no_rows;
    }

    //TODO: clustering as a factor --> might have an impact on SIMD
    for(size_t i = 0; i < entries; ++i) {
        size_t cell_offset = rand() % cells_with_space;
        cells_with_space--;

        for(size_t col = 0; col < entries; ++col) {
            if (cell_offset < matrix->col_ptr[col]) {
                --matrix->col_ptr[col];
                break;
            }
            cell_offset -= matrix->col_ptr[col];
        }
    }
    uint64_t col_index = 0;
    for(size_t i = 0; i < entries; ++i) {
        matrix->col_ptr[i + 1] = (col_index += matrix->col_ptr[i]);
    }

    uint64_t* row_indices = alloca(matrix->no_rows * sizeof *row_indices);
    if(!row_indices) {
        return OUT_OF_MEMORY;
    }
    for(uint64_t col = 0; col < matrix->no_cols; ++col) {
        const uint64_t filled_cells = matrix->col_ptr[col + 1] - matrix->col_ptr[col];
        uint64_t free_cells = filled_cells;

        for(uint64_t i = 0; i < filled_cells; ++i) {
            row_indices[i] = rand() % free_cells;
            --free_cells;

            for(uint64_t j = 0; j < i; ++j) {
                if(row_indices[i] <= row_indices[j]) {
                    ++row_indices[i];
                }
            }
        }

        qsort(row_indices, filled_cells, sizeof(*row_indices), cmp_u64);
        for(uint64_t i = 0; i < filled_cells; ++i) {
            dynamic_array_at(DYNAMIC_ARRAY(pair), &matrix->dynamic_values_x_row_indices, i + matrix->col_ptr[col]).index = row_indices[i];
        }
    }

    return OK;
}
error unattended_benchmark_for_dimensions(const uint64_t rows, const uint64_t cols, const uint32_t iterations_per_run, FILE *const output, matrix_data *const a, matrix_data *const b, matrix_data *const c, struct timespec *const duration_buffer) {
    a->no_cols = b->no_rows = cols;
    a->no_rows = b->no_cols = rows;

    //Reset arrays without freeing memory since we'll still use it
    a->dynamic_values_x_row_indices.length = 0;
    b->dynamic_values_x_row_indices.length = 0;

    for(uint8_t filledPerTen = 0; filledPerTen < 10 && continue_unattended_benchmark; ++filledPerTen) {
        //TODO: handle errors
        error err = fill_matrix(a, filledPerTen);
        if(err) {
            return err;
        }
        err = fill_matrix(b, filledPerTen);
        if(err) {
            return err;
        }

        bool prev_continue_unattended_benchmark = true;
        for(size_t algo_id = 0; algo_id < matrix_mul_algorithm_count; ++algo_id) {
            const matrix_multiplier algo = matrix_mul_algorithms[algo_id];

            //TODO: temp
            destruct_matrix_data(c);
            err = execute_benchmark(a, b, c, iterations_per_run, algo, duration_buffer);
            if(!continue_unattended_benchmark && prev_continue_unattended_benchmark) {
                prev_continue_unattended_benchmark = false;
                --algo_id; //repeat measurement
                continue;
            }

            if(err) {
                //TODO: log error1
            } else {
                fprintf(output, "%lu,%lu,%d,%lu,%f\n", rows, cols, (10 - filledPerTen) * 10, algo_id, (duration_buffer->tv_sec / (double)SEC_IN_NANOS + duration_buffer->tv_nsec) / iterations_per_run);
            }
        }
    }

    return OK;
}
error unattended_benchmark(const uint32_t iterations_per_run, const char *const export_csv) {
    signal(SIGINT, interrupt_handler);
    srand(123456789); //We want predictable numbers in our matrices.

    continue_unattended_benchmark = true;

    //TODO: return out of memory when initializing arrays and free other memory

    matrix_data a, b; //We don't need to initialize other fields than dynamic arrays as we initialize them on each run.
    dynamic_array_new(pair, 0, &a.dynamic_values_x_row_indices);
    dynamic_array_new(pair, 0, &b.dynamic_values_x_row_indices);

    matrix_data c = {.no_rows = 0, .no_cols = 0, .col_ptr = NULL };
    dynamic_array_new(pair, 0, &c.dynamic_values_x_row_indices);

    FILE *const output = fopen(export_csv, "w");
    fprintf(output, "#Rows,#Columns,Sparseness [%% empty],Algorithm,Duration [nsec]\n");

    struct timespec duration_buffer = {.tv_nsec = 0, .tv_sec = 0};
    //Exponentially increase #rows
    for(uint64_t row_count = 1; row_count < UINT64_MAX / 2 && continue_unattended_benchmark; row_count *= 2) {
        //Cover 10 equally spaced values in range #rows/2 to #rows*2 for #columns
        uint64_t increment = (row_count / 2) / 5;
        if(increment < 1) {
            increment = 1;
        }

        printf("\rBenchmarke mit Matrizen der Größe %ld (Letzter Durchlauf brauchte ca. %ld Sek.)", row_count, duration_buffer.tv_sec);
        fflush(stdout);
        for(uint64_t col_count = row_count / 2; col_count < (row_count * 2) && continue_unattended_benchmark; col_count += increment) {
            //TODO: different column counts for a and b
            if(col_count == 0) {
                continue;
            }

            //TODO: handle error
            unattended_benchmark_for_dimensions(row_count, col_count, iterations_per_run, output, &a, &b, &c, &duration_buffer);
        }
    }
    printf("\n");

    dynamic_array_destroy((generic_dynamic_array*) &a.dynamic_values_x_row_indices);
    dynamic_array_destroy((generic_dynamic_array*) &b.dynamic_values_x_row_indices);
    dynamic_array_destroy((generic_dynamic_array*) &c.dynamic_values_x_row_indices);

    fclose(output);
    signal(SIGINT, SIG_DFL); //reset signal handler
    return OK;
}
