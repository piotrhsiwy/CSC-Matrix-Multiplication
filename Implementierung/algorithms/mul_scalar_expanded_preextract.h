
#ifndef MUL_SCALAR_EXPANDED_PREEXTRACT_H
#define MUL_SCALAR_EXPANDED_PREEXTRACT_H

#include <string.h> // memset
#include "../matrix.h"
#include "../matrix_util.h"

/**
 * like matrix_mul_scalar_expanded, but it extracts all of the rows of matrix A, before entering the two nested-for loops.
 * This should result in the row_indices having been iterated over only once, as opposed to n x m (matrix_A) times.
 * (+) it promises some performance improvements
 * (-) this function will face sharp limits due storage space by where no_rows and no_cols of matrix_A are the limiting factors.
 */
__attribute__((hot)) error matrix_mul_scalar_expanded_preextract(const matrix_data* const a, const matrix_data* const b, matrix_data* const result);

#endif //MUL_SCALAR_EXPANDED_H
