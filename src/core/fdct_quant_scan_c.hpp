// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include <algorithm>
#include "common/cpu.hpp"

uint8_t default_intra_quantiser_matrix[64] = {
    8,  16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
    27, 29, 35, 38, 46, 56, 69, 83 };

uint8_t default_non_intra_quantiser_matrix[64] = {
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16 };

static const double s[8] = {
    0.353553390593273762200422,
    0.254897789552079584470970,
    0.270598050073098492199862,
    0.300672443467522640271861,
    0.353553390593273762200422,
    0.449988111568207852319255,
    0.653281482438188263928322,
    1.281457723870753089398043,
};

static const double a[6] = {
    NAN,
    0.707106781186547524400844,
    0.541196100146196984399723,
    0.707106781186547524400844,
    1.306562964876376527856643,
    0.382683432365089771728460,
};

template<typename src_t, typename dst_t>
void fdct_1d(dst_t dst[8], src_t* src) {
    const double v0 = static_cast<double>(src[0] + src[7]);
    const double v1 = static_cast<double>(src[1] + src[6]);
    const double v2 = static_cast<double>(src[2] + src[5]);
    const double v3 = static_cast<double>(src[3] + src[4]);
    const double v4 = static_cast<double>(src[3] - src[4]);
    const double v5 = static_cast<double>(src[2] - src[5]);
    const double v6 = static_cast<double>(src[1] - src[6]);
    const double v7 = static_cast<double>(src[0] - src[7]);

    const double v8 = v0 + v3;
    const double v9 = v1 + v2;
    const double v10 = v1 - v2;
    const double v11 = v0 - v3;
    const double v12 = -v4 - v5;
    const double v13 = (v5 + v6) * a[3];
    const double v14 = v6 + v7;
    const double v15 = v8 + v9;
    const double v16 = v8 - v9;
    const double v17 = (v10 + v11) * a[1];
    const double v18 = (v12 + v14) * a[5];
    const double v19 = -v12 * a[2] - v18;
    const double v20 = v14 * a[4] - v18;
    const double v21 = v17 + v11;
    const double v22 = v11 - v17;
    const double v23 = v13 + v7;
    const double v24 = v7 - v13;
    const double v25 = v19 + v24;
    const double v26 = v23 + v20;
    const double v27 = v23 - v20;
    const double v28 = v24 - v19;

    dst[0] = static_cast<dst_t>(s[0] * v15);
    dst[1] = static_cast<dst_t>(s[1] * v26);
    dst[2] = static_cast<dst_t>(s[2] * v21);
    dst[3] = static_cast<dst_t>(s[3] * v28);
    dst[4] = static_cast<dst_t>(s[4] * v16);
    dst[5] = static_cast<dst_t>(s[5] * v25);
    dst[6] = static_cast<dst_t>(s[6] * v22);
    dst[7] = static_cast<dst_t>(s[7] * v27);
}

#define COPY(j,i) { dst[j] = src[i]; if(!src[i]) { nnz |= msk; }; msk >>= 1; }

uint64_t make_alt_scan(int16_t dst[65], int16_t src[64])
{
    uint64_t nnz = 0;
    uint64_t msk = 0x8000000000000000;

    COPY(63, 63); COPY(62, 55); COPY(61, 47); COPY(60, 39); COPY(59, 62); COPY(58, 54); COPY(57, 46); COPY(56, 38);
    COPY(55, 31); COPY(54, 23); COPY(53, 15); COPY(52, 7);  COPY(51, 30); COPY(50, 22); COPY(49, 61); COPY(48, 53);
    COPY(47, 45); COPY(46, 37); COPY(45, 60); COPY(44, 52); COPY(43, 44); COPY(42, 36); COPY(41, 29); COPY(40, 21);
    COPY(39, 14); COPY(38, 6);  COPY(37, 13); COPY(36, 5);  COPY(35, 28); COPY(34, 20); COPY(33, 59); COPY(32, 51);
    COPY(31, 43); COPY(30, 35); COPY(29, 58); COPY(28, 50); COPY(27, 42); COPY(26, 34); COPY(25, 27); COPY(24, 19);
    COPY(23, 12); COPY(22, 4);  COPY(21, 11); COPY(20, 3);  COPY(19, 18); COPY(18, 26); COPY(17, 33); COPY(16, 41);
    COPY(15, 49); COPY(14, 57); COPY(13, 56); COPY(12, 48); COPY(11, 40); COPY(10, 32); COPY(9,  25); COPY(8,  17);
    COPY(7,  10); COPY(6,  2);  COPY(5,  9);  COPY(4,  1);  COPY(3,  24); COPY(2,  16); COPY(1,  8);  COPY(0,  0);

    dst[64] = 0;
    return nnz;
}

