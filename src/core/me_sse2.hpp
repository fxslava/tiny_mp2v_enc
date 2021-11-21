#pragma once
#include "common/cpu.hpp"
#include <stdint.h>
#include <emmintrin.h>

MP2V_INLINE void downscale_kernel(uint8_t* dst, uint8_t* src, size_t stride) {
    __m128i tmp0 = _mm_load_si128((__m128i*) & src[0]);
    __m128i tmp1 = _mm_load_si128((__m128i*) & src[stride]);
    __m128i zero = _mm_setzero_si128();
    __m128i tmp2 = _mm_add_epi16(_mm_unpacklo_epi8(tmp0, zero), _mm_unpacklo_epi8(tmp1, zero));
    __m128i tmp3 = _mm_add_epi16(_mm_unpackhi_epi8(tmp0, zero), _mm_unpackhi_epi8(tmp1, zero));
    __m128i res0 = _mm_srli_epi16(_mm_add_epi16(_mm_hadd_epi16(tmp2, tmp3), _mm_set1_epi16(3)), 2);
            tmp0 = _mm_load_si128((__m128i*) & src[stride * 2]);
            tmp1 = _mm_load_si128((__m128i*) & src[stride * 4 - 1]);
            zero = _mm_setzero_si128();
            tmp2 = _mm_add_epi16(_mm_unpacklo_epi8(tmp0, zero), _mm_unpacklo_epi8(tmp1, zero));
            tmp3 = _mm_add_epi16(_mm_unpackhi_epi8(tmp0, zero), _mm_unpackhi_epi8(tmp1, zero));
    __m128i res1 = _mm_srli_epi16(_mm_add_epi16(_mm_hadd_epi16(tmp2, tmp3), _mm_set1_epi16(3)), 2);
    __m128i res  = _mm_packus_epi16(res0, res1);
    _mm_storel_epi64((__m128i*) & dst[0], res);
    _mm_storel_epi64((__m128i*) & dst[stride], _mm_srli_si128(res, 8));
}

template<int border_width, bool UP>
MP2V_INLINE void make_horiz_border(uint8_t* dst, int width, int left, int right, int stride) {
    uint8_t* ptr = dst - left;
    for (int x = 0; x < width + left + right; x += 16, ptr += 16) {
        __m128i tmp = _mm_load_si128((__m128i*) ptr);
        for (int i = 0; i < border_width; i++)
            _mm_store_si128((__m128i*) & ptr[(UP ? -1 : 1) * i * stride], tmp);
    }
}

template<int border_width, bool LEFT = false>
MP2V_INLINE void make_vert_border(uint8_t* dst, int height, int stride) {
    uint8_t* ptr = dst;
    for (int y = 0; y < height; y++, ptr += stride) {
        int8_t filler = ((int8_t*)ptr)[0];
        __m128i tmp = _mm_set1_epi8(filler);
        for (int i = 0; i < border_width; i += 16)
            _mm_store_si128((__m128i*) ptr + (LEFT ? -i : i), tmp);
    }
}
