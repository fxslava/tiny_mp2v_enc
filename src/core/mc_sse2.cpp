#include "mc.h"
#include "common/cpu.hpp"
#include <emmintrin.h>

MP2V_INLINE void store_linex4_w8_sse2(uint8_t* dst, __m128i vdst[4], uint32_t stride) {
    for (int i = 0; i < 4; i++)
        _mm_storel_epi64((__m128i*) & dst[i * stride], vdst[i]);
}

MP2V_INLINE void store_linex8_w8_sse2(uint8_t* dst, __m128i vdst[4], uint32_t stride) {
    for (int i = 0; i < 4; i++) {
        _mm_storel_epi64((__m128i*) & dst[stride * (i * 2 + 0)], vdst[i]);
        _mm_storel_epi64((__m128i*) & dst[stride * (i * 2 + 1)], _mm_bsrli_si128(vdst[i], 8));
    }
}

MP2V_INLINE void store_linex4_w16_sse2(uint8_t* dst, __m128i vdst[4], uint32_t stride) {
    for (int i = 0; i < 4; i++)
        _mm_store_si128((__m128i*) & dst[i * stride], vdst[i]);
}

MP2V_INLINE void pred_linex4_mc00_w8_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    for (int i = 0; i < 4; i++)
        dst[i] = _mm_loadl_epi64((__m128i*) & src[i * stride]);
}

MP2V_INLINE void pred_linex8_mc00_w8_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    for (int i = 0; i < 4; i++)
        dst[i] = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) & src[i * 2 * stride]), _mm_loadl_epi64((__m128i*) & src[(i * 2 + 1) * stride]));
}

void mc_pred00_8xh_sse2(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) {
    __m128i tmp[4];
    for (int i = 0; i < height; i += 4) {
        pred_linex4_mc00_w8_sse2(tmp, &src[i * stride], stride);
        store_linex4_w8_sse2(&dst[i * stride], tmp, stride);
    }
}

MP2V_INLINE void pred_linex4_mc00_w16_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    for (int i = 0; i < 4; i++)
        dst[i] = _mm_loadu_si128((__m128i*) & src[i * stride]);
}

void mc_pred00_16xh_sse2(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) {
    __m128i tmp[4];
    for (int i = 0; i < height; i += 4) {
        pred_linex4_mc00_w16_sse2(tmp, &src[i * stride], stride);
        store_linex4_w16_sse2(&dst[i * stride], tmp, stride);
    }
}

MP2V_INLINE void pred_linex8_mc01_w8_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    for (int i = 0; i < 4; i++) {
        __m128i tmp0 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) & src[(i * 2 + 0) * stride]), _mm_loadl_epi64((__m128i*) & src[(i * 2 + 1) * stride]));
        __m128i tmp1 = _mm_unpacklo_epi8(_mm_loadl_epi64((__m128i*) & src[(i * 2 + 0) * stride + 1]), _mm_loadl_epi64((__m128i*) & src[(i * 2 + 1) * stride + 1]));
        dst[i] = _mm_avg_epu8(tmp0, tmp1);
    }
}

void mc_pred01_8xh_sse2(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) {
    __m128i tmp[4];
    if (height == 8) {
        pred_linex8_mc01_w8_sse2(tmp, src, stride);
        store_linex8_w8_sse2(dst, tmp, stride);
    }
}

MP2V_INLINE void pred_linex4_mc01_w16_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    for (int i = 0; i < 4; i++) {
        __m128i tmp0 = _mm_loadu_si128((__m128i*) & src[i * stride]);
        __m128i tmp1 = _mm_loadu_si128((__m128i*) & src[i * stride + 1]);
        dst[i] = _mm_avg_epu8(tmp0, tmp1);
    }
}

void mc_pred01_16xh_sse2(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) {
    __m128i tmp[4];
    for (int i = 0; i < height; i += 4) {
        pred_linex4_mc01_w16_sse2(tmp, &src[i * stride], stride);
        store_linex4_w16_sse2(&dst[i * stride], tmp, stride);
    }
}

MP2V_INLINE __m128i pred_linex2_mc10_w8_sse2(const __m128i buf0, const __m128i buf1, const __m128i buf2, uint32_t stride) {
    __m128i tmp0 = _mm_unpacklo_epi8(buf0, buf1);
    __m128i tmp1 = _mm_unpacklo_epi8(buf1, buf2);
    return _mm_avg_epu8(tmp0, tmp1);
}

