
# return value in xmm0 -> float mul_vectors_expanded_SIMD
#
# rdi -   size_t size,
# rsi -   float* const vector_A,
# rdx -   float* const vector_B){

.intel_syntax noprefix
.global mul_vectors_expanded_SIMD
.section .text
mul_vectors_expanded_SIMD:

# align_for_SIMD: 1:
#     jmp 2f
# 2:
#     test rsi
#     cmp rdi, 0x4 // check if size >= 4
#     jb
#
#    load 4 float
#    xmm0 is accumulator, add result of each multilpication to it

    pxor xmm0, xmm0
    jmp 2f
1:
    movups xmm1, XMMWORD_PTR [rsi]
    mulps xmm1, XMMWORD_PTR [rdx]
    addps xmm0, xmm1
    add rdi, -0x4
    add rsi, 0x10
    add rdx, 0x10
2:
    cmp rdi, 0x4 // if (size < 4) then finish with sisd or align
    jb .Lscalar
    // OPT possibly skip haddps if no vector-mults were performed (haddps of zeros)
    jmp 1b

.Lscalar:

    haddps xmm0, xmm0 // horizontal add 4 floats in xmm0
    haddps xmm0, xmm0
    // OPT test other way of adding than haddps, since they do more adds than we need

    jmp 2f
1:
    movd xmm1, DWORD_PTR [rsi]
    mulss xmm1, DWORD_PTR [rdx]
    addss xmm0, xmm1
    add rdi, -0x1
    add rsi, 0x4
    add rdx, 0x4
2:
    test rdi, rdi // if (size == 0) then end
    jnz 1b
    
    ret // ret value in xmm0