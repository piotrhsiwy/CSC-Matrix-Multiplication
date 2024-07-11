#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#define _GNU_SOURCE // needed if we use reallocarray() instead of realloc. See man realloc
#include <string.h> // memset
#include <stdint.h> // size_t, uint64_t
#include <errno.h> // errno
#include <stdlib.h> // reallocarray, free
#include "pair.h"

// Where do we use this?
// - in matrix_mul_algorithms/mul_scalar.c we use it to allocate unknown-length arrays that are part of the matrix_data: row_indices and values
// - in matrix_util.c we use it to allocate pair-arrays when extracting compressed rows and columns from matrices

/**
 * A DYNAMIC_ARRAY(void) that guarantees to have the same memory layout as any specific DYNAMIC_ARRAY.
 */
typedef struct {
    void* data;
    size_t length;
    size_t capacity;
} generic_dynamic_array;

/// A poor man's std::vector<T>. "Poor man" as in "C developer".
#define DYNAMIC_ARRAY(T) ___dynamic_array_##T##_macro_defined
#define dynamic_array_add(T, arr, v) ___dynamic_array_##T##_macro_defined_add(arr, v)
#define dynamic_array_set(T, arr, i, v) __dynamic_array_##T##_macro_defined_set(arr, i, v)
#define dynamic_array_concat(T, dest, src) ___dynamic_array_##T##_macro_defined_concat(dest, src)
#define dynamic_array_at(T, arr, i) ___dynamic_array_##T##_macro_defined_at(arr, i)
#define dynamic_array_new(T, capacity, arr) ___dynamic_array_##T##_macro_defined_new(capacity, arr)

#define DYNAMIC_ARRAY_HEADER___(T)                                                                                     \
    typedef struct {                                                                                                   \
        T* data;                                                                                                       \
        size_t length;                                                                                                 \
        size_t capacity;                                                                                               \
    } DYNAMIC_ARRAY(T);                                                                                                \
    int ___dynamic_array_##T##_macro_defined_add(DYNAMIC_ARRAY(T)* arr, T v);                                          \
    void ___dynamic_array_##T##_macro_defined_set(const DYNAMIC_ARRAY(T)* arr, const size_t index, const T v);         \
    int ___dynamic_array_##T##_macro_defined_concat(DYNAMIC_ARRAY(T)* dest, const DYNAMIC_ARRAY(T)* src);       \
    T __attribute__((pure)) ___dynamic_array_##T##_macro_defined_at(const DYNAMIC_ARRAY(T)* arr, const size_t index);  \
    int ___dynamic_array_##T##_macro_defined_new(size_t capacity, DYNAMIC_ARRAY(T)* arr);

DYNAMIC_ARRAY_HEADER___(float)
DYNAMIC_ARRAY_HEADER___(uint64_t)
DYNAMIC_ARRAY_HEADER___(pair)
typedef DYNAMIC_ARRAY(pair) pair2D;
DYNAMIC_ARRAY_HEADER___(pair2D)

/**
 * Clears all fields of the given array (including als pointers to allocated memory).
 * This is useful when e. g. transferring ownership of the managed memory but you are on your own now to free it.
 * @param arr The array to clear
 */
void dynamic_array_unbind(generic_dynamic_array* arr);

/**
 * Frees the given array and clears it's fields to avoid accidental double free bugs.
 * @param arr The array to destroy
 */
void dynamic_array_destroy(generic_dynamic_array* arr);

/**
 * Recursivels calls dynamic_array_destroy on a 2D dynamic array
 */
void dynamic_array_recursive_destroy(DYNAMIC_ARRAY(pair2D)* arr);

/**
 * calls dynamic_array_destroy on a all elements of a 2D dynamic array
 */
void dynamic_array_destroy_top_layer(DYNAMIC_ARRAY(pair2D)* arr);

#endif //DYNAMIC_ARRAY_H
