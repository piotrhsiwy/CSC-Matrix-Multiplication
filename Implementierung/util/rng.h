#ifndef GRA24CAPSPROJEKT_T064_RNG_H
#define GRA24CAPSPROJEKT_T064_RNG_H

#include <stdint.h>

typedef struct {
    unsigned int initial_seed;
    uint64_t index_in_series;
} rng_start_values;

/**
 * Sets the initial seed for the random number generator
 */
void init_rng(void);

/**
 * Sets the initial seed for the random number generator
 */
__attribute__((unused)) void init_rng_with_seed(rng_start_values seed);

/**
 * Gets all the info required to recreate the next pseudo-random values.
 * @return the values
 */
rng_start_values get_current_rng_seed(void);

/**
 * Generates the next random number and updates the result of get_current_rng_seed accordingly
 * @warning Do not use rand() directly when relying on the result of get_current_rng_seed!
 * @return a pseudo random number
 */
int next_random_number(void);

#endif // GRA24CAPSPROJEKT_T064_RNG_H
