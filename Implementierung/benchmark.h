#ifndef GRA24CAPSPROJEKT_T064_BENCHMARK_H
#define GRA24CAPSPROJEKT_T064_BENCHMARK_H

#include "matrix.h"
#include "algorithms/mul_algorithms.h"
#include "config.h"

#include <time.h>


/**
 * Performs a matrix multiplication multiple times and measures the time
 * @param a the first matrix of the multiplication
 * @param b the seconds matrix of the multiplication
 * @param result the place to store the result
 * @param iterations the amount of times to run the multiplication
 * @param algorithm the algorithm to use
 * @param duration The time it took for perform the multiplication the given amount of times. The value pointed to will
 *                 be set after execution, if no error is returned.
 * @returns The error if one occurred, OK otherwise.
 */
__attribute__((hot)) error execute_benchmark(const matrix_data* a, const matrix_data* b, matrix_data* result, uint32_t iterations, matrix_multiplier algorithm, struct timespec* duration);

/**
 * Performs benchmarks each algorithm on multiple matrix sizes and sparseness and exports the results to the given
 * directory.
 * @param iterations_per_run The amount of iterations to run for each combination of parameters
 * @param export_directory The directory where the files for the benchmarking results will be stored
 * @return The error if one occurred, OK otherwise.
 */
error unattended_benchmark(const uint32_t iterations_per_run, const char *const export_directory);

#endif //GRA24CAPSPROJEKT_T064_BENCHMARK_H
