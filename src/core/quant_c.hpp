// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "common/cpu.hpp" 
#include <stdint.h>
#include <algorithm>

template <typename T> 
MP2V_INLINE int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template<bool intra>
MP2V_INLINE int16_t inverse_quant_scalar_c(int16_t &F, int16_t QF, uint16_t W, uint8_t quantizer_scale) {
    int32_t k = intra ? 0 : sgn(QF);
    int16_t res = (int16_t)(((((int32_t)QF * 2) + k) * (int32_t)W * (int32_t)quantizer_scale) / 32);
    F = std::max<int16_t>(std::min<int16_t>(res, (int16_t)2047), (int16_t)-2048);
    return F;
}

template<bool intra>
MP2V_INLINE void inverse_quant_template_c(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_precision) {
    int32_t sum = 0;
    int i = intra ? 1 : 0;
    if (intra)
    {
        int32_t res = QF[0] << (3 - intra_dc_precision);
        F[0] = res;
        sum += res;
    }
    for (; i < 64; i++)
        sum += inverse_quant_scalar_c<intra>(F[i], QF[i], W[i], quantizer_scale);
    F[63] = F[63] ^ (sum & 1); //Mismatch control
}

MP2V_INLINE void inverse_quant_c(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale) {
    inverse_quant_template_c<false>(F, QF, W, quantizer_scale, 0);
}

MP2V_INLINE void inverse_quant_intra_c(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_precision) {
    inverse_quant_template_c<true>(F, QF, W, quantizer_scale, intra_dc_precision);
}