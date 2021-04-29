// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>

void inverse_quant_c(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale);
void inverse_quant_intra_c(int16_t F[64], int16_t QF[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_precision);