
#ifndef MUL_SCALAR_EXPANDED_PREEXTRACT_H
#define MUL_SCALAR_EXPANDED_PREEXTRACT_H

#include <string.h> // memset
#include "../util/matrix_util.h"

/**
 * like matrix_mul_scalar_expanded, but it extracts all of the rows of matrix A, before entering the two nested-for loops.
 * This should result in the row_indices having been iterated over only once, as opposed to n x m (matrix_A) times.
 * (+) it promises some performance improvements. In comparison should be faster for more densly populated matrices, because
 *   a) the longer the values array, the bigger advantage will be made by traversing it only once
 *   b) this function cannot sort-out empty rows in advance, so the less of them, the bigger will the advantage be
 * (-) this function will face sharp limits due storage space by where no_rows and no_cols of matrix_A are the limiting factors.
 */
__attribute__((hot)) void matrix_mul_scalar_expanded_preextract(const void*  a, const void*  b, void*  c);

#endif //MUL_SCALAR_EXPANDED_H
