#include "mul_algorithms.h"
#include "mul_naive.h"


matrix_multiplier algorithms[] = {matrix_mult_csc };

__attribute__((always_inline)) matrix_multiplier id_to_algorithm(const unsigned int algorithm_id) {
    if(algorithm_id >= sizeof algorithms / sizeof *algorithms)
        return NULL;

    return algorithms[algorithm_id];
}
