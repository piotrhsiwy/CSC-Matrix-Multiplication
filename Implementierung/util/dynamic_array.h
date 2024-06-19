#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stdlib.h>


#define DYNAMIC_ARRAY(T) ___dynamic_array_##T##_macro_defined

#define dynamic_array_add(T, arr, v) ___dynamic_array_##T##_macro_defined_add(arr, v)
#define dynamic_array_at(T, arr, i) ___dynamic_array_##T##_macro_defined_at(arr, i)
#define dynamic_array_new(T, capacity, arr) ___dynamic_array_##T##_macro_defined_new(capacity, arr)
#define DYNAMIC_ARRAY_DECL(T)                                                                                \
    typedef struct {                                                                                         \
        T* data;                                                                                             \
        size_t length;                                                                                       \
        size_t capacity;                                                                                     \
    } DYNAMIC_ARRAY(T);                                                                                      \
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
    const char* ___dynamic_array_##T##_macro_defined_new(size_t capacity, DYNAMIC_ARRAY(T) *const arr) {   \
        if(!(arr->data = (T*)malloc(capacity * sizeof(T))))                                                  \
            return "Out of memory";                                                                          \
        arr->length = 0;                                                                                     \
        arr->capacity = capacity;                                                                            \
        return NULL;                                                                                         \
    }

DYNAMIC_ARRAY_DECL(float)



#endif //DYNAMIC_ARRAY_H
