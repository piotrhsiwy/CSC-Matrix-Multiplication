#include "mul_algorithms.h"
#include "../util/attributes.h"

matrix_multiplier matrix_mul_algorithms[] = {
    matr_mult_csc_internal,
    matr_mult_csc_V1,
    matr_mult_csc_V2,
    matr_mult_csc_V3,
    matr_mult_csc_V4,
    matr_mult_csc_V5,
    matr_mult_csc_V6,
    matr_mult_csc_V7,
};
const char* matrix_mul_algorithm_names[] = {
    "scalar", // 0
    "scalar_expanded", // 1
    "scalar_expanded_preextract", // 2
    "vector", // 3
    "gustavson", // 4
    "naive", // 5
    "scalar_expanded_preextract_simd", // 6
    "gustavson_simd", // 7
};

size_t matrix_mul_algorithm_count = sizeof matrix_mul_algorithms / sizeof *matrix_mul_algorithms;
_Static_assert((sizeof matrix_mul_algorithms / sizeof *matrix_mul_algorithms) == (sizeof matrix_mul_algorithm_names / sizeof *matrix_mul_algorithm_names),
    "Length mismatch between matrix_mul_algorithms and matrix_mul_algorithm_names!");

__attribute__((cold)) matrix_multiplier id_to_algorithm(const unsigned int algorithm_id)
{
    if (algorithm_id >= matrix_mul_algorithm_count)
        return NULL;

    return matrix_mul_algorithms[algorithm_id];
}
const char* id_to_name(const unsigned int algorithm_id)
{
    if (algorithm_id >= matrix_mul_algorithm_count)
        return NULL;

    return matrix_mul_algorithm_names[algorithm_id];
}

__attribute__((unused)) always_inline void matr_mult_csc(void* a, void* b, void* result)
{
    matrix_mul_algorithms[0](a, b, result);
}
