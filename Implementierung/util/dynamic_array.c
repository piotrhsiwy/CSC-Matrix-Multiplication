#include "dynamic_array.h"


void dynamic_array_destroy(generic_dynamic_array *const arr) {
    free(arr->data);
    dynamic_array_unbind(arr);
}

void dynamic_array_unbind(generic_dynamic_array *const arr) {
    arr->data = NULL;
    arr->capacity = arr->length = 0;
}
#include <stdio.h>
#define DYNAMIC_ARRAY_DECL(T)                                                                                \
    int ___dynamic_array_##T##_macro_defined_add(DYNAMIC_ARRAY(T) *const arr, T v) {                         \
        if(arr->capacity <= arr->length) {                                                                   \
            T* buf = (T*)reallocarray(arr->data, arr->capacity * 2, sizeof(T));                              \
            if(!buf){                                                                                        \   
                return -1; /*TODO: try increasing by only 1*/                                                \
            } else{                                                                                          \
                arr->data = buf;                                                                             \ 
                arr->capacity *= 2;                                                                          \
            }                                                                                                \      
        }                                                                                                    \
        arr->data[arr->length] = v;                                                                          \
        arr->length++;                                                                                       \
        return 0;                                                                                            \
    }                                                                                                        \
    T __attribute__((pure)) ___dynamic_array_##T##_macro_defined_at(                                         \
      const DYNAMIC_ARRAY(T) *const arr, const size_t index) {                                               \
        return arr->data[index];                                                                             \
    }                                                                                                        \
    int ___dynamic_array_##T##_macro_defined_new(size_t capacity, DYNAMIC_ARRAY(T) *const arr) {             \
        if(!(arr->data = (T*)malloc(capacity * sizeof(T))))                                                  \
            return -1;                                                                                       \
        arr->length = 0;                                                                                     \
        arr->capacity = capacity;                                                                            \
        return 0;                                                                                            \
    }

DYNAMIC_ARRAY_DECL(float)
DYNAMIC_ARRAY_DECL(uint64_t)
DYNAMIC_ARRAY_DECL(pair)
