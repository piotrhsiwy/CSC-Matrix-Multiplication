#ifndef MUL_GUSTAFSON_SIMD_H
#define MUL_GUSTAFSON_SIMD_H

#include <string.h> // memset
#include "../util/matrix_util.h"


// Symbol defined in ../mul_vector_compressed_w_scalar_SIMD.S
void mul_vector_compressed_w_scalar_SIMD(DYNAMIC_ARRAY(float)*, const float);

/**
 * Just as matrix_mul_gustavson, but uses @b simd method above. Vectorization implemented in this routine
 * should bring perfomance benefits.
 */
__attribute__((hot)) void matrix_mul_gustavson_simd(const void*  a, const void*  b, void*  c);

#endif //MUL_GUSTAFSON_SIMD_H
