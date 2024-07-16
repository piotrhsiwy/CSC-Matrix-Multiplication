#ifndef GRA24CAPSPROJEKT_T064_MUL_ALGORITHMS_H
#define GRA24CAPSPROJEKT_T064_MUL_ALGORITHMS_H

#include "../error_handling/errors.h"
#include "../matrix.h"

#include "mul_scalar.h"
#include "mul_scalar_expanded.h"
#include "mul_scalar_expanded_preextract.h"
#include "mul_vector.h"
#include "mul_gustavson.h"
#include "mul_naive.h"

/**
 * A function that multiplies two given matrices in the order "<result> = <a> x <b>"
 */
typedef void (*matrix_multiplier) (const matrix_data* const a, const matrix_data* const b, matrix_data* const result);

/**
 * Converts the given id to a pointer to the implementation of the multiplication algorithm
 * @param algorithm_id the id of the algorithm
 * @return NULL if the id is out of range; a valid function pointer otherwise
 */
matrix_multiplier id_to_algorithm(unsigned int algorithm_id);

/**
 * All implemented algorithms for multiplying two csc-matrices, ordered by id
 */
extern matrix_multiplier matrix_mul_algorithms[];
/**
 * The amount of elements stored in matrix_mul_algorithms
 */
extern size_t matrix_mul_algorithm_count;

/**
 * The default multiplication algorithm. (This is just an alias for the default algorithm.)
 * @param a The first matrix
 * @param b The second matrix
 * @param result The matrix to store the result and possible errors in
 */
__attribute__((unused)) void matr_mult_csc(const void* a, const void* b, void* result);

#endif //GRA24CAPSPROJEKT_T064_MUL_ALGORITHMS_H
