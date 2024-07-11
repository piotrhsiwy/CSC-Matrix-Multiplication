#include "dynamic_array.h"


void dynamic_array_destroy(generic_dynamic_array *const arr) {
    free(arr->data);
    dynamic_array_unbind(arr);
}

void dynamic_array_unbind(generic_dynamic_array *const arr) {
    arr->data = NULL;
    arr->capacity = arr->length = 0;
}

#define DYNAMIC_ARRAY_DECL(T)                                                                                \
    int ___dynamic_array_##T##_macro_defined_add(DYNAMIC_ARRAY(T) *const arr, T v) {                         \
        if(arr->capacity <= arr->length) {                                                                   \
            T* buf = (T*)reallocarray(arr->data, arr->capacity * 2, sizeof(T));                              \
            if(!buf){                                                                                        \
                int err = errno;                                                                             \
                errno = 0;                                                                                   \
                return err; /*TODO: try increasing by only 1*/                                               \
            } else{                                                                                          \
                arr->data = buf;                                                                             \
                arr->capacity *= 2;                                                                          \
                T* uninitialized_from = arr->data + arr->length;                                             \
                size_t unitialized_how_many = ( arr->capacity - arr->length ) * sizeof(T);                   \
                memset(uninitialized_from, 0, unitialized_how_many);                                         \
            }                                                                                                \
        }                                                                                                    \
        arr->data[arr->length] = v;                                                                          \
        arr->length++;                                                                                       \
        return 0;                                                                                            \
    }                                                                                                        \
    /*void ___dynamic_array_##T##_macro_defined_set(                                                         \
        const DYNAMIC_ARRAY(T)* arr, const size_t index, const T v){                                         \
       arr->data[index] = v;                                                                                 \
    }*/                                                                                                      \
    T __attribute__((pure)) ___dynamic_array_##T##_macro_defined_at(                                         \
      const DYNAMIC_ARRAY(T) *const arr, const size_t index) {                                               \
        return arr->data[index];                                                                             \
    }                                                                                                        \
    int ___dynamic_array_##T##_macro_defined_new(size_t capacity, DYNAMIC_ARRAY(T) *const arr) {             \
        if(!(arr->data = (T*)reallocarray(NULL, capacity, sizeof(T)))) {                                     \
            int err = errno;                                                                                 \
            errno = 0;                                                                                       \
            return err;                                                                                      \
        }                                                                                                    \
        memset(arr->data, 0, capacity * sizeof(T));                                                          \
        arr->length = 0;                                                                                     \
        arr->capacity = capacity;                                                                            \
        return 0;                                                                                            \
    }

DYNAMIC_ARRAY_DECL(float)
DYNAMIC_ARRAY_DECL(uint64_t)
DYNAMIC_ARRAY_DECL(pair)
