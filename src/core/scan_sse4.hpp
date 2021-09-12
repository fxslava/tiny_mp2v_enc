// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "common/cpu.hpp"
#include <stdint.h>
#include <emmintrin.h>

// PLANE C routines
void inverse_alt_scan_sse2(int16_t QF[64], int16_t QFS[64]) {
    __m128i v0 = _mm_loadu_si128((__m128i*) & QFS[0]);
    __m128i v1 = _mm_loadu_si128((__m128i*) & QFS[8]);
    __m128i v2 = _mm_loadu_si128((__m128i*) & QFS[16]);
    __m128i v3 = _mm_loadu_si128((__m128i*) & QFS[24]);
    __m128i v4 = _mm_loadu_si128((__m128i*) & QFS[32]);
    __m128i v5 = _mm_loadu_si128((__m128i*) & QFS[40]);
    __m128i v6 = _mm_loadu_si128((__m128i*) & QFS[48]);
    __m128i v7 = _mm_loadu_si128((__m128i*) & QFS[56]);

    __m128i out0 = _mm_unpacklo_epi64(v0, _mm_bsrli_si128(v1, 4));
    __m128i tmp0 = _mm_unpacklo_epi32(_mm_bsrli_si128(v0, 8), v1); // 04, 05, 10, 11, [06, 07, 12, 13]
    __m128i tmp1 = _mm_shufflelo_epi16(_mm_unpacklo_epi32(v2, _mm_bsrli_si128(v1, 12)), _MM_SHUFFLE(2, 3, 0, 1)); // 21, 20, 17, 16, [22, 23, xx, xx]
    __m128i out1 = _mm_unpacklo_epi64(tmp0, tmp1); // 04, 05, 10, 11, 21, 20, 17, 16

    __m128i tmp2 = _mm_shufflelo_epi16(_mm_unpackhi_epi32(tmp0, tmp1), _MM_SHUFFLE(2, 3, 1, 0)); // 06, 07, 23, 22, [12, 13, xx, xx]
    __m128i tmp3 = _mm_bsrli_si128(v3, 4); //32, 33, 34, 35, 36, 37, xx, xx
    __m128i out2 = _mm_unpacklo_epi64(tmp2, tmp3); // 06, 07, 23, 22, 32, 33, 34, 35

    __m128i tmp4 = _mm_unpacklo_epi32(_mm_bsrli_si128(v2, 8), v3); // 24, 25, 30, 31, [26, 27, 32, 33]
    __m128i tmp5 = _mm_unpacklo_epi32(_mm_bsrli_si128(v3, 12), v4); // 36, 37, 40, 41, [xx, xx, 42, 43]
    __m128i out3 = _mm_unpacklo_epi64(tmp4, tmp5); // 24, 25, 30, 31, 36, 37, 40, 41

    __m128i tmp6 = _mm_unpackhi_epi32(tmp4, _mm_bslli_si128(v4, 4)); // 26, 27, 42, 43, [32, 33, 44, 45]
    __m128i out4 = _mm_unpacklo_epi64(tmp6, _mm_bsrli_si128(v5, 4)); // 26, 27, 42, 43, 52, 53, 54, 55

    __m128i tmp7 = _mm_unpackhi_epi32(v4, _mm_bslli_si128(v5, 8)); // 44, 45, 50, 51, [46, 47, 52, 53]
    __m128i tmp8 = _mm_unpacklo_epi32(_mm_bsrli_si128(v5, 12), v6); // 56, 57, 60, 61, [xx, xx, 62, 63]
    __m128i out5 = _mm_unpacklo_epi64(tmp7, tmp8);

    __m128i tmp9 = _mm_unpackhi_epi32(v4, tmp8); // 44, 45, xx, xx, 46, 47, 62, 63
    __m128i out6 = _mm_unpackhi_epi64(tmp9, _mm_bslli_si128(v7, 8)); //46, 47, 62, 63, 70, 71, 72, 73

    __m128i out7 = _mm_unpackhi_epi64(v6, v7); //64, 65, 66, 67, 74, 75, 76, 77

    _mm_store_si128((__m128i*) & QF[0 ], out0);
    _mm_store_si128((__m128i*) & QF[8 ], out1);
    _mm_store_si128((__m128i*) & QF[16], out2);
    _mm_store_si128((__m128i*) & QF[24], out3);
    _mm_store_si128((__m128i*) & QF[32], out4);
    _mm_store_si128((__m128i*) & QF[40], out5);
    _mm_store_si128((__m128i*) & QF[48], out6);
    _mm_store_si128((__m128i*) & QF[56], out7);
}