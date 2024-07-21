#ifndef MUL_VECTOR_H
#define MUL_VECTOR_H

#include <string.h> // memset
#include "../util/matrix_util.h"

/**
 * This algorithm utilizes @b expanded vectors (fully-sized rows and columns) which should
 * stronly limit the possible size of input matrices, namely no_rows and no_cols, but 
 * improve performance. It also silently utilizes the @b preextract optimization, that was
 * introduced for the first time in mul_scalar_expanded_preextract.
 */
__attribute__((hot)) void matrix_mul_vector(const void*  a, const void*  b, void*  c);

#endif //MUL_VECTOR_H
