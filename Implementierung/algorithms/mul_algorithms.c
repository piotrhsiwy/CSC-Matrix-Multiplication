#include "mul_algorithms.h"
#include "mul_scalar.h"


matrix_multiplier algorithms[] = { matrix_mul_scalar };

__attribute__((cold)) matrix_multiplier id_to_algorithm(const unsigned int algorithm_id) {
    if(algorithm_id >= sizeof algorithms / sizeof *algorithms)
        return NULL;

    return algorithms[algorithm_id];
}
