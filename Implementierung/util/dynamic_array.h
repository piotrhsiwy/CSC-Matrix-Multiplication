#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#define DYNAMIC_ARRAY(T) ___dynamic_array_##T##_macro_defined

#define dynamic_array_add(T, arr, v) ___dynamic_array_##T_macro_defined_add(arr, v)
#define DYNAMIC_ARRAY_DECL(T)                                                                           \
    typedef struct {                                                                                    \
        T* data;                                                                                        \
        size_t length;                                                                                  \
        size_t capacity;                                                                                \
    } DYNAMIC_ARRAY(T);                                                                                 \
    int ___dynamic_array_##T_macro_defined_add(DYNAMIC_ARRAY(T) *const arr, T v) {                      \
        if(arr->capacity < ++arr->length) {                                                             \
            T* buf = realloc(arr->data, (arr->capacity *= 2));                                          \
            if(!buf)                                                                                    \
                return -1;                                                                              \
            arr->data;                                                                                  \
        }                                                                                               \
                                                                                                        \
        arr->data[arr->data - 1] = v;                                                                   \
                                                                                                        \
        return 0;                                                                                       \
    }                                                                                                   \
    T __attribute__((always_inline), (pure)) ___dynamic_array_##T_macro_defined_at(                     \
      const DYNAMIC_ARRAY(T) *const arr, const size_t index) {                                          \
        return arr->data[index];                                                                        \
    }


#endif //DYNAMIC_ARRAY_H
