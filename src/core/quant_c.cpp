#include "quant.h"
#include <algorithm>

uint16_t default_intra_quantiser_matrix[64] = {
    8,  16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
    27, 29, 35, 38, 46, 56, 69, 83 };

uint16_t default_non_intra_quantiser_matrix[64] = {
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16 };

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

template<bool intra>
int16_t inverse_quant_scalar_c(int16_t &F, int16_t QF, uint16_t W, uint8_t quantizer_scale) {
    int32_t k = intra ? 0 : sgn(QF);
    int16_t res = (int16_t)(((((int32_t)QF << 1) + k) * (int32_t)W * (int32_t)quantizer_scale) >> 5);
    F = std::max<int16_t>(std::min<int16_t>(res, (int16_t)2047), (int16_t)-2048);
    return F;
}

template<bool intra>
void inverse_quant_template_c(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_precision) {
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

void inverse_quant_c(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale) {
    inverse_quant_template_c<false>(F, QF, W, quantizer_scale, 0);
}

void inverse_quant_intra_c(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_precision) {
    inverse_quant_template_c<true>(F, QF, W, quantizer_scale, intra_dc_precision);
}