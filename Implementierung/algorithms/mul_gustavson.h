#ifndef MUL_GUSTAFSON_H
#define MUL_GUSTAFSON_H

#include <string.h> // memset
#include "../matrix.h"
#include "../matrix_util.h"

/**
 *https://people.csail.mit.edu/sanchez/papers/2021.gamma.asplos.pdf 
 */
__attribute__((hot)) void matrix_mul_gustavson(const matrix_data* const a, const matrix_data* const b, matrix_data* const result);

#endif //MUL_GUSTAFSON_H
