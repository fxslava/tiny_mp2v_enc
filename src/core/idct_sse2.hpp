// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include <emmintrin.h>
#include "common/cpu.hpp"
#include "idct.h"

MP2V_INLINE __m128i _mm_tmp_op0_epi16(__m128i src) { // src / 0.707106781186547524400844 : S1[0], S[2]
    return _mm_adds_epi16(src, _mm_mulhi_epi16(src, _mm_set1_epi16(27145)));
}

MP2V_INLINE __m128i _mm_tmp_op1_epi16(__m128i src) { // src * 0.541196100146196984399723 : S1[1]
    return _mm_subs_epi16(src, _mm_mulhi_epi16(src, _mm_set1_epi16(30068)));
}

MP2V_INLINE __m128i _mm_tmp_op3_epi16(__m128i src) { // src * 1.306562964876376527856643 : S1[3]
    return _mm_adds_epi16(src, _mm_mulhi_epi16(src, _mm_set1_epi16(20090)));
}

MP2V_INLINE __m128i _mm_tmp_op4_epi16(__m128i src) { // src * 0.382683432365089771728460 : S1[4]
    return _mm_mulhi_epi16(src, _mm_set1_epi16(25079));
}

MP2V_INLINE void idct_1d_sse2(__m128i (&src)[8]) {
    // step 0
    const __m128i v15 = _mm_adds_epi16(_mm_slli_epi16(_mm_mulhi_epi16(src[0], _mm_set1_epi16(27145)), 1), _mm_slli_epi16(src[0], 1));
    const __m128i v26 = _mm_adds_epi16(_mm_mulhi_epi16(src[1], _mm_set1_epi16(-5037)), _mm_slli_epi16(src[1], 2));
    const __m128i v21 = _mm_adds_epi16(_mm_mulhi_epi16(src[2], _mm_set1_epi16(-19954)), _mm_slli_epi16(src[2], 2));
    const __m128i v28 = _mm_adds_epi16(_mm_slli_epi16(_mm_mulhi_epi16(src[3], _mm_set1_epi16(-22089)), 1), _mm_slli_epi16(src[3], 2));
    const __m128i v16 = _mm_adds_epi16(_mm_slli_epi16(_mm_mulhi_epi16(src[4], _mm_set1_epi16(27145)), 1), _mm_slli_epi16(src[4], 1));
    const __m128i v25 = _mm_adds_epi16(_mm_mulhi_epi16(src[5], _mm_set1_epi16(14567)), _mm_slli_epi16(src[5], 1));
    const __m128i v22 = _mm_adds_epi16(_mm_slli_epi16(_mm_mulhi_epi16(src[6], _mm_set1_epi16(17391)), 1), src[6]);
    const __m128i v27 = _mm_slli_epi16(_mm_mulhi_epi16(src[7], _mm_set1_epi16(25570)), 1);
    // step 1
    const __m128i v19 = _mm_subs_epi16(v25, v28); // /2
    const __m128i v20 = _mm_subs_epi16(v26, v27); // /2
    const __m128i v23 = _mm_adds_epi16(v26, v27); // /2
    const __m128i v24 = _mm_adds_epi16(v25, v28); // /2
    const __m128i v7  = _mm_adds_epi16(v23, v24); // /4
    const __m128i v11 = _mm_adds_epi16(v21, v22); // /2
    const __m128i v13 = _mm_subs_epi16(v23, v24); // /4
    const __m128i v17 = _mm_subs_epi16(v21, v22); // /2
    const __m128i v8  = _mm_adds_epi16(v15, v16); // /2
    const __m128i v9  = _mm_subs_epi16(v15, v16); // /2
    // step 2
    const __m128i v18 = _mm_tmp_op4_epi16(_mm_subs_epi16(v19, v20));   //(v19 - v20) * s1[4]; /2
    const __m128i v12 = _mm_subs_epi16(v18, _mm_tmp_op3_epi16(v19));   // v18 - v19 * s1[3];  /2
    const __m128i v14 = _mm_subs_epi16(_mm_tmp_op1_epi16(v20), v18);   // v20 * s1[1] - v18); /2
    const __m128i v6  = _mm_subs_epi16(_mm_slli_epi16(v14, 1), v7);    // v14 - v7            /4
    const __m128i v5  = _mm_subs_epi16(_mm_tmp_op0_epi16(v13), v6);    // v13 / s1[2] - v6;   /4
    const __m128i v4  = _mm_adds_epi16(v5, _mm_slli_epi16(v12, 1));    // v5 + v12;           /4
    const __m128i v10 = _mm_subs_epi16(_mm_tmp_op0_epi16(v17), v11);   // v17 / s1[0] - v11;  /2
    const __m128i v0  = _mm_adds_epi16(v8, v11); // /4
    const __m128i v1  = _mm_adds_epi16(v9, v10); // /4
    const __m128i v2  = _mm_subs_epi16(v9, v10); // /4
    const __m128i v3  = _mm_subs_epi16(v8, v11); // /4
    // step 3
    src[0] = _mm_adds_epi16(v0, v7); // /8
    src[1] = _mm_adds_epi16(v1, v6); // /8
    src[2] = _mm_adds_epi16(v2, v5); // /8
    src[3] = _mm_subs_epi16(v3, v4); // /8
    src[4] = _mm_adds_epi16(v3, v4); // /8
    src[5] = _mm_subs_epi16(v2, v5); // /8
    src[6] = _mm_subs_epi16(v1, v6); // /8
    src[7] = _mm_subs_epi16(v0, v7); // /8
}

