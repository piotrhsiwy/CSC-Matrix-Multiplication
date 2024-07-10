#include "mul_algorithms.h"

matrix_multiplier algorithms[] = { matrix_mul_scalar, matrix_mul_scalar_expanded  };

__attribute__((cold)) matrix_multiplier id_to_algorithm(const unsigned int algorithm_id) {
    if(algorithm_id >= sizeof algorithms / sizeof *algorithms)
        return NULL;

    return algorithms[algorithm_id];
}
