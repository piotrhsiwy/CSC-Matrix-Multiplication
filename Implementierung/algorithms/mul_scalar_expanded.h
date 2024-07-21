#ifndef MUL_SCALAR_EXPANDED_H
#define MUL_SCALAR_EXPANDED_H

#include "../util/matrix_util.h"
#include <string.h> // memset

/**
 * like matr_mult_csc_internal (See mul_scalar.h), but @b expands the vectors before multiplying, making some
 * potential optimizations possible. This is only possible if respectively no_rows
 * and no_cols are small enough so that we are able to allocate an array of pairs
 * of that size.
 * */
__attribute__((hot)) void matr_mult_csc_V1(const void* a, const void* b, void* c);

#endif // MUL_SCALAR_EXPANDED_H
