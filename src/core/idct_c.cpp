// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include <stdint.h>
#include <algorithm>
#include <math.h>
#include "idct.h"

constexpr double s1[5] = {
    0.707106781186547524400844,
    0.541196100146196984399723,
    0.707106781186547524400844,
    1.306562964876376527856643,
    0.382683432365089771728460,
};

template<typename src_t, typename dst_t>
void idct_1d(dst_t dst[8], src_t *src) {
    const double v15 = static_cast<double>(src[0]) / 0.353553390593273762200422;
    const double v26 = static_cast<double>(src[1]) / 0.254897789552079584470970;
    const double v21 = static_cast<double>(src[2]) / 0.270598050073098492199862;
    const double v28 = static_cast<double>(src[3]) / 0.300672443467522640271861;
    const double v16 = static_cast<double>(src[4]) / 0.353553390593273762200422;
    const double v25 = static_cast<double>(src[5]) / 0.449988111568207852319255;
    const double v22 = static_cast<double>(src[6]) / 0.653281482438188263928322;
    const double v27 = static_cast<double>(src[7]) / 1.281457723870753089398043;
    
    const double v19 = (v25 - v28) / 2;
    const double v20 = (v26 - v27) / 2;
    const double v23 = (v26 + v27) / 2;
    const double v24 = (v25 + v28) / 2;
    const double v7  = (v23 + v24) / 2;
    const double v11 = (v21 + v22) / 2;
    const double v13 = (v23 - v24) / 2;
    const double v17 = (v21 - v22) / 2;
    const double v8  = (v15 + v16) / 2;
    const double v9  = (v15 - v16) / 2;
    const double v18 = (v19 - v20) * s1[4];
    const double v12 = (v19 * s1[3] - v18) / (s1[1] * s1[4] - s1[1] * s1[3] - s1[3] * s1[4]);
    const double v14 = (v18 - v20 * s1[1]) / (s1[1] * s1[4] - s1[1] * s1[3] - s1[3] * s1[4]);
    const double v6  = v14 - v7;
    const double v5  = v13 / s1[2] - v6;
    const double v4  = -v5 - v12;
    const double v10 = v17 / s1[0] - v11;
    const double v0  = (v8 + v11) / 2;
    const double v1  = (v9 + v10) / 2;
    const double v2  = (v9 - v10) / 2;
    const double v3  = (v8 - v11) / 2;
    
    dst[0] = static_cast<dst_t>((v0 + v7) / 2);
    dst[1] = static_cast<dst_t>((v1 + v6) / 2);
    dst[2] = static_cast<dst_t>((v2 + v5) / 2);
    dst[3] = static_cast<dst_t>((v3 + v4) / 2);
    dst[4] = static_cast<dst_t>((v3 - v4) / 2);
    dst[5] = static_cast<dst_t>((v2 - v5) / 2);
    dst[6] = static_cast<dst_t>((v1 - v6) / 2);
    dst[7] = static_cast<dst_t>((v0 - v7) / 2);
}

template<typename dst_t, typename src_t, bool add, int src_stride>
void inverse_dct_template(dst_t* plane, src_t* src, int16_t F[64], int dst_stride) {
    double tmp0[8][8];
    double tmp1[8][8];

    for (int i = 0; i < 8; i++)
        idct_1d(tmp0[i], &F[i * 8]);

    //transpose
    for (int j = 0; j < 8; j++)
        for (int i = 0; i < 8; i++)
            tmp1[i][j] = tmp0[j][i];

    for (int i = 0; i < 8; i++)
        idct_1d(tmp0[i], tmp1[i]);

    //transpose store
    for (int j = 0; j < 8; j++)
        for (int i = 0; i < 8; i++) {
            int res = add ? (int)tmp0[i][j] + (int)src[j * src_stride + i] : (int)tmp0[i][j];
            plane[j * dst_stride + i] = (dst_t)(std::max(std::min(res, 255), 0));
        }
}

void inverse_dct(uint8_t* plane, int16_t F[64], int stride) {
    inverse_dct_template<uint8_t, uint8_t, false, 0>(plane, nullptr, F, stride);
}
void add_cache16_inverse_dct(uint8_t* plane, uint8_t* src, int16_t F[64], int dst_stride) {
    inverse_dct_template<uint8_t, uint8_t, true, 16>(plane, src, F, dst_stride);
}

void add_cache8_inverse_dct(uint8_t* plane, uint8_t* src, int16_t F[64], int dst_stride) {
    inverse_dct_template<uint8_t, uint8_t, true, 8>(plane, src, F, dst_stride);
}