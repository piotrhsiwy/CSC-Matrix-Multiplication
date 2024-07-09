#ifndef PAIR_H
#define PAIR_H

#include <stdint.h>

/*
 * Represents a (index,value) pair that is very often used to store either row or column of a sparse matrix
 *
 * */
typedef struct {uint64_t index; float value;} pair;

#endif // PAIR_H