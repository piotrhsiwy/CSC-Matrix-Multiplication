#ifndef GRA24CAPSPROJEKT_T064_MUL_ALGORITHMS_H
#define GRA24CAPSPROJEKT_T064_MUL_ALGORITHMS_H

/**
 * A function that multiplies two given matrices in the order "<result> = <a> x <b>"
 */
typedef void (*matrix_multiplier) (const void* a, const void* b, void* result);

/**
 * Converts the given id to a pointer to the implementation of the multiplication algorithm
 * @param algorithm_id the id of the algorithm
 * @return NULL if the id is out of range; a valid function pointer otherwise
 */
matrix_multiplier id_to_algorithm(unsigned int algorithm_id);

#endif //GRA24CAPSPROJEKT_T064_MUL_ALGORITHMS_H
