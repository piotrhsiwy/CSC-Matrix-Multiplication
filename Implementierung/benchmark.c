#include "benchmark.h"
#include "util/signal_io.h"

#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#define SEC_IN_NANOS 1000000000l


error execute_benchmark(const matrix_data* a, const matrix_data* b, matrix_data *const result, const uint32_t iterations, const matrix_multiplier algorithm, struct timespec *const duration) {
    // We don't want to benchmark the memory allocator --> preallocate.
    // This should also load the function (instr.) and structures (data) into cache.
    algorithm(a, b, result);

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

    //Due to subtraction we might end up with negative values for nanoseconds. -> "Borrow" a second fron sec.
    //Theoretically the duration before and after are equivalent but outputting it might not handle it correctly.
    if(duration->tv_nsec < 0l) {
        duration->tv_nsec += SEC_IN_NANOS;
        duration->tv_sec -= 1l;
    }

    return OK;
}

volatile bool continue_unattended_benchmark;
volatile bool was_signal_interrupt_executed = false;
void interrupt_handler(const int sig) {
    was_signal_interrupt_executed = true; //let the time measurement know that the duration has been tempered with

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

error unattended_benchmark_for_dimensions(const uint64_t rows, const uint64_t cols, const uint32_t iterations_per_run, FILE *const output, matrix_data *const a, matrix_data *const b, matrix_data *const c, struct timespec *const run_duration) {
    a->no_cols = b->no_rows = cols;
    a->no_rows = b->no_cols = rows;

    //Reset arrays without freeing memory since we'll still use it
    a->dynamic_values_x_row_indices.length = 0;
    b->dynamic_values_x_row_indices.length = 0;

    for(uint8_t filledPercent = 0; filledPercent <= 100; ++filledPercent) {
        //TODO: handle errors
        error err = fill_matrix(a, filledPercent);
        if(err) {
            return err;
        }
        err = fill_matrix(b, filledPercent);
        if(err) {
            return err;
        }
        err = allocate_matrix_data(a->no_rows, b->no_cols, c);
        if(err) {
            return err;
        }

        for(size_t algo_id = 0; algo_id < matrix_mul_algorithm_count; ++algo_id) {
            const matrix_multiplier algo = matrix_mul_algorithms[algo_id];

            struct timespec duration;
            err = execute_benchmark(a, b, c, iterations_per_run, algo, &duration);
            if(was_signal_interrupt_executed) {
                was_signal_interrupt_executed = false;

                //repeat measurement (duration is too long)
                --algo_id;
                continue;
            }

            if(err) {
                fprintf(output, "%lu,%lu,%d,%s,-,%s\n", rows, cols, 100 - filledPercent, id_to_name(algo_id), error_to_str(err));
            } else {
                fprintf(output, "%lu,%lu,%d,%s,%f,-\n", rows, cols, 100 - filledPercent, id_to_name(algo_id), (duration.tv_sec / (double)SEC_IN_NANOS + duration.tv_nsec) / iterations_per_run);
            }

            run_duration->tv_sec += duration.tv_sec;
            run_duration->tv_sec += run_duration->tv_nsec / SEC_IN_NANOS;
            run_duration->tv_nsec += duration.tv_nsec;
            run_duration->tv_nsec %= SEC_IN_NANOS;
        }
    }

    return OK;
}
error unattended_benchmark(const uint32_t iterations_per_run, const char *const export_csv) {
    signal(SIGINT, interrupt_handler);
    srand(123456789); //We want predictable numbers in our matrices.

    continue_unattended_benchmark = true;

    matrix_data a = EMPTY_MATRIX_INIT;
    matrix_data b = EMPTY_MATRIX_INIT;

    matrix_data c = EMPTY_MATRIX_INIT;

    FILE *const output = fopen(export_csv, "w");
    fprintf(output, "#Rows,#Columns,Sparseness [%% empty],Algorithm,Duration [nsec],Error\n");

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

        for(uint64_t col_count = row_count / 2; col_count < (row_count * 2) && continue_unattended_benchmark; col_count += increment) {
            //TODO: different column counts for a and b
            if(col_count == 0) {
                continue;
            }

            //TODO: handle error
            unattended_benchmark_for_dimensions(row_count, col_count, iterations_per_run, output, &a, &b, &c, &run_duration);
            fflush(output); //reduce data-loss in case of Ctrl-C/q
        }
    }
    printf("\n");

    destruct_matrix_data(&a);
    destruct_matrix_data(&b);
    destruct_matrix_data(&c);

    fclose(output);
    signal(SIGINT, SIG_DFL); //reset signal handler
    return OK;
}
