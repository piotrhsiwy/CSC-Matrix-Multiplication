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

DYNAMIC_ARRAY_DECL(float)
DYNAMIC_ARRAY_DECL(uint64_t)
