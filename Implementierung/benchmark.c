#include "benchmark.h"
#include <errno.h>

#define SEC_IN_NANOS 1000000000l


error execute_benchmark(const matrix_data* a, const matrix_data* b, matrix_data* result, uint32_t iterations, matrix_multiplier algorithm, struct timespec* duration) {
    struct timespec start;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        fprintf(stderr, "failed to get time. errno is: %d\n", errno);
        errno = 0;
        return COULD_NOT_FETCH_TIME;
    }

    for(uint32_t i = 0; i < iterations; ++i) {
        error err = algorithm(a, b, result);
        if(err != OK) {
            return err;
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
error unattended_benchmark_for_dimensions(const uint64_t rows, const uint64_t cols, const uint32_t iterations_per_run, const char *const export_directory, matrix_data *const a, matrix_data *const b, matrix_data *const c, struct timespec *const duration_buffer) {
    a->no_cols = b->no_rows = cols;
    a->no_rows = b->no_cols = rows;

    //Reset arrays without freeing memory since we'll still use it
    a->dynamic_row_indices.length = a->dynamic_values.length = 0;
    b->dynamic_row_indices.length = b->dynamic_values.length = 0;

    //TODO: create .csv file
    for(uint16_t filledPerThousand = 0; filledPerThousand < 1000 && continue_unattended_benchmark; ++filledPerThousand) {
        //TODO: generate matrix a and b
        //TODO: experiment with clustering as a factor(?) --> might have an impact on SIMD

        for(size_t algo_id = 0; algo_id < matrix_mul_algorithm_count; ++algo_id) {
            matrix_multiplier algo = matrix_mul_algorithms[algo_id];

            error err = execute_benchmark(a, b, c, iterations_per_run, algo, duration_buffer);

            if(err) {
                //TODO: log error
            } else {
                //TODO: add line to file
            }
        }
    }
    //TODO: close file

    return OK;
}
error unattended_benchmark(const uint32_t iterations_per_run, const char *const export_directory) {
    //TODO: register signal handler for Ctrl+C and set continue_unattended_benchmark to false
    continue_unattended_benchmark = true;

    //TODO: return out of memory when initializing arrays and free other memory

    matrix_data a, b; //We don't need to initialize other fields than dynamic arrays as we initialize them on each run.
    dynamic_array_new(float,    0, &a.dynamic_values);
    dynamic_array_new(uint64_t, 0, &a.dynamic_row_indices);
    dynamic_array_new(float,    0, &b.dynamic_values);
    dynamic_array_new(uint64_t, 0, &b.dynamic_row_indices);

    matrix_data c = {.no_rows = 0, .no_cols = 0, .col_ptr = NULL };
    dynamic_array_new(float,    0, &c.dynamic_values);
    dynamic_array_new(uint64_t, 0, &c.dynamic_row_indices);


    struct timespec duration_buffer = {.tv_nsec = 0, .tv_sec = 0};
    //Exponentially increase #rows
    for(uint64_t row_count = 1; row_count < UINT64_MAX / 2 && continue_unattended_benchmark; row_count *= 2) {
        //Cover 20 equally spaced values in range #rows/2 to #rows*2 for #columns
        uint64_t increment = (row_count / 2) / 10;
        if(increment < 1) {
            increment = 1;
        }

        printf("\rBenchmarke mit Matrizen der Größe %ld (Letzter Durchlauf brauchte ca. %ld Sek.)", row_count, duration_buffer.tv_sec);
        for(uint64_t col_count = row_count / 2; col_count < (row_count * 2) && continue_unattended_benchmark; col_count += increment) {
            //TODO: different column counts for a and b
            if(col_count == 0) {
                continue;
            }

            //TODO: handle error
            unattended_benchmark_for_dimensions(row_count, col_count, iterations_per_run, export_directory, &a, &b, &c, &duration_buffer);
        }
    }

    dynamic_array_destroy((generic_dynamic_array*) &a.dynamic_values);
    dynamic_array_destroy((generic_dynamic_array*) &a.dynamic_row_indices);
    dynamic_array_destroy((generic_dynamic_array*) &b.dynamic_values);
    dynamic_array_destroy((generic_dynamic_array*) &b.dynamic_row_indices);
    dynamic_array_destroy((generic_dynamic_array*) &c.dynamic_values);
    dynamic_array_destroy((generic_dynamic_array*) &c.dynamic_row_indices);
    return OK;
}
