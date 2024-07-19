
#**
#* multiply in place each value in the vector
# (pair.value) with scalar.
#*/

# rdi - DYNAMIC_ARRAY(pair)* vector
# xmm0 - const float scalar

# *vector layout:
# offset    size(B) type    name
# 0x0       8B      void*   data
# 0x08      8B      size_t  length
# 0x10      8B      size_t  capacity
# 0x18      8B      -       padding

# *vector->data layout:
# |---------------------------------
# |offset    size(B) type    name
# |0x0       8B      size_t  index
# |0x8       4B      float   value <-----------
# |0xc       4B      -       padding
# |---------------------------------
# |0x0       8B      size_t  index
# |0x8       4B      float   value <-----------
# |0xc       4B      -       padding
# |---------------------------------
# | ... size 0x10, vector->length times ...

.intel_syntax noprefix
.global mul_compressed_values_vector_w_scalar_SIMD
.section .text

mul_compressed_values_vector_w_scalar_SIMD:
    lea r8, [rdi + 0x08]
    mov rcx, QWORD PTR [r8]         // rdx  := vector->length, counter

    pshufd xmm0, xmm0, 0x00         // xmm0 := s, s, s, s
    mov r8, QWORD PTR [rdi]         // r8   := &(cur_pair.value)
    lea r9, [0x8*rcx]
    lea r8, [-0x08 + r8 + 0x02*r9]

    jmp 2f
1:

    movd xmm1, DWORD PTR [r8]
    lea r9, [r8 - 0x10]
    movd xmm2, DWORD PTR [r9]
    lea r10, [r9 - 0x10]
    movd xmm3, DWORD PTR [r10]
    lea r11, [r10 - 0x10]
    movd xmm4, DWORD PTR [r11]
    shufps xmm1, xmm2, 0x44 // == 0b 01 00 01 00
    # now xmm1 ==   0,  f2,  0,  f1  
    #               127-----------0
    shufps xmm3, xmm4, 0x44
    # now xmm3 ==   0,  f4,  0,  f3  
    #               127-----------0
    shufps xmm1, xmm3, 0x88 // == 0b 10 00 10 00
    # now xmm1 ==   f4, f3, f2, f1  
    #               127-----------0
    mulps xmm1, xmm0
    # now xmm1 ==   sf4,sf3,sf2,sf1  
    #               127-----------0
    movd DWORD PTR[r8], xmm1
    pshufd xmm1, xmm1, 0x39 // 0b 00 11 10 01 aka xmm1 >> 32
    movd DWORD PTR[r9], xmm1
    pshufd xmm1, xmm1, 0x39 // 0b 00 11 10 01
    movd DWORD PTR[r10], xmm1
    pshufd xmm1, xmm1, 0x39 // 0b 00 11 10 01
    movd DWORD PTR[r11], xmm1

    add rcx, -0x4
    mov r8, r11
    // OPT maybe faster to only keep track of rcx and recalculate address every time?

2:
    cmp rcx, 4 // we can get -1 here!
    jb .Lscalar
    jmp 1b

.Lscalar:
    jmp 2f
1:
    movd xmm1, DWORD PTR [r8]
    // OPT maybe faster to copy xmm0 and mulss straight from mem32?
    mulss xmm1, xmm0
    movd DWORD PTR [r8], xmm1
    dec rcx
    add r8, -0x10
2:
    test rcx, rcx
    jnz 1b

.Lend:
ret
