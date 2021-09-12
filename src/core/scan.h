// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "common/cpu.hpp"
#include <stdint.h>

extern uint8_t g_scan[2][64];

// PLANE C routines
MP2V_INLINE void inverse_alt_scan_c(int16_t QF[64], int16_t QFS[64]) {
    QF[0 ] = QFS[0 ]; QF[8 ] = QFS[1 ]; QF[16] = QFS[2 ]; QF[24] = QFS[3 ]; QF[1 ] = QFS[4 ]; QF[9 ] = QFS[5 ]; QF[2 ] = QFS[6 ]; QF[10] = QFS[7 ]; 
    QF[17] = QFS[8 ]; QF[25] = QFS[9 ]; QF[32] = QFS[10]; QF[40] = QFS[11]; QF[48] = QFS[12]; QF[56] = QFS[13]; QF[57] = QFS[14]; QF[49] = QFS[15];
    QF[41] = QFS[16]; QF[33] = QFS[17]; QF[26] = QFS[18]; QF[18] = QFS[19]; QF[3 ] = QFS[20]; QF[11] = QFS[21]; QF[4 ] = QFS[22]; QF[12] = QFS[23]; 
    QF[19] = QFS[24]; QF[27] = QFS[25]; QF[34] = QFS[26]; QF[42] = QFS[27]; QF[50] = QFS[28]; QF[58] = QFS[29]; QF[35] = QFS[30]; QF[43] = QFS[31];
    QF[51] = QFS[32]; QF[59] = QFS[33]; QF[20] = QFS[34]; QF[28] = QFS[35]; QF[5 ] = QFS[36]; QF[13] = QFS[37]; QF[6 ] = QFS[38]; QF[14] = QFS[39]; 
    QF[21] = QFS[40]; QF[29] = QFS[41]; QF[36] = QFS[42]; QF[44] = QFS[43]; QF[52] = QFS[44]; QF[60] = QFS[45]; QF[37] = QFS[46]; QF[45] = QFS[47];
    QF[53] = QFS[48]; QF[61] = QFS[49]; QF[22] = QFS[50]; QF[30] = QFS[51]; QF[7 ] = QFS[52]; QF[15] = QFS[53]; QF[23] = QFS[54]; QF[31] = QFS[55]; 
    QF[38] = QFS[56]; QF[46] = QFS[57]; QF[54] = QFS[58]; QF[62] = QFS[59]; QF[39] = QFS[60]; QF[47] = QFS[61]; QF[55] = QFS[62]; QF[63] = QFS[63];
    /*for (int i = 0; i < 64; i++)
        QF[g_scan[1][i]] = QFS[i];*/
}

MP2V_INLINE void inverse_scan_c(int16_t QF[64], int16_t QFS[64]) {
    for (int i = 0; i < 64; i++)
        QF[i] = QFS[g_scan[0][i]];
}