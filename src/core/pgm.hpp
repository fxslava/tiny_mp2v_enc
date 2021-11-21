#pragma once
#include <stdint.h>
#include <stdio.h>
#include "common/cpu.hpp"

MP2V_INLINE void save_image(const char* filename, uint8_t* plane, int width, int height, int stride) {
    constexpr int MAXVAL = 65535;

    FILE* fp = fopen(filename, "wb");
    fprintf(fp, "P5\n");
    fprintf(fp, "%d\n", width);
    fprintf(fp, "%d\n", height);
    fprintf(fp, "%d\n", 255);

    for (int y = 0; y < height; y++)
        fwrite(&plane[y * stride], sizeof(uint8_t), width, fp);

    fclose(fp);
}
