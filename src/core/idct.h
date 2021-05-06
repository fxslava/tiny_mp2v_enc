// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#pragma once
#include <stdint.h>

void inverse_dct(uint8_t* plane, int16_t F[64], int stride);
void add_cache16_inverse_dct(uint8_t* plane, uint8_t* src, int16_t F[64], int dst_stride);
void add_cache8_inverse_dct(uint8_t* plane, uint8_t* src, int16_t F[64], int dst_stride);
