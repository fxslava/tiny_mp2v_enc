// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.

#pragma once
#include <stdint.h>

void inverse_dct_c(uint8_t* plane, int16_t F[64], int stride);
void add_inverse_dct_c(uint8_t* plane, int16_t F[64], int stride);
void inverse_dct_sse2(uint8_t* plane, int16_t F[64], int stride);
void add_inverse_dct_sse2(uint8_t* plane, int16_t F[64], int stride);