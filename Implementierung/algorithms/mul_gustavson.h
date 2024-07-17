#ifndef MUL_GUSTAFSON_H
#define MUL_GUSTAFSON_H

#include <string.h> // memset
#include "../util/matrix_util.h"

/**
 *https://people.csail.mit.edu/sanchez/papers/2021.gamma.asplos.pdf 
 */
__attribute__((hot)) void matrix_mul_gustavson(const void*  a, const void*  b, void*  c);

#endif //MUL_GUSTAFSON_H
