#pragma clang diagnostic push
#pragma ide diagnostic ignored "cert-msc50-cpp"

#include "rng.h"

#include <stdlib.h>


unsigned int start_seed;
uint64_t index_in_series;

void init_rng(void) {
    srand(start_seed = 123456789);
    index_in_series = 0;
}

__attribute__((unused)) void init_rng_with_seed(rng_start_values seed) {
    srand(start_seed = seed.initial_seed);
    for(index_in_series = 0; index_in_series < seed.index_in_series; ++index_in_series) {
        rand();
    }
}

__attribute__((pure)) rng_start_values get_current_rng_seed(void) {
    rng_start_values result = {.initial_seed = start_seed, .index_in_series = index_in_series};
    return result;
}

int next_random_number(void) {
    ++index_in_series;
    return rand();
}

#pragma clang diagnostic pop
