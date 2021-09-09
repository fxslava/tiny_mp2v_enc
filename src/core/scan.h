// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "common/cpu.hpp"
#include <stdint.h>

extern uint8_t g_scan[2][64];

// PLANE C routines
MP2V_INLINE void inverse_alt_scan_c(int16_t QF[64], int16_t QFS[64]) {
    for (int i = 0; i < 64; i++)
        QF[g_scan[1][i]] = QFS[i];
}

MP2V_INLINE void inverse_scan_c(int16_t QF[64], int16_t QFS[64]) {
    for (int i = 0; i < 64; i++)
        QF[i] = QFS[g_scan[0][i]];
}