#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h>
#include <stdint.h>


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
#define dynamic_array_at(T, arr, i) ___dynamic_array_##T##_macro_defined_at(arr, i)
#define dynamic_array_new(T, capacity, arr) ___dynamic_array_##T##_macro_defined_new(capacity, arr)
#define DYNAMIC_ARRAY_DECL(T)                                                                                \
    int ___dynamic_array_##T##_macro_defined_add(DYNAMIC_ARRAY(T) *const arr, T v) {                         \
        if(arr->capacity < ++arr->length) {                                                                  \
            T* buf = (T*)realloc(arr->data, sizeof(T) * (arr->capacity *= 2));                               \
            if(!buf)                                                                                         \
                return -1; /*TODO: try increasing by only 1*/                                                \
            arr->data;                                                                                       \
        }                                                                                                    \
                                                                                                             \
        arr->data[arr->length - 1] = v;                                                                      \
                                                                                                             \
        return 0;                                                                                            \
    }                                                                                                        \
    T __attribute__((pure)) ___dynamic_array_##T##_macro_defined_at(                                         \
      const DYNAMIC_ARRAY(T) *const arr, const size_t index) {                                               \
        return arr->data[index];                                                                             \
    }                                                                                                        \
    const char* ___dynamic_array_##T##_macro_defined_new(size_t capacity, DYNAMIC_ARRAY(T) *const arr) {     \
        if(!(arr->data = (T*)malloc(capacity * sizeof(T))))                                                  \
            return "Out of memory";                                                                          \
        arr->length = 0;                                                                                     \
        arr->capacity = capacity;                                                                            \
        return NULL;                                                                                         \
    }


//DYNAMIC_ARRAY_DECL(float)
//DYNAMIC_ARRAY_DECL(uint64_t)



#define DYNAMIC_ARRAY_HEADER(T) \
    typedef struct {                                                                                         \
        T* data;                                                                                             \
        size_t length;                                                                                       \
        size_t capacity;                                                                                     \
    } DYNAMIC_ARRAY(T);                                                                                      \
    int __dynamic_array_##T##_macro_defined_add(DYNAMIC_ARRAY(T) *const arr, T v); \
    T __attribute__((pure)) ___dynamic_array_##T##_macro_defined_at(                                         \
      const DYNAMIC_ARRAY(T) *const arr, const size_t index);                      \
    const char* ___dynamic_array_##T##_macro_defined_new(size_t capacity, DYNAMIC_ARRAY(T) *const arr);

/**
     * Clears all fields of the given array (including als pointers to allocated memory).
     * This is useful when e. g. transferring ownership of the managed memory but you are on own now to free it.
     * @param arr The array to clear
 */
void dynamic_array_unbind(generic_dynamic_array *const arr);

/**
     * Frees the given array and clears it's fields to avoid accidental double free bugs.
     * @param arr The array to destroy
 */
void dynamic_array_destroy(generic_dynamic_array *const arr);

DYNAMIC_ARRAY_HEADER(float)
DYNAMIC_ARRAY_HEADER(uint64_t)



#endif //DYNAMIC_ARRAY_H
