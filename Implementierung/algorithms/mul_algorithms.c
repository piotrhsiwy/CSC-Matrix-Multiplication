#include "mul_algorithms.h"


matrix_multiplier matrix_mul_algorithms[] = {
        matrix_mul_scalar,                          // 0
        matrix_mul_scalar_expanded,                 // 1
        matrix_mul_scalar_expanded_preextract,      // 2
        matrix_mul_vector,                          // 3
        matrix_mul_gustavson,                       // 4
        matrix_mul_naive,                           // 5
        matrix_mul_gustavson_preextract,            // 6
};
size_t matrix_mul_algorithm_count = sizeof matrix_mul_algorithms / sizeof *matrix_mul_algorithms;

__attribute__((cold)) matrix_multiplier id_to_algorithm(const unsigned int algorithm_id) {
    if(algorithm_id >= sizeof matrix_mul_algorithms / sizeof *matrix_mul_algorithms)
        return NULL;

    return matrix_mul_algorithms[algorithm_id];
}

__attribute__((unused)) __attribute__((always_inline)) void matr_mult_csc(const void *a, const void *b, void *result) {
    matrix_mul_algorithms[0](a, b, result);
}