uint64_t make_zigzag_scan(int16_t dst[65], int16_t src[64])
{
    uint64_t nnz = 0;
    uint64_t msk = 0x8000000000000000;

    COPY(63, 63); COPY(62, 62); COPY(61, 55); COPY(60, 47); COPY(59, 54); COPY(58, 61); COPY(57, 60); COPY(56, 53);
    COPY(55, 46); COPY(54, 39); COPY(53, 31); COPY(52, 38); COPY(51, 45); COPY(50, 52); COPY(49, 59); COPY(48, 58);
    COPY(47, 51); COPY(46, 44); COPY(45, 37); COPY(44, 30); COPY(43, 23); COPY(42, 15); COPY(41, 22); COPY(40, 29);
    COPY(39, 36); COPY(38, 43); COPY(37, 50); COPY(36, 57); COPY(35, 56); COPY(34, 49); COPY(33, 42); COPY(32, 35);
    COPY(31, 28); COPY(30, 21); COPY(29, 14); COPY(28, 7);  COPY(27, 6);  COPY(26, 13); COPY(25, 20); COPY(24, 27);
    COPY(23, 34); COPY(22, 41); COPY(21, 48); COPY(20, 40); COPY(19, 33); COPY(18, 26); COPY(17, 19); COPY(16, 12);
    COPY(15, 5);  COPY(14, 4);  COPY(13, 11); COPY(12, 18); COPY(11, 25); COPY(10, 32); COPY(9,  24); COPY(8,  17);
    COPY(7,  10); COPY(6,  3);  COPY(5,  2);  COPY(4,  9);  COPY(3,  16); COPY(2,  8);  COPY(1,  1);  COPY(0,  0);

    dst[64] = 0;
    return nnz;
}

#define SIGN(val) (val < 0 ? -1 : (val == 0 ? 0 : 1))

#define SATURATE(val) (std::max<int16_t>(std::min<int16_t>(val, (int16_t)2047), (int16_t)-2048))

template<bool alt_scan, bool intra = true>
MP2V_INLINE uint64_t forward_dct_scan_quant_template(int16_t(&F)[65], uint8_t* src, uint8_t W[64], int stride, int quantizer_scale, int dc_prec) {
    double  tmp0[64];
    double  tmp1[64];
    int16_t tmp3[64];

    for (int i = 0; i < 8; i++)
        fdct_1d(&tmp0[i * 8], &src[i * stride]);

    //transpose
    for (int j = 0; j < 8; j++)
        for (int i = 0; i < 8; i++)
            tmp1[j * 8 + i] = tmp0[i * 8 + j];

    for (int i = 0; i < 8; i++)
        fdct_1d(&tmp0[i * 8], &tmp1[i * 8]);

    //transpose & quantization
    for (int j = 0; j < 8; j++)
        for (int i = 0; i < 8; i++)
        {
            int32_t f = (int32_t)tmp0[j * 8 + i];
            int32_t WS = W[i * 8 + j] * quantizer_scale;
            int32_t res = ((f << 5) + SIGN(f) * WS) / (WS * 2);
            tmp3[i * 8 + j] = SATURATE((int16_t)res);
        }
    tmp3[0] = (int16_t)tmp0[0] >> (3 - dc_prec);

    if (alt_scan) return make_alt_scan(F, tmp3); else return make_zigzag_scan(F, tmp3);
}