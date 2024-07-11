#include "dynamic_array.h"

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
    void ___dynamic_array_##T##_macro_defined_set(                                                           \
        const DYNAMIC_ARRAY(T)* arr, const size_t index, const T v){                                         \
        arr->data[index] = v;                                                                                \
    }                                                                                                        \
    /* the capacity might not be multiple of 2 after concat! */                                              \
    int ___dynamic_array_##T##_macro_defined_concat(                                                         \
        DYNAMIC_ARRAY(T)* dest, const DYNAMIC_ARRAY(T)* src){                                                \
        if(dest->capacity < dest->length + src-> length){                                                    \
            T* buf = reallocarray(dest->data, dest->length + src->length, sizeof(T));                        \
            if(!buf){                                                                                        \
                int err = errno;                                                                             \
                errno = 0;                                                                                   \
                return err;                                                                                  \
            }                                                                                                \
            dest->data = buf;                                                                                \
            dest->capacity = dest->length + src->length;                                                     \
        }                                                                                                    \
        T* uninitialized_from = dest->data + dest->length;                                                   \
        size_t how_many_copy = src->length * sizeof(T);                                                      \
        memmove(uninitialized_from, src->data, how_many_copy);                                               \
        dest->length = dest->length + src->length;                                                           \
        return 0;                                                                                            \
    }                                                                                                        \
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
DYNAMIC_ARRAY_DECL(___dynamic_array_pair_macro_defined)


void dynamic_array_destroy_top_layer(DYNAMIC_ARRAY_2D* arr){
    for (size_t i = 0; i < arr->length; i++)
    {
        dynamic_array_destroy( (generic_dynamic_array*)( arr->data ) );
    }
    arr->length = 0;
}

void dynamic_array_recursive_destroy(DYNAMIC_ARRAY_2D* arr){
    dynamic_array_destroy_top_layer(arr);
    dynamic_array_destroy((generic_dynamic_array*) arr);
}

void dynamic_array_destroy(generic_dynamic_array* arr) {
    free(arr->data);
    dynamic_array_unbind(arr);
}

void dynamic_array_unbind(generic_dynamic_array* arr) {
    arr->data = NULL;
    arr->capacity = arr->length = 0;
}