MP2V_INLINE void pred_linex8_mc10_w8_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    __m128i buf0 = _mm_loadl_epi64((__m128i*) & src[0 * stride]);
    __m128i buf1 = _mm_loadl_epi64((__m128i*) & src[1 * stride]);
    __m128i buf2 = _mm_loadl_epi64((__m128i*) & src[2 * stride]);
    dst[0] = pred_linex2_mc10_w8_sse2(buf0, buf1, buf2, stride);
    buf0 = _mm_loadl_epi64((__m128i*) & src[3 * stride]);
    buf1 = _mm_loadl_epi64((__m128i*) & src[4 * stride]);
    dst[1] = pred_linex2_mc10_w8_sse2(buf2, buf0, buf1, stride);
    buf2 = _mm_loadl_epi64((__m128i*) & src[5 * stride]);
    buf0 = _mm_loadl_epi64((__m128i*) & src[6 * stride]);
    dst[2] = pred_linex2_mc10_w8_sse2(buf1, buf2, buf0, stride);
    buf1 = _mm_loadl_epi64((__m128i*) & src[7 * stride]);
    buf2 = _mm_loadl_epi64((__m128i*) & src[8 * stride]);
    dst[3] = pred_linex2_mc10_w8_sse2(buf0, buf1, buf2, stride);
}

void mc_pred10_8xh_sse2(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) {
    __m128i tmp[4];
    if (height == 8) {
        pred_linex8_mc10_w8_sse2(tmp, src, stride);
        store_linex8_w8_sse2(dst, tmp, stride);
    }
}

MP2V_INLINE void pred_linex2_mc10_w16_sse2(__m128i (&dst)[4], const __m128i buf0, const __m128i buf1, uint32_t stride) {
    _mm_store_si128((__m128i*) dst, _mm_avg_epu8(buf0, buf1));
}

MP2V_INLINE void pred_linex4_mc10_w16_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    __m128i tmp1, tmp0 = _mm_loadu_si128((__m128i*) & src[0]);
    for (int i = 0; i < 4; i += 2) {
        tmp1 = _mm_loadu_si128((__m128i*) & src[(i + 1) * stride]);
        dst[i * 2 + 0] = _mm_avg_epu8(tmp0, tmp1);
        tmp0 = _mm_loadu_si128((__m128i*) & src[(i + 2) * stride]);
        dst[i * 2 + 1] = _mm_avg_epu8(tmp1, tmp0);
    }
}

void mc_pred10_16xh_sse2(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) {
    __m128i tmp[4];
    for (int i = 0; i < height; i += 4) {
        pred_linex4_mc10_w16_sse2(tmp, &src[i * stride], stride);
        store_linex4_w16_sse2(&dst[i * stride], tmp, stride);
    }
}

MP2V_INLINE __m128i avg_load_linex2_w8_sse2(uint8_t* src) {
    return _mm_avg_epu8(_mm_loadl_epi64((__m128i*) & src[0]), _mm_loadl_epi64((__m128i*) & src[1]));
}

MP2V_INLINE void pred_linex8_mc11_w8_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    __m128i buf0 = avg_load_linex2_w8_sse2(&src[0 * stride]);
    __m128i buf1 = avg_load_linex2_w8_sse2(&src[1 * stride]);
    __m128i buf2 = avg_load_linex2_w8_sse2(&src[2 * stride]);
    dst[0] = pred_linex2_mc10_w8_sse2(buf0, buf1, buf2, stride);
    buf0 = avg_load_linex2_w8_sse2(&src[3 * stride]);
    buf1 = avg_load_linex2_w8_sse2(&src[4 * stride]);
    dst[1] = pred_linex2_mc10_w8_sse2(buf2, buf0, buf1, stride);
    buf2 = avg_load_linex2_w8_sse2(&src[5 * stride]);
    buf0 = avg_load_linex2_w8_sse2(&src[6 * stride]);
    dst[2] = pred_linex2_mc10_w8_sse2(buf1, buf2, buf0, stride);
    buf1 = avg_load_linex2_w8_sse2(&src[7 * stride]);
    buf2 = avg_load_linex2_w8_sse2(&src[8 * stride]);
    dst[3] = pred_linex2_mc10_w8_sse2(buf0, buf1, buf2, stride);
}

void mc_pred11_8xh_sse2(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) {
    __m128i tmp[4];
    if (height == 8) {
        pred_linex8_mc11_w8_sse2(tmp, src, stride);
        store_linex8_w8_sse2(dst, tmp, stride);
    }
}

MP2V_INLINE __m128i avg_loadx2_w16_sse2(uint8_t* src) {
    return _mm_avg_epu8(_mm_loadu_si128((__m128i*) & src[0]), _mm_loadu_si128((__m128i*) & src[1]));
}

