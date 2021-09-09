// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include <emmintrin.h>
#include "common/cpu.hpp"
#include "idct.h"

constexpr __m128i S0x32_M128[8] = {
    { 0xB504B504B504B504, 0xB504B504B504B504 }, // = 16384.0 / 0.353553390593273762200422 = 46340
    { 0xFB14FB14FB14FB14, 0xFB14FB14FB14FB14 }, // = 16384.0 / 0.254897789552079584470970 = 64276
    { 0xEC83EC83EC83EC83, 0xEC83EC83EC83EC83 }, // = 16384.0 / 0.270598050073098492199862 = 60547
    { 0xD4DBD4DBD4DBD4DB, 0xD4DBD4DBD4DBD4DB }, // = 16384.0 / 0.300672443467522640271861 = 54491
    { 0xB504B504B504B504, 0xB504B504B504B504 }, // = 16384.0 / 0.353553390593273762200422 = 46340
    { 0x8E398E398E398E39, 0x8E398E398E398E39 }, // = 16384.0 / 0.449988111568207852319255 = 36409
    { 0x61F761F761F761F7, 0x61F761F761F761F7 }, // = 16384.0 / 0.653281482438188263928322 = 25079
    { 0x31F131F131F131F1, 0x31F131F131F131F1 }  // = 16384.0 / 1.281457723870753089398043 = 12785
};

constexpr __m128i S1x128_M128[5] = {
    { 0xB504B504B504B504, 0xB504B504B504B504 }, // = 32768.0 / 0.707106781186547524400844 = 46340
    { 0x4545454545454545, 0x4545454545454545 }, // = 32768.0 * 0.541196100146196984399723 = 17733
    { 0xB504B504B504B504, 0xB504B504B504B504 }, // = 32768.0 / 0.707106781186547524400844 = 46340
    { 0xA73DA73DA73DA73D, 0xA73DA73DA73DA73D }, // = 32768.0 * 1.306562964876376527856643 = 42813
    { 0x30FB30FB30FB30FB, 0x30FB30FB30FB30FB }  // = 32768.0 * 0.382683432365089771728460 = 12539
};

MP2V_INLINE void idct_1d_sse2(__m128i (&src)[8]) {
    // step 0
    const __m128i v15 = _mm_mulhi_epi16(src[0], _mm_load_si128(&S0x32_M128[0]));
    const __m128i v26 = _mm_mulhi_epi16(src[1], _mm_load_si128(&S0x32_M128[1]));
    const __m128i v21 = _mm_mulhi_epi16(src[2], _mm_load_si128(&S0x32_M128[2]));
    const __m128i v28 = _mm_mulhi_epi16(src[3], _mm_load_si128(&S0x32_M128[3]));
    const __m128i v16 = _mm_mulhi_epi16(src[4], _mm_load_si128(&S0x32_M128[4]));
    const __m128i v25 = _mm_mulhi_epi16(src[5], _mm_load_si128(&S0x32_M128[5]));
    const __m128i v22 = _mm_mulhi_epi16(src[6], _mm_load_si128(&S0x32_M128[6]));
    const __m128i v27 = _mm_mulhi_epi16(src[7], _mm_load_si128(&S0x32_M128[7]));
    // step 1
    const __m128i v19 = _mm_subs_epi16(v25, v28); // *2
    const __m128i v20 = _mm_subs_epi16(v26, v27); // *2
    const __m128i v23 = _mm_adds_epi16(v26, v27); // *2
    const __m128i v24 = _mm_adds_epi16(v25, v28); // *2
    const __m128i v7  = _mm_srli_epi16(_mm_adds_epi16(v23, v24), 1); // *2
    const __m128i v11 = _mm_adds_epi16(v21, v22); // *2
    const __m128i v13 = _mm_subs_epi16(v23, v24);
    const __m128i v17 = _mm_subs_epi16(v21, v22); // *2
    const __m128i v8  = _mm_srli_epi16(_mm_adds_epi16(v15, v16), 1); // *4
    const __m128i v9  = _mm_srli_epi16(_mm_subs_epi16(v15, v16), 1); // *4
    // step 2
    const __m128i v18 = _mm_mulhi_epi16(_mm_subs_epi16(v19, v20), S1x128_M128[4]);                         //(v19 - v20) * s1[4]; // *4
    const __m128i v12 = _mm_subs_epi16(v18, _mm_mulhi_epi16(v19, _mm_load_si128(&S1x128_M128[3])));        // v18 - v19 * s1[3];  // *4
    const __m128i v14 = _mm_subs_epi16(_mm_mulhi_epi16(v20, _mm_load_si128(&S1x128_M128[1])), v18);        // v20 * s1[1] - v18); // *4
    const __m128i v6  = _mm_subs_epi16(_mm_adds_epi16(v14, v14), v7);                                      // v14 - v7            // *2
    const __m128i v5  = _mm_subs_epi16(_mm_mulhi_epi16(v13, S1x128_M128[2]), v6);                          // v13 / s1[2] - v6;   // *2
    const __m128i v4  = _mm_subs_epi16(_mm_setzero_si128(), _mm_adds_epi16(v5, _mm_adds_epi16(v12, v12))); //-v5 - v12;           // *2
    const __m128i v10 = _mm_subs_epi16(_mm_mulhi_epi16(v17, S1x128_M128[0]), _mm_srli_epi16(v11, 1));      // v17 / s1[0] - v11;  // *4
    const __m128i v0  = _mm_adds_epi16(v8, v11); // *2
    const __m128i v1  = _mm_adds_epi16(v9, v10); // *2
    const __m128i v2  = _mm_subs_epi16(v9, v10); // *2
    const __m128i v3  = _mm_subs_epi16(v8, v11); // *2
    // step 3
    src[0] = _mm_adds_epi16(v0, v7);
    src[1] = _mm_adds_epi16(v1, v6);
    src[2] = _mm_adds_epi16(v2, v5);
    src[3] = _mm_adds_epi16(v3, v4);
    src[4] = _mm_subs_epi16(v3, v4);
    src[5] = _mm_subs_epi16(v2, v5);
    src[6] = _mm_subs_epi16(v1, v6);
    src[7] = _mm_subs_epi16(v0, v7);
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
    int i = 0;
    for (auto& reg : buffer)
        reg = _mm_load_si128((__m128i*) & F[i++]);

    idct_1d_sse2(buffer);
    transpose_8x8_sse2(buffer);
    idct_1d_sse2(buffer);
    transpose_8x8_sse2(buffer);

    for (i = 0; i < 4; i++) {
        __m128i tmp;
        if (add) {
            __m128i dstl = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) & plane[(i * 2 + 0) * stride]), _mm_setzero_si128());
            __m128i dsth = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) & plane[(i * 2 + 1) * stride]), _mm_setzero_si128());
            tmp = _mm_packus_epi16(_mm_adds_epi16(dstl, buffer[i * 2]), _mm_adds_epi16(dsth, buffer[i * 2 + 1]));
        } else
            tmp = _mm_packus_epi16(buffer[i * 2], buffer[i * 2 + 1]);

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