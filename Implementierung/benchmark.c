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
        algorithm(a, b, result);
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
