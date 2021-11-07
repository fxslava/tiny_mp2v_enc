// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include <algorithm>
#include <emmintrin.h>
#include "common/cpu.hpp"
#include "quant_scan_c.hpp"

MP2V_INLINE __m128i _mm_tmp_op1_epi16(__m128i src) {
    const __m128i tmp = _mm_mulhi_epi16(src, _mm_set1_epi16(9598));
    return _mm_subs_epi16(src, _mm_adds_epi16(tmp, tmp));
}

MP2V_INLINE __m128i _mm_tmp_op2_epi16(__m128i src) {
    const __m128i tmp = _mm_mulhi_epi16(src, _mm_set1_epi16(15034));
    return _mm_subs_epi16(src, _mm_adds_epi16(tmp, tmp));
}

MP2V_INLINE __m128i _mm_tmp_op4_epi16(__m128i src) {
    const __m128i tmp = _mm_mulhi_epi16(src, _mm_set1_epi16(10045));
    return _mm_adds_epi16(src, _mm_adds_epi16(tmp, tmp));
}

MP2V_INLINE __m128i _mm_tmp_op5_epi16(__m128i src) {
    const __m128i tmp = _mm_mulhi_epi16(src, _mm_set1_epi16(12540));
    return _mm_adds_epi16(tmp, tmp);
}

MP2V_INLINE __m128i _mm_x2_epi16(__m128i tmp) {
    return _mm_adds_epi16(tmp, tmp);
}

MP2V_INLINE void fdct_1d_sse2(__m128i (&src)[8]) {
    const __m128i v0 = _mm_adds_epi16(src[0], src[7]);
    const __m128i v1 = _mm_adds_epi16(src[1], src[6]);
    const __m128i v2 = _mm_adds_epi16(src[2], src[5]);
    const __m128i v3 = _mm_adds_epi16(src[3], src[4]);
    const __m128i v4 = _mm_subs_epi16(src[3], src[4]);
    const __m128i v5 = _mm_subs_epi16(src[2], src[5]);
    const __m128i v6 = _mm_subs_epi16(src[1], src[6]);
    const __m128i v7 = _mm_subs_epi16(src[0], src[7]);

    const __m128i v8 = _mm_adds_epi16(v0, v3);
    const __m128i v9 = _mm_adds_epi16(v1, v2);
    const __m128i v10 = _mm_subs_epi16(v1, v2);
    const __m128i v11 = _mm_subs_epi16(v0, v3);
    const __m128i v12 = _mm_adds_epi16(v4, v5);
    const __m128i v13 = _mm_tmp_op1_epi16(_mm_adds_epi16(v5, v6));
    const __m128i v14 = _mm_adds_epi16(v6, v7);
    const __m128i v15 = _mm_adds_epi16(v8, v9);
    const __m128i v16 = _mm_subs_epi16(v8, v9);
    const __m128i v17 = _mm_tmp_op1_epi16(_mm_adds_epi16(v10, v11));
    const __m128i v18 = _mm_tmp_op5_epi16(_mm_subs_epi16(v14, v12));
    const __m128i v19 = _mm_subs_epi16(_mm_tmp_op2_epi16(v12), v18);
    const __m128i v20 = _mm_subs_epi16(_mm_tmp_op4_epi16(v14), v18);
    const __m128i v21 = _mm_adds_epi16(v17, v11);
    const __m128i v22 = _mm_subs_epi16(v11, v17);
    const __m128i v23 = _mm_adds_epi16(v13, v7);
    const __m128i v24 = _mm_subs_epi16(v7, v13);
    const __m128i v25 = _mm_adds_epi16(v19, v24);
    const __m128i v26 = _mm_adds_epi16(v23, v20);
    const __m128i v27 = _mm_subs_epi16(v23, v20);
    const __m128i v28 = _mm_subs_epi16(v24, v19);

    src[0] = _mm_x2_epi16(_mm_mulhi_epi16(v15, _mm_set1_epi16(11585)));
    src[1] = _mm_x2_epi16(_mm_mulhi_epi16(v26, _mm_set1_epi16(8352)));
    src[2] = _mm_x2_epi16(_mm_mulhi_epi16(v21, _mm_set1_epi16(8867)));
    src[3] = _mm_x2_epi16(_mm_mulhi_epi16(v28, _mm_set1_epi16(9852)));
    src[4] = _mm_x2_epi16(_mm_mulhi_epi16(v16, _mm_set1_epi16(11585)));
    src[5] = _mm_x2_epi16(_mm_mulhi_epi16(v25, _mm_set1_epi16(14745)));
    src[6] = _mm_x2_epi16(_mm_subs_epi16(v22, _mm_mulhi_epi16(v22, _mm_set1_epi16(11361))));
    src[7] = _mm_x2_epi16(_mm_adds_epi16(v27, _mm_mulhi_epi16(v27, _mm_set1_epi16(9223))));
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

template<bool alt_scan, bool intra = true>
uint64_t forward_dct_scan_quant_template(int16_t(&F)[65], uint8_t* src, uint8_t W[64], int stride, int quantizer_scale, int dc_prec) {
    ALIGN(16) int16_t tmp[64];
    __m128i buffer[8];
    for (int i = 0; i < 8; i++)
        buffer[i] = _mm_cvtepu8_epi16(_mm_loadl_epi64((__m128i*) & src[i * stride]));

    fdct_1d_sse2(buffer);
    transpose_8x8_sse2(buffer);
    fdct_1d_sse2(buffer);
    //transpose_8x8_sse2(buffer);

    for (int i = 0; i < 8; i++)
        _mm_store_si128((__m128i*) & tmp[i * 8], buffer[i]);

    //transpose & quantization
    return transpose_quant_scan<alt_scan, intra>(F, tmp, W, stride, quantizer_scale, dc_prec);
}