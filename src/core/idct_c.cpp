// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include <stdint.h>
#include <algorithm>
#include <math.h>
#include "idct.h"

static double cos_lut[8][8];

void idct_init(void)
{
    for (int i = 0; i < 8; i++)
        for (int j = 0; j < 8; j++) {
            double s = (j == 0) ? 1.0 / sqrt(2.0) : 1.0;
            cos_lut[i][j] = (double)cos(((2 * i + 1) * j) * (3.14159265358979323846 / 16.0)) * s;
        }
}

void inverse_dct(uint8_t* plane, int16_t F[64], int stride) {
    double tmp[8][8];

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            double sum = 0.0;
            for (int v = 0; v < 8; v++) {
                float conv = 0.0;
                for (int u = 0; u < 8; u++)
                    conv += (double)F[v * 8 + u] * cos_lut[x][u];
                sum += cos_lut[y][v] * conv;
            }
            tmp[y][x] = sum * (2.0 / 8.0);
        }
    }

    for (int v = 0; v < 8; v++)
        for (int u = 0; u < 8; u++) {
            int res = (int)tmp[v][u];
            plane[v * stride + u] = (uint8_t)(std::max(std::min(res, 255), 0));
        }
}