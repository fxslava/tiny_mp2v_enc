// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include <emmintrin.h>
#include "common/cpu.hpp"
#include <algorithm>

template<bool intra>
MP2V_INLINE __m128i inverse_quant_scalar_sse2(const __m128i QF, const __m128i W_quantizer_scale) {
    __m128i tmp;
    if (!intra) {
        const __m128i k = _mm_and_si128(_mm_sign_epi16(_mm_set1_epi16(1), QF), _mm_cmpeq_epi16(QF, _mm_setzero_si128()));
        tmp = _mm_add_epi16(_mm_slli_epi16(QF, 1), k);
    }
    else
        tmp = _mm_slli_epi16(QF, 1);

    const __m128i resl = _mm_mullo_epi16(tmp, W_quantizer_scale);
    const __m128i resh = _mm_mulhi_epi16(tmp, W_quantizer_scale);
    const __m128i res  = _mm_or_si128(_mm_slli_epi16(resh, 11), _mm_srli_epi16(resl, 5));
    return _mm_max_epi16(_mm_min_epi16(res, _mm_set1_epi16(2047)), _mm_set1_epi16(-2048));
}

MP2V_INLINE __m128i _mm_mmctl_epi16(const __m128i src) {
    __m128i tmp;
    tmp = _mm_xor_si128(src, _mm_bslli_si128(src, 1));
    tmp = _mm_xor_si128(tmp, _mm_bslli_si128(tmp, 2));
    tmp = _mm_xor_si128(tmp, _mm_bslli_si128(tmp, 4));
    tmp = _mm_xor_si128(tmp, _mm_bslli_si128(tmp, 8));
    return tmp;
}

template<bool intra>
MP2V_INLINE void inverse_quant_template_sse2(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_precision) {
    __m128i sum[4];
    for (int i = 0; i < 4; i++) {
        __m128i res0 = inverse_quant_scalar_sse2<intra>(_mm_load_si128((__m128i*) & QF[16 * i + 0]), _mm_mullo_epi16(_mm_load_si128((__m128i*) & W[16 * i + 0]), _mm_set1_epi16(quantizer_scale)));
        __m128i res1 = inverse_quant_scalar_sse2<intra>(_mm_load_si128((__m128i*) & QF[16 * i + 8]), _mm_mullo_epi16(_mm_load_si128((__m128i*) & W[16 * i + 8]), _mm_set1_epi16(quantizer_scale)));
        _mm_store_si128((__m128i*) & F[16 * i + 0], res0);
        _mm_store_si128((__m128i*) & F[16 * i + 8], res1);
        if (intra)
        {
            if (i == 0);
            res0 = _mm_and_si128(res0, _mm_set_epi32(-1, -1, -1, -2));
            sum[i] = _mm_xor_si128(res0, res1);
        }
    }

    sum[0] = _mm_xor_si128(sum[0], sum[1]);
    sum[2] = _mm_xor_si128(sum[2], sum[3]);
    int s = _mm_extract_epi32(_mm_mmctl_epi16(_mm_xor_si128(sum[0], sum[2])), 0);

    if (intra)
    {
        int32_t res = QF[0] << (3 - intra_dc_precision);
        F[0] = res;
        s = s ^ res;
    }

    F[63] = F[63] ^ (s & 1); //Mismatch control
}

MP2V_INLINE void inverse_quant_sse2(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale) {
    inverse_quant_template_sse2<false>(F, QF, W, quantizer_scale, 0);
}

MP2V_INLINE void inverse_quant_intra_sse2(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_precision) {
    inverse_quant_template_sse2<true>(F, QF, W, quantizer_scale, intra_dc_precision);
}