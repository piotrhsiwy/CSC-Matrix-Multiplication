#ifndef MUL_SCALAR_EXPANDED_PREEXTRACT_SIMD_H
#define MUL_SCALAR_EXPANDED_PREEXTRACT_SIMD_H

#include "../util/matrix_util.h"
#include <string.h> // memset

// Symbol defined in ../mul_vectors_expanded_SIMD.S
float mul_vectors_expanded_SIMD(size_t, float*, float*);

/**
 * Like matr_mult_csc_V2 so @b expands and @b preextracts but it
 * uses @b simd when multiplying/h-adding the expanded vectors. This should bring some performance benefits.
 */
__attribute__((hot)) void matr_mult_csc_V6(const void* a, const void* b, void* c);

#endif // MUL_SCALAR_EXPANDED_PREEXTRACT_SIMD_H