MP2V_INLINE void pred_linex4_mc11_w16_sse2(__m128i (&dst)[4], uint8_t* src, uint32_t stride) {
    __m128i tmp1, tmp0 = avg_loadx2_w16_sse2(&src[0]);
    for (int i = 0; i < 4; i += 2) {
        tmp1 = avg_loadx2_w16_sse2(&src[(i + 1) * stride]);
        dst[i * 2 + 0] = _mm_avg_epu8(tmp0, tmp1);
        tmp0 = avg_loadx2_w16_sse2(&src[(i + 2) * stride]);
        dst[i * 2 + 1] = _mm_avg_epu8(tmp1, tmp0);
    }
}

void mc_pred11_16xh_sse2(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { 
    __m128i tmp[4];
    for (int i = 0; i < height; i += 4) {
        pred_linex4_mc11_w16_sse2(tmp, &src[i * stride], stride);
        store_linex4_w16_sse2(&dst[i * stride], tmp, stride);
    }
}

mc_pred_func_t mc_pred_16xh_sse2[4] = {
    mc_pred00_16xh_sse2,
    mc_pred01_16xh_sse2,
    mc_pred10_16xh_sse2,
    mc_pred11_16xh_sse2
};

mc_pred_func_t mc_pred_8xh_sse2[4] = {
    mc_pred00_8xh_sse2,
    mc_pred01_8xh_sse2,
    mc_pred10_8xh_sse2,
    mc_pred11_8xh_sse2
};

template<mc_type_e mc_type_src0, mc_type_e mc_type_src1, int width>
MP2V_INLINE void bidir_mc_template_w16_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, int height)
{
    __m128i tmp0[4], tmp1[4];
    for (int i = 0; i < height; i += 4) {
        switch (mc_type_src0) {
        case MC_00: pred_linex4_mc00_w16_sse2(tmp0, &src0[i * stride], stride); break;
        case MC_01: pred_linex4_mc01_w16_sse2(tmp0, &src0[i * stride], stride); break;
        case MC_10: pred_linex4_mc10_w16_sse2(tmp0, &src0[i * stride], stride); break;
        case MC_11: pred_linex4_mc11_w16_sse2(tmp0, &src0[i * stride], stride); break;
        }
        switch (mc_type_src1) {
        case MC_00: pred_linex4_mc00_w16_sse2(tmp1, &src1[i * stride], stride); break;
        case MC_01: pred_linex4_mc01_w16_sse2(tmp1, &src1[i * stride], stride); break;
        case MC_10: pred_linex4_mc10_w16_sse2(tmp1, &src1[i * stride], stride); break;
        case MC_11: pred_linex4_mc11_w16_sse2(tmp1, &src1[i * stride], stride); break;
        }

        for (int i = 0; i < 4; i++)
            tmp0[i] = _mm_avg_epu8(tmp0[i], tmp1[i]);

        store_linex4_w16_sse2(&dst[i * stride], tmp0, stride);
    }
}

template<mc_type_e mc_type_src0, mc_type_e mc_type_src1, int width>
MP2V_INLINE void bidir_mc_template_w8_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, int height)
{
    __m128i tmp0[4], tmp1[4];
    bool forward_mc00 = !((mc_type_src0 != MC_00) || (mc_type_src1 != MC_00));
    for (int i = 0; i < height; i += (forward_mc00 ? 8 : 4)) {
        switch (mc_type_src0) {
        case MC_00: 
            if (mc_type_src0 != mc_type_src1)
                pred_linex8_mc00_w8_sse2(tmp0, &src0[i * stride], stride);
            else
                pred_linex4_mc00_w8_sse2(tmp0, &src0[i * stride], stride); 
            break;
        case MC_01: pred_linex8_mc01_w8_sse2(tmp0, &src0[i * stride], stride); break;
        case MC_10: pred_linex8_mc10_w8_sse2(tmp0, &src0[i * stride], stride); break;
        case MC_11: pred_linex8_mc11_w8_sse2(tmp0, &src0[i * stride], stride); break;
        }
        switch (mc_type_src1) {
        case MC_00: 
            if (mc_type_src0 != mc_type_src1)
                pred_linex8_mc00_w8_sse2(tmp1, &src1[i * stride], stride);
            else
                pred_linex4_mc00_w8_sse2(tmp1, &src1[i * stride], stride); 
            break;
        case MC_01: pred_linex8_mc01_w8_sse2(tmp1, &src1[i * stride], stride); break;
        case MC_10: pred_linex8_mc10_w8_sse2(tmp1, &src1[i * stride], stride); break;
        case MC_11: pred_linex8_mc11_w8_sse2(tmp1, &src1[i * stride], stride); break;
        }

        for (int i = 0; i < 4; i++)
            tmp0[i] = _mm_avg_epu8(tmp0[i], tmp1[i]);

        if (forward_mc00)
            store_linex4_w8_sse2(&dst[i * stride], tmp0, stride);
        else
            store_linex8_w8_sse2(&dst[i * stride], tmp0, stride);
    }
}