MP2V_INLINE void transpose_8x8_sse2(__m128i (&src)[8]) {
    __m128i a03b03 = _mm_unpacklo_epi16(src[0], src[1]);
    __m128i c03d03 = _mm_unpacklo_epi16(src[2], src[3]);
    __m128i e03f03 = _mm_unpacklo_epi16(src[4], src[5]);
    __m128i g03h03 = _mm_unpacklo_epi16(src[6], src[7]);
    __m128i a47b47 = _mm_unpackhi_epi16(src[0], src[1]);
    __m128i c47d47 = _mm_unpackhi_epi16(src[2], src[3]);
    __m128i e47f47 = _mm_unpackhi_epi16(src[4], src[5]);
    __m128i g47h47 = _mm_unpackhi_epi16(src[6], src[7]);

    __m128i a01b01c01d01 = _mm_unpacklo_epi32(a03b03, c03d03);
    __m128i a23b23c23d23 = _mm_unpackhi_epi32(a03b03, c03d03);
    __m128i e01f01g01h01 = _mm_unpacklo_epi32(e03f03, g03h03);
    __m128i e23f23g23h23 = _mm_unpackhi_epi32(e03f03, g03h03);
    __m128i a45b45c45d45 = _mm_unpacklo_epi32(a47b47, c47d47);
    __m128i a67b67c67d67 = _mm_unpackhi_epi32(a47b47, c47d47);
    __m128i e45f45g45h45 = _mm_unpacklo_epi32(e47f47, g47h47);
    __m128i e67f67g67h67 = _mm_unpackhi_epi32(e47f47, g47h47);

    src[0] = _mm_unpacklo_epi64(a01b01c01d01, e01f01g01h01);
    src[1] = _mm_unpackhi_epi64(a01b01c01d01, e01f01g01h01);
    src[2] = _mm_unpacklo_epi64(a23b23c23d23, e23f23g23h23);
    src[3] = _mm_unpackhi_epi64(a23b23c23d23, e23f23g23h23);
    src[4] = _mm_unpacklo_epi64(a45b45c45d45, e45f45g45h45);
    src[5] = _mm_unpackhi_epi64(a45b45c45d45, e45f45g45h45);
    src[6] = _mm_unpacklo_epi64(a67b67c67d67, e67f67g67h67);
    src[7] = _mm_unpackhi_epi64(a67b67c67d67, e67f67g67h67);
}

template<bool add>
MP2V_INLINE void inverse_dct_template_sse2(uint8_t* plane, int16_t F[64], int stride) {
    __m128i buffer[8];
    for (int i = 0; i < 8; i++)
        buffer[i] = _mm_load_si128((__m128i*) & F[i*8]);

    idct_1d_sse2(buffer);
    transpose_8x8_sse2(buffer);
    idct_1d_sse2(buffer);
    transpose_8x8_sse2(buffer);

    for (int i = 0; i < 4; i++) {
        __m128i tmp, b0, b1;
        b0 = _mm_srai_epi16(buffer[i * 2], 6);
        b1 = _mm_srai_epi16(buffer[i * 2 + 1], 6);
        if (add) {
            __m128i dstl = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) & plane[(i * 2 + 0) * stride]), _mm_setzero_si128());
            __m128i dsth = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) & plane[(i * 2 + 1) * stride]), _mm_setzero_si128());
            tmp = _mm_packus_epi16(_mm_adds_epi16(dstl, b0), _mm_adds_epi16(dsth, b1));
        } else
            tmp = _mm_packus_epi16(b0, b1);

        _mm_storel_epi64((__m128i*) & plane[(i * 2 + 0) * stride], tmp);
        _mm_storel_epi64((__m128i*) & plane[(i * 2 + 1) * stride], _mm_srli_si128(tmp, 8));
    }
}

void inverse_dct_sse2(uint8_t* plane, int16_t F[64], int stride) {
    inverse_dct_template_sse2<false>(plane, F, stride);
}
void add_inverse_dct_sse2(uint8_t* plane, int16_t F[64], int stride) {
    inverse_dct_template_sse2<true>(plane, F, stride);
}