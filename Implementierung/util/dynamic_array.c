#include "dynamic_array.h"

#define DYNAMIC_ARRAY_DECL(T)                                                                            \
    int ___dynamic_array_##T##_macro_defined_reserve(size_t capacity, DYNAMIC_ARRAY(T)* const arr)       \
    {                                                                                                    \
        if (capacity <= arr->capacity) {                                                                 \
            return 0;                                                                                    \
        }                                                                                                \
        T* buf = (T*)realloc(arr->data, capacity * sizeof(T)); /*zeroing free memory would be overkill*/ \
        if (!buf) {                                                                                      \
            int err = errno;                                                                             \
            errno = 0;                                                                                   \
            return err;                                                                                  \
        }                                                                                                \
                                                                                                         \
        arr->data = buf;                                                                                 \
        arr->capacity = capacity;                                                                        \
        return 0;                                                                                        \
    }                                                                                                    \
    int ___dynamic_array_##T##_macro_defined_add(DYNAMIC_ARRAY(T)* const arr, T v)                       \
    {                                                                                                    \
        if (arr->capacity <= arr->length) {                                                              \
            if (dynamic_array_reserve(T, arr->capacity * 2, arr)) {                                      \
                int err = dynamic_array_reserve(T, arr->capacity + 1, arr);                              \
                if (err) {                                                                               \
                    return err;                                                                          \
                }                                                                                        \
            }                                                                                            \
        }                                                                                                \
        arr->data[arr->length] = v;                                                                      \
        arr->length++;                                                                                   \
        return 0;                                                                                        \
    }                                                                                                    \
    int ___dynamic_array_##T##_macro_defined_concat(                                                     \
        DYNAMIC_ARRAY(T) * dest, const DYNAMIC_ARRAY(T) * src)                                           \
    {                                                                                                    \
        if (dest->capacity < dest->length + src->length) {                                               \
            int err = dynamic_array_reserve(T, dest->length + src->length, dest);                        \
            if (err) {                                                                                   \
                return err;                                                                              \
            }                                                                                            \
        }                                                                                                \
        T* uninitialized_from = dest->data + dest->length;                                               \
        size_t how_many_copy = src->length * sizeof(T);                                                  \
        memmove(uninitialized_from, src->data, how_many_copy);                                           \
        dest->length = dest->length + src->length;                                                       \
        return 0;                                                                                        \
    }                                                                                                    \
    int ___dynamic_array_##T##_macro_defined_new(size_t capacity, DYNAMIC_ARRAY(T)* const arr)           \
    {                                                                                                    \
        if (!(arr->data = (T*)malloc(capacity * sizeof(T)))) {                                           \
            int err = errno;                                                                             \
            errno = 0;                                                                                   \
            return err;                                                                                  \
        }                                                                                                \
        arr->length = 0;                                                                                 \
        arr->capacity = capacity;                                                                        \
        return 0; /*zeroing memory would be overkill*/                                                   \
    }

DYNAMIC_ARRAY_DECL(float)
DYNAMIC_ARRAY_DECL(___dynamic_array_float_macro_defined)
DYNAMIC_ARRAY_DECL(uint64_t)
DYNAMIC_ARRAY_DECL(___dynamic_array_uint64_t_macro_defined)
DYNAMIC_ARRAY_DECL(pair)

void dynamic_array_destroy_top_layer(generic_dynamic_array* arr)
{
    generic_dynamic_array* base = arr->data;
    for (size_t i = 0; i < arr->length; i++) {
        dynamic_array_destroy((base + i));
    }
    arr->length = 0;
}

void dynamic_array_recursive_destroy(generic_dynamic_array* arr)
{
    dynamic_array_destroy_top_layer(arr);
    dynamic_array_destroy((generic_dynamic_array*)arr);
}

void dynamic_array_destroy(generic_dynamic_array* arr)
{
    free(arr->data);
    dynamic_array_unbind(arr);
}

void dynamic_array_unbind(generic_dynamic_array* arr)
{
    arr->data = NULL;
    arr->capacity = arr->length = 0;
}
