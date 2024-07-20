#include "benchmark.h"
#include "util/signal_io.h"
#include "util/attributes.h"
#include "util/rng.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>


#define SEC_IN_NANOS 1000000000l


always_inline void reset_result_and_execute_algorithm(const matrix_multiplier algorithm, const matrix_data* a, const matrix_data* b, matrix_data *const restrict result) {
    result->row_indices.length = 0;
    result->values.length = 0;
    algorithm(a, b, result);
}
__attribute__((hot)) error execute_benchmark(const matrix_data* a, const matrix_data* b, matrix_data *const result, const uint32_t iterations, const matrix_multiplier algorithm, struct timespec *const duration) {
    // We don't want to benchmark the memory allocator --> preallocate.
    // This should also load the function (instr.) and structures (data) into cache.
    reset_result_and_execute_algorithm(algorithm, a, b, result);

    struct timespec start;
    if (clock_gettime(CLOCK_MONOTONIC, &start) == -1) {
        fprintf(stderr, "failed to get time. errno is: %d\n", errno);
        errno = 0;
        return COULD_NOT_FETCH_TIME;
    }

    for(uint32_t i = 0; i < iterations; ++i) {
        reset_result_and_execute_algorithm(algorithm, a, b, result);
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

    //Due to subtraction we might end up with negative values for nanoseconds. -> "Borrow" a second fron sec.
    //Theoretically the duration before and after are equivalent but outputting it might not handle it correctly.
    if(duration->tv_nsec < 0l) {
        duration->tv_nsec += SEC_IN_NANOS;
        duration->tv_sec -= 1l;
    }

    return OK;
}

volatile bool continue_unattended_benchmark_current_size;
volatile bool continue_unattended_benchmark;
volatile bool was_signal_interrupt_executed = false;
__attribute__((cold)) void interrupt_handler(const int sig) {
    was_signal_interrupt_executed = true; //let the time measurement know that the duration has been tempered with

    if(sig != SIGINT) {
        return;
    }

    print_sig("\nCtrl-C erkannt.");
    while(true) {
        print_sig("\nTesten fortführen/Nach diesem Run beenden/Nach allen Runs der aktuellen Größe beenden/Sofort beenden? [c/r/s/q] ");

        switch(getchar_sig()) {
            case 'c':
            case 'C':
                print_sig("Fahre fort...\n");
                break;

            case 'r':
            case 'R':
                print_sig("Warte auf letzten Run...\n");
                continue_unattended_benchmark = false;
                continue_unattended_benchmark_current_size = false;
                break;

            case 's':
            case 'S':
                print_sig("Warte auf letzte Runs in der aktuellen Größe...\n");
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

error unattended_benchmark_for_dimensions(const uint64_t rows, const uint64_t cols, uint32_t iterations_per_run[const matrix_mul_algorithm_count], FILE *const output, matrix_data *const a, matrix_data *const b, matrix_data cs[const 2], struct timespec *const run_duration) {
    a->no_cols = b->no_rows = cols;
    a->no_rows = b->no_cols = rows;

    //Reset arrays without freeing memory since we'll still use it
    a->row_indices.length = 0;
    a->values.length = 0;
    b->row_indices.length = 0;
    b->values.length = 0;

    for(long double filledRelativeBase = 0.; filledRelativeBase <= 1; filledRelativeBase += 0.05) { //Using floating-points in a loop is fine in this case since the resolution is high enough.
        const long double filledRelativeBaseTransformed = filledRelativeBase * filledRelativeBase;

        rng_start_values rng_state = get_current_rng_seed();
        error err = fill_matrix(a, filledRelativeBaseTransformed);
        if(err) {
            return err;
        }
        err = fill_matrix(b, filledRelativeBaseTransformed);
        if(err) {
            return err;
        }
        err = allocate_matrix_data(a->no_rows, b->no_cols, &cs[0]);
        if(err) {
            return err;
        }
        err = allocate_matrix_data(a->no_rows, b->no_cols, &cs[1]);
        if(err) {
            return err;
        }

        size_t res_index = 0;
        for(size_t algo_id = 0; algo_id < matrix_mul_algorithm_count; ++algo_id) {
            const matrix_multiplier algo = matrix_mul_algorithms[algo_id];

            struct timespec duration;
            err = execute_benchmark(a, b, &cs[res_index % 2], iterations_per_run[algo_id], algo, &duration);
            if(was_signal_interrupt_executed) {
                was_signal_interrupt_executed = false;

                //repeat measurement (duration is too long)
                --algo_id;
                continue;
            }

            matrix_data *const current_result = &cs[res_index % 2];
            const matrix_data *const previous_result = &cs[(res_index + 1) % 2];
            if(res_index > 0 &&
                    (current_result->values.length != previous_result->values.length
                    || current_result->row_indices.length != previous_result->row_indices.length
                    || current_result->no_cols != previous_result->no_cols
                    || current_result->no_rows != previous_result->no_rows
                    || memcmp(current_result->values.data, previous_result->values.data, current_result->values.length * sizeof *current_result->values.data) != 0
                    || memcmp(current_result->row_indices.data, previous_result->row_indices.data, current_result->row_indices.length * sizeof *current_result->row_indices.data) != 0
                    || memcmp(current_result->col_ptr, previous_result->col_ptr, (current_result->no_cols + 1) * sizeof *current_result->col_ptr) != 0)
              ) {
                current_result->mul_error = INCONSISTENT_RESULTS;
            } else {
                ++res_index;
            }

            if(err) {
                fprintf(output, "%lu,%lu,%Lf,%s,-,(%d; %ld),%s\n", rows, cols, 1. - filledRelativeBaseTransformed, id_to_name(algo_id), rng_state.initial_seed, rng_state.index_in_series, error_to_str(err));
            } else {
                fprintf(output, "%lu,%lu,%Lf,%s,%Lf,(%d; %ld),-\n", rows, cols, 1. - filledRelativeBaseTransformed, id_to_name(algo_id), (duration.tv_sec / (long double)SEC_IN_NANOS + duration.tv_nsec) / iterations_per_run[algo_id], rng_state.initial_seed, rng_state.index_in_series);
            }

            //Target 3 seconds of computation but a minimum of 3 iterations. Just an estimation since we're not using nsecs.
            if(duration.tv_sec > 0) {
                iterations_per_run[algo_id] = 3 * iterations_per_run[algo_id] / duration.tv_sec;
                if (iterations_per_run[algo_id] < 3) {
                    iterations_per_run[algo_id] = 3;
                }
            }

            run_duration->tv_sec += duration.tv_sec;
            run_duration->tv_nsec += duration.tv_nsec;
            run_duration->tv_sec += run_duration->tv_nsec / SEC_IN_NANOS;
            run_duration->tv_nsec %= SEC_IN_NANOS;
        }
    }

    return OK;
}
error unattended_benchmark(const uint32_t initial_iterations_per_run, const char *const export_csv) {
    signal(SIGINT, interrupt_handler);
    init_rng();

    uint32_t* iterations_per_run = alloca(matrix_mul_algorithm_count * sizeof *iterations_per_run);
    for(uint32_t algo_id = 0; algo_id < matrix_mul_algorithm_count; ++algo_id) {
        iterations_per_run[algo_id] = initial_iterations_per_run;
    }
    continue_unattended_benchmark = true;
    continue_unattended_benchmark_current_size = true;

    matrix_data a = EMPTY_MATRIX_INIT;
    matrix_data b = EMPTY_MATRIX_INIT;

    matrix_data cs[] = { EMPTY_MATRIX_INIT, EMPTY_MATRIX_INIT };

    FILE *const output = fopen(export_csv, "w");
    fprintf(output, "#Rows,#Columns,Sparseness [%% empty],Algorithm,Duration [nsec],Matrix-Seed (initial seed; rng index),Error\n");

    struct timespec run_duration = {.tv_sec = 0, .tv_nsec = 0};
    for(uint64_t row_count = 1; row_count < UINT64_MAX / 2 && continue_unattended_benchmark; ++row_count) {
        //Cover 10 equally spaced values in range #rows/2 to #rows*2 for #columns
        uint64_t increment = (row_count / 2) / 5;
        if(increment < 1) {
            increment = 1;
        }

        printf("\rBenchmarke mit Matrizen der Größe %ld (Letzter Durchlauf brauchte ca. %ld Sek.)          ", row_count, run_duration.tv_sec);
        run_duration.tv_nsec = run_duration.tv_sec = 0;

        fflush(stdout);

        for(uint64_t col_count = row_count / 2; col_count < (row_count * 2) && continue_unattended_benchmark_current_size; col_count += increment) {
            if(col_count == 0) {
                continue;
            }

            error err = unattended_benchmark_for_dimensions(row_count, col_count, iterations_per_run, output, &a, &b, cs, &run_duration);
            fflush(output); //reduce data-loss in case of Ctrl-C/q
            if (err == DIMENSION_MISMATCH || err == COULD_NOT_FETCH_TIME) {
                fprintf(stderr, "\nKritischer Fehler: %s\n", error_to_str(err));

                //quit
                continue_unattended_benchmark = false;
                break;
            }
        }
    }
    printf("\n");

    destruct_matrix_data(&a);
    destruct_matrix_data(&b);
    destruct_matrix_data(&cs[0]);
    destruct_matrix_data(&cs[1]);

    fclose(output);
    signal(SIGINT, SIG_DFL); //reset signal handler
    return OK;
}
