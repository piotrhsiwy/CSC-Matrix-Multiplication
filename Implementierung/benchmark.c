#include "benchmark.h"

#define SEC_IN_NANOS 1000000000l


struct timespec execute_benchmark(const matrix_data *const a, const matrix_data *const b, matrix_data *const result, const uint32_t iterations, const matrix_multiplier algorithm) {
    struct timespec start;
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for(uint32_t i = 0; i < iterations; ++i)
        algorithm(a, b, result);

    clock_gettime(CLOCK_MONOTONIC, &end);

    end.tv_sec -= start.tv_sec;
    end.tv_nsec -= start.tv_nsec;

    if(end.tv_nsec < 0l) {
        end.tv_nsec += SEC_IN_NANOS;
        end.tv_sec -= 1l;
    }

    return end;
}
