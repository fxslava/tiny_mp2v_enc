// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>

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

#define COPY(j,i) { dst[j] = src[i]; if(!src[i]) { nnz |= msk; }; msk >>= 1; }

MP2V_INLINE uint64_t make_alt_scan(int16_t dst[65], int16_t src[64])
{
    uint64_t nnz = 0;
    uint64_t msk = 0x8000000000000000;

    COPY(63, 63); COPY(62, 55); COPY(61, 47); COPY(60, 39); COPY(59, 62); COPY(58, 54); COPY(57, 46); COPY(56, 38);
    COPY(55, 31); COPY(54, 23); COPY(53, 15); COPY(52, 7);  COPY(51, 30); COPY(50, 22); COPY(49, 61); COPY(48, 53);
    COPY(47, 45); COPY(46, 37); COPY(45, 60); COPY(44, 52); COPY(43, 44); COPY(42, 36); COPY(41, 29); COPY(40, 21);
    COPY(39, 14); COPY(38, 6);  COPY(37, 13); COPY(36, 5);  COPY(35, 28); COPY(34, 20); COPY(33, 59); COPY(32, 51);
    COPY(31, 43); COPY(30, 35); COPY(29, 58); COPY(28, 50); COPY(27, 42); COPY(26, 34); COPY(25, 27); COPY(24, 19);
    COPY(23, 12); COPY(22, 4);  COPY(21, 11); COPY(20, 3);  COPY(19, 18); COPY(18, 26); COPY(17, 33); COPY(16, 41);
    COPY(15, 49); COPY(14, 57); COPY(13, 56); COPY(12, 48); COPY(11, 40); COPY(10, 32); COPY(9, 25); COPY(8, 17);
    COPY(7, 10); COPY(6, 2);  COPY(5, 9);  COPY(4, 1);  COPY(3, 24); COPY(2, 16); COPY(1, 8);  COPY(0, 0);

    dst[64] = 0;
    return nnz;
}

MP2V_INLINE uint64_t make_zigzag_scan(int16_t dst[65], int16_t src[64])
{
    uint64_t nnz = 0;
    uint64_t msk = 0x8000000000000000;

    COPY(63, 63); COPY(62, 62); COPY(61, 55); COPY(60, 47); COPY(59, 54); COPY(58, 61); COPY(57, 60); COPY(56, 53);
    COPY(55, 46); COPY(54, 39); COPY(53, 31); COPY(52, 38); COPY(51, 45); COPY(50, 52); COPY(49, 59); COPY(48, 58);
    COPY(47, 51); COPY(46, 44); COPY(45, 37); COPY(44, 30); COPY(43, 23); COPY(42, 15); COPY(41, 22); COPY(40, 29);
    COPY(39, 36); COPY(38, 43); COPY(37, 50); COPY(36, 57); COPY(35, 56); COPY(34, 49); COPY(33, 42); COPY(32, 35);
    COPY(31, 28); COPY(30, 21); COPY(29, 14); COPY(28, 7);  COPY(27, 6);  COPY(26, 13); COPY(25, 20); COPY(24, 27);
    COPY(23, 34); COPY(22, 41); COPY(21, 48); COPY(20, 40); COPY(19, 33); COPY(18, 26); COPY(17, 19); COPY(16, 12);
    COPY(15, 5);  COPY(14, 4);  COPY(13, 11); COPY(12, 18); COPY(11, 25); COPY(10, 32); COPY(9, 24); COPY(8, 17);
    COPY(7, 10); COPY(6, 3);  COPY(5, 2);  COPY(4, 9);  COPY(3, 16); COPY(2, 8);  COPY(1, 1);  COPY(0, 0);

    dst[64] = 0;
    return nnz;
}

#define SIGN(val) (val < 0 ? -1 : (val == 0 ? 0 : 1))

template<bool alt_scan, bool intra, typename src_t>
MP2V_INLINE uint64_t transpose_quant_scan(int16_t(&F)[65], src_t(&src)[64], uint8_t W[64], int stride, int quantizer_scale, int dc_prec) {
    int16_t tmp[64];

    for (int j = 0; j < 8; j++)
        for (int i = 0; i < 8; i++)
        {
            int32_t f = (int32_t)src[j * 8 + i];
            int32_t WS = W[i * 8 + j] * quantizer_scale;
            int32_t res = ((f << 5) + SIGN(f) * WS) / (WS * 2);
            tmp[i * 8 + j] = (int16_t)res;
        }
    tmp[0] = (int16_t)src[0] >> (3 - dc_prec);

    if (alt_scan) return make_alt_scan(F, tmp); else return make_zigzag_scan(F, tmp);
}
