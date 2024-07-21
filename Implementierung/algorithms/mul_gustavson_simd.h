#ifndef MUL_GUSTAFSON_SIMD_H
#define MUL_GUSTAFSON_SIMD_H

#include "../util/matrix_util.h"
#include <string.h> // memset

// Symbol defined in ../mul_compressed_values_vector_w_scalar_SIMD.S
void mul_compressed_values_vector_w_scalar_SIMD(DYNAMIC_ARRAY(float) *, float);

/**
 * Just as matr_mult_csc_V4, but uses @b simd method above. Vectorization implemented in this routine
 * should bring perfomance benefits.
 */
__attribute__((hot)) void matr_mult_csc_V7(const void* a, const void* b, void* c);

#endif // MUL_GUSTAFSON_SIMD_H
