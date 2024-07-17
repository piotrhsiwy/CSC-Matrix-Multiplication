
; return value in xmm0 -> float mul_vectors_expanded_SIMD

; rdi -   size_t size,
; rsi -   float* const vector_A,
; rdx -   float* const vector_B){

.intel_syntax noprefix
.global mul_vectors_expanded_SIMD
.section .text
mul_vectors_expanded_SIMD:
    mov 