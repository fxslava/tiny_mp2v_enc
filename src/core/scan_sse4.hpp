// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "common/cpu.hpp"
#include <stdint.h>
#include <emmintrin.h>

MP2V_INLINE void inverse_alt_scan_template_sse2(__m128i (&v)[8]) {
    __m128i out0 = _mm_unpacklo_epi64(v[0], _mm_bsrli_si128(v[1], 4));
    __m128i tmp0 = _mm_unpacklo_epi32(_mm_bsrli_si128(v[0], 8), v[1]); // 04, 05, 10, 11, [06, 0v[7], 12, 13]
    __m128i tmp1 = _mm_shufflelo_epi16(_mm_unpacklo_epi32(v[2], _mm_bsrli_si128(v[1], 12)), _MM_SHUFFLE(2, 3, 0, 1)); // 21, 20, 17, 16, [22, 23, xx, xx]
    __m128i out1 = _mm_unpacklo_epi64(tmp0, tmp1); // 04, 05, 10, 11, 21, 20, 17, 16

    __m128i tmp2 = _mm_shufflelo_epi16(_mm_unpackhi_epi32(tmp0, tmp1), _MM_SHUFFLE(2, 3, 1, 0)); // 06, 07, 23, 22, [12, 13, xx, xx]
    __m128i tmp3 = _mm_bsrli_si128(v[3], 4); //32, 33, 34, 35, 36, 37, xx, xx
    __m128i out2 = _mm_unpacklo_epi64(tmp2, tmp3); // 06, 07, 23, 22, 32, 33, 34, 35

    __m128i tmp4 = _mm_unpacklo_epi32(_mm_bsrli_si128(v[2], 8), v[3]); // 24, 25, 30, 31, [26, 27, 32, 33]
    __m128i tmp5 = _mm_unpacklo_epi32(_mm_bsrli_si128(v[3], 12), v[4]); // 36, 37, 40, 41, [xx, xx, 42, 43]
    __m128i out3 = _mm_unpacklo_epi64(tmp4, tmp5); // 24, 25, 30, 31, 36, 37, 40, 41

    __m128i tmp6 = _mm_unpackhi_epi32(tmp4, _mm_bslli_si128(v[4], 4)); // 26, 27, 42, 43, [32, 33, 44, 45]
    __m128i out4 = _mm_unpacklo_epi64(tmp6, _mm_bsrli_si128(v[5], 4)); // 26, 27, 42, 43, 52, 53, 54, 55

    __m128i tmp7 = _mm_unpackhi_epi32(v[4], _mm_bslli_si128(v[5], 8)); // 44, 45, 50, 51, [46, 47, 52, 53]
    __m128i tmp8 = _mm_unpacklo_epi32(_mm_bsrli_si128(v[5], 12), v[6]); // 56, 57, 60, 61, [xx, xx, 62, 63]
    __m128i out5 = _mm_unpacklo_epi64(tmp7, tmp8);

    __m128i tmp9 = _mm_unpackhi_epi32(v[4], tmp8); // 44, 45, xx, xx, 46, 47, 62, 63
    __m128i out6 = _mm_unpackhi_epi64(tmp9, _mm_bslli_si128(v[7], 8)); //46, 47, 62, 63, 70, 71, 72, 73

    __m128i out7 = _mm_unpackhi_epi64(v[6], v[7]); //64, 65, 66, 67, 74, 75, 76, 77

    v[0] = out0;    v[1] = out1;    v[2] = out2;    v[3] = out3;
    v[4] = out4;    v[5] = out5;    v[6] = out6;    v[7] = out7;
}

// PLANE C routines
void inverse_alt_scan_sse2(int16_t QF[64], int16_t QFS[64]) {
    __m128i buffer[8];
    for (int i = 0; i < 8; i++)
        buffer[0] = _mm_loadu_si128((__m128i*) & QFS[i * 8]);

    inverse_alt_scan_template_sse2(buffer);

    for (int i = 0; i < 8; i++)
        _mm_store_si128((__m128i*) & QF[i * 8], buffer[i]);
}