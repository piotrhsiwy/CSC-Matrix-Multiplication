#include "mul_naive.h"

__attribute__((hot)) void matr_mult_csc_V5(const void* const a, const void* const b, void* const restrict c)
{
    const matrix_data* const matrix_A = (const matrix_data* const)a;
    const matrix_data* const matrix_B = (const matrix_data* const)b;
    matrix_data* const matrix_C = (matrix_data* const)c;

    float** expanded_A = malloc((matrix_A->no_cols) * sizeof(*expanded_A));
    if (!expanded_A) {
        fprintf(stderr, "Cannot allocate memory for field expanded_A in function %s\n", __func__);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    for (size_t col = 0; col < matrix_A->no_cols; col++) {
        float* temp = malloc((matrix_A->no_rows) * sizeof(*temp));
        if (!temp) {
            fprintf(stderr, "Cannot allocate memory for field expanded_A[%lu] in function %s\n", col, __func__);
            nested_free(col, expanded_A);
            matrix_C->mul_error = OUT_OF_MEMORY;
            return;
        }
        expanded_A[col] = temp;
        memset(temp, 0, (matrix_A->no_rows) * sizeof(*temp));
    }

    float** expanded_B = malloc((matrix_B->no_cols * matrix_B->no_rows) * sizeof(*expanded_B));
    if (!expanded_B) {
        fprintf(stderr, "Cannot allocate memory for field expanded_B in function %s\n", __func__);
        nested_free(matrix_A->no_cols, expanded_A);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    for (size_t col = 0; col < matrix_B->no_cols; col++) {
        float* temp = malloc((matrix_B->no_rows) * sizeof(*temp));
        if (!temp) {
            fprintf(stderr, "Cannot allocate memory for field expanded_B[%lu] in function %s\n", col, __func__);
            nested_free(matrix_A->no_cols, expanded_A);
            nested_free(col, expanded_B);
            matrix_C->mul_error = OUT_OF_MEMORY;
            return;
        }
        expanded_B[col] = temp;
        memset(temp, 0, (matrix_B->no_rows) * sizeof(*temp));
    }

    float** expanded_C = malloc((matrix_C->no_cols * matrix_C->no_rows) * sizeof(*expanded_C)); // TODO: Shouldn't this just be matrix_C->no_cols * sizeof? (I just fixed a memory-error here that was caused by wrong types but this still does not make sense.)
    if (!expanded_C) {
        fprintf(stderr, "Cannot allocate memory for field expanded_C in function %s\n", __func__);
        nested_free(matrix_A->no_cols, expanded_A);
        nested_free(matrix_B->no_cols, expanded_B);
        matrix_C->mul_error = OUT_OF_MEMORY;
        return;
    }
    for (size_t col = 0; col < matrix_C->no_cols; col++) {
        float* temp = malloc((matrix_C->no_rows) * sizeof(*temp));
        if (!temp) {
            fprintf(stderr, "Cannot allocate memory for field expanded_C[%lu] in function %s\n", col, __func__);
            nested_free(matrix_A->no_cols, expanded_A);
            nested_free(matrix_B->no_cols, expanded_B);
            nested_free(col, expanded_C);
            matrix_C->mul_error = OUT_OF_MEMORY;
            return;
        }
        expanded_C[col] = temp;
        memset(temp, 0, (matrix_C->no_rows) * sizeof(*temp));
    }

    expand_matrix(a, expanded_A);
    expand_matrix(b, expanded_B);

    for (size_t col = 0; col < matrix_C->no_cols; col++) {
        for (size_t row = 0; row < matrix_C->no_rows; row++) {
            float acc = 0.0f;
            for (size_t k = 0; k < matrix_A->no_cols; k++) {
                acc += expanded_A[k][row] * expanded_B[col][k];
            }
            expanded_C[col][row] = acc;
        }
    }

    {
        error error = compress_matrix(expanded_C, matrix_C);
        if (error != OK) {
            nested_free(matrix_A->no_cols, expanded_A);
            nested_free(matrix_B->no_cols, expanded_B);
            nested_free(matrix_C->no_cols, expanded_C);
            matrix_C->mul_error = error;
            return;
        }
    }

    nested_free(matrix_A->no_cols, expanded_A);
    nested_free(matrix_B->no_cols, expanded_B);
    nested_free(matrix_C->no_cols, expanded_C);
    matrix_C->mul_error = OK;
}
