#ifndef MUL_VECTOR_H
#define MUL_VECTOR_H

#include <string.h> // memset
#include "../matrix.h"
#include "../matrix_util.h"

/**
 * This algorithm utilizes @b expanded vectors (fully-sized rows and columns) which should
 * stronly limit the possible size of input matrices, namely no_rows and no_cols, but 
 * improve performance. It also silently utilizes the @b preextract optimization, that was
 * introduced for the first time in mul_scalar_expanded_preextract.
 */
__attribute__((hot)) void matrix_mul_vector(const matrix_data* const a, const matrix_data* const b, matrix_data* const result);

#endif //MUL_SCALAR_EXPANDED_H