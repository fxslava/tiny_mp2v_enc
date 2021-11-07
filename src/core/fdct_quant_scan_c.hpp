// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include <algorithm>
#include "common/cpu.hpp"
#include "quant_scan_c.hpp"

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

template<bool alt_scan, bool intra = true>
MP2V_INLINE uint64_t forward_dct_scan_quant_template(int16_t(&F)[65], uint8_t* src, int stride, int quantizer_scale, int dc_prec) {
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
    return transpose_quant_scan<alt_scan, intra>(F, tmp0, intra ? default_intra_quantiser_matrix : default_non_intra_quantiser_matrix, stride, quantizer_scale, dc_prec);
}