#ifndef MUL_GUSTAFSON_SIMD_H
#define MUL_GUSTAFSON_SIMD_H

#include <string.h> // memset
#include "../util/matrix_util.h"

void mul_vector_compressed_w_scalar_SIMD(DYNAMIC_ARRAY(pair)*, const float);

/**
 *https://people.csail.mit.edu/sanchez/papers/2021.gamma.asplos.pdf 
 */
__attribute__((hot)) void matrix_mul_gustavson_simd(const void*  a, const void*  b, void*  c);

#endif //MUL_GUSTAFSON_SIMD_H
