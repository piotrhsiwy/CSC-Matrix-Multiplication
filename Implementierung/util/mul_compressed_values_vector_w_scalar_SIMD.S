
#**
#* multiply in place each value in the vector
#(pair.value) with scalar.
#* /

#rdi - DYNAMIC_ARRAY(float) * vector
#xmm0 - const float scalar

#* vector layout:
#offset size(B) type name
# 0x0 8B float * data
# 0x08 8B size_t length
# 0x10 8B size_t capacity
# 0x18 8B - padding

#* vector->data layout:
#| -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -
#| offset size(B) type name
#| 0x00 4B float value0
#| 0x04 4B float value1
#| 0x08 4B float value2
#|... vector->length times...

.intel_syntax noprefix
    .global mul_compressed_values_vector_w_scalar_SIMD
    .section.text

        mul_compressed_values_vector_w_scalar_SIMD : mov r8,
    QWORD PTR[rdi] // r8   := & cur_tuple
    mov r9,
    QWORD PTR[rdi + 0x08] lea r10, [r8 + r9 * 0x04] // r10  := & 1 float after last_tuple         vector->data[length]
    lea r9,
    [r10 - 0xc] // r9   := & 1 float after last_simd_tuple    vector->data[length-3]

    pshufd xmm0,
    xmm0, 0x00 // xmm0 := s, s, s, s

    jmp 2f 1 : movups xmm1,
               XMMWORD PTR[r8] mulps xmm1,
               xmm0
                   movups XMMWORD PTR[r8],
               xmm1
                   add r8,
               0x10 2 : cmp r8,
                        r9
                            jb 1b

                        jmp 2f 1 : movd xmm1,
                                   DWORD PTR[r8] mulss xmm1,
                                   xmm0
                                       movd DWORD PTR[r8],
                                   xmm1
                                       add r8,
                                   0x04 2 : cmp r8,
                                            r10
                                                jb 1b

                                            ret
