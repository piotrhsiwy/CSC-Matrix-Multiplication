#ifndef MATRIX_UTIL_H
#define MATRIX_UTIL_H

#include <stdint.h> // uint64_t
#include "errors.h"

/*
 * Represents a (index,value) pair that is very often used to store either row or column of a sparse matrix
 *
 * */
typedef struct {uint64_t index; float value;} pair;

/*
 * @param 
 *
 *
 * */
error expand_row()

#endif // MATRIX_UTIL_H