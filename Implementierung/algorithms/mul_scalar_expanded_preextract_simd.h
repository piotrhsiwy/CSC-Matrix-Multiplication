#ifndef MUL_SCALAR_EXPANDED_PREEXTRACT_SIMD_H
#define MUL_SCALAR_EXPANDED_PREEXTRACT_SIMD_H

#include <string.h> // memset
#include "../util/matrix_util.h"

float mul_vectors_expanded_SIMD(size_t, float* const, float* const);

/**
 * like matrix_mul_scalar_expanded_preextract, but it uses @b simd  when multiplying/h-adding the expanded vectors
 */
__attribute__((hot)) void matrix_mul_scalar_expanded_preextract_simd(const void* a, const void* b, void* c);

#endif //MUL_SCALAR_EXPANDED_PREEXTRACT_SIMD_H
