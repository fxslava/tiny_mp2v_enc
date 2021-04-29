// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>

extern uint8_t g_scan[2][64];

// PLANE C routines
void inverse_alt_scan_c(int16_t QF[64], int16_t QFS[64]);
void inverse_scan_c(int16_t QF[64], int16_t QFS[64]);