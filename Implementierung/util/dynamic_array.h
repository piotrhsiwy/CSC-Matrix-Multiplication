#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h>
#include <stdint.h>
#include "../matrix_util.h" // for the dynamic pair-array

// Where do we use this?
// - in algorithms/mul_scalar.c we use it to allocate unknown-length arrays that are part of the matrix_data: row_indices and values
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

#define DYNAMIC_ARRAY_HEADER___(T)                                                                                     \
    typedef struct {                                                                                                   \
        T* data;                                                                                                       \
        size_t length;                                                                                                 \
        size_t capacity;                                                                                               \
    } DYNAMIC_ARRAY(T);                                                                                                \
    int ___dynamic_array_##T##_macro_defined_add(DYNAMIC_ARRAY(T)* arr, T v);                                          \
    T __attribute__((pure)) ___dynamic_array_##T##_macro_defined_at(const DYNAMIC_ARRAY(T)* arr, const size_t index);  \
    const char* ___dynamic_array_##T##_macro_defined_new(size_t capacity, DYNAMIC_ARRAY(T)* arr);

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

DYNAMIC_ARRAY_HEADER___(float)
DYNAMIC_ARRAY_HEADER___(uint64_t)
DYNAMIC_ARRAY_HEADER___(pair)

#endif //DYNAMIC_ARRAY_H