void mc_bidir0000_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_00, MC_00, 16>(dst, src0, src1, stride, height); }
void mc_bidir0001_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_00, MC_01, 16>(dst, src0, src1, stride, height); }
void mc_bidir0010_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_00, MC_10, 16>(dst, src0, src1, stride, height); }
void mc_bidir0011_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_00, MC_11, 16>(dst, src0, src1, stride, height); }
void mc_bidir0100_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_01, MC_00, 16>(dst, src0, src1, stride, height); }
void mc_bidir0101_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_01, MC_01, 16>(dst, src0, src1, stride, height); }
void mc_bidir0110_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_01, MC_10, 16>(dst, src0, src1, stride, height); }
void mc_bidir0111_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_01, MC_11, 16>(dst, src0, src1, stride, height); }
void mc_bidir1000_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_10, MC_00, 16>(dst, src0, src1, stride, height); }
void mc_bidir1001_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_10, MC_01, 16>(dst, src0, src1, stride, height); }
void mc_bidir1010_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_10, MC_10, 16>(dst, src0, src1, stride, height); }
void mc_bidir1011_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_10, MC_11, 16>(dst, src0, src1, stride, height); }
void mc_bidir1100_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_11, MC_00, 16>(dst, src0, src1, stride, height); }
void mc_bidir1101_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_11, MC_01, 16>(dst, src0, src1, stride, height); }
void mc_bidir1110_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_11, MC_10, 16>(dst, src0, src1, stride, height); }
void mc_bidir1111_16xh_sse2(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w16_sse2<MC_11, MC_11, 16>(dst, src0, src1, stride, height); }
void mc_bidir0000_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_00, MC_00, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0001_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_00, MC_01, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0010_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_00, MC_10, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0011_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_00, MC_11, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0100_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_01, MC_00, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0101_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_01, MC_01, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0110_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_01, MC_10, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0111_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_01, MC_11, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1000_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_10, MC_00, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1001_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_10, MC_01, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1010_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_10, MC_10, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1011_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_10, MC_11, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1100_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_11, MC_00, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1101_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_11, MC_01, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1110_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_11, MC_10, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1111_8xh_sse2 (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template_w8_sse2<MC_11, MC_11, 8 >(dst, src0, src1, stride, height); }

mc_bidir_func_t mc_bidir_16xh_sse2[16] = {
    mc_bidir0000_16xh_sse2,
    mc_bidir0001_16xh_sse2,
    mc_bidir0010_16xh_sse2,
    mc_bidir0011_16xh_sse2,
    mc_bidir0100_16xh_sse2,
    mc_bidir0101_16xh_sse2,
    mc_bidir0110_16xh_sse2,
    mc_bidir0111_16xh_sse2,
    mc_bidir1000_16xh_sse2,
    mc_bidir1001_16xh_sse2,
    mc_bidir1010_16xh_sse2,
    mc_bidir1011_16xh_sse2,
    mc_bidir1100_16xh_sse2,
    mc_bidir1101_16xh_sse2,
    mc_bidir1110_16xh_sse2,
    mc_bidir1111_16xh_sse2
};

mc_bidir_func_t mc_bidir_8xh_sse2[16] = {
    mc_bidir0000_8xh_sse2,
    mc_bidir0001_8xh_sse2,
    mc_bidir0010_8xh_sse2,
    mc_bidir0011_8xh_sse2,
    mc_bidir0100_8xh_sse2,
    mc_bidir0101_8xh_sse2,
    mc_bidir0110_8xh_sse2,
    mc_bidir0111_8xh_sse2,
    mc_bidir1000_8xh_sse2,
    mc_bidir1001_8xh_sse2,
    mc_bidir1010_8xh_sse2,
    mc_bidir1011_8xh_sse2,
    mc_bidir1100_8xh_sse2,
    mc_bidir1101_8xh_sse2,
    mc_bidir1110_8xh_sse2,
    mc_bidir1111_8xh_sse2
};
