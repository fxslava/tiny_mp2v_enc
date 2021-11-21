#include <algorithm>
#include "common/cpu.hpp"
#include "motion_estimator.h"

#if defined(CPU_PLATFORM_AARCH64)
#include "me_aarch64.hpp"
#elif defined(CPU_PLATFORM_X64)
#include "me_sse2.hpp"
#else
#include "me_c.hpp"
#endif

#include "pgm.hpp"

constexpr int PU_SIZE = 16;
constexpr int STRIDE_ALIGNMENT = 32;

#define ALIGN_SIZE(size, align_size) (((size) + (align_size) - 1) & ~((align_size) - 1))

void create_pyramide(pyramide_t& pyr, int width, int height, int num_levels, int border_width) {
    int cur_width  = width;
    int cur_height = height;
    pyr.num_levels = num_levels;

    for (int l = 0; l < num_levels; l++) {
        auto& level = pyr.levels[l];
        level.width = ALIGN_SIZE(cur_width, PU_SIZE);
        level.height = ALIGN_SIZE(cur_height, PU_SIZE);
        level.borders.left = border_width;
        level.borders.top = border_width;
        level.borders.right = level.width - cur_width + border_width;
        level.borders.bottom = level.height - cur_height + border_width;
        level.stride = ALIGN_SIZE(cur_width + border_width * 2, STRIDE_ALIGNMENT);
        level.layer_data = new uint8_t[level.stride * (level.height + border_width * 2)];
        level.layer_origin = level.layer_data + border_width * (1 + level.stride);

        cur_width /= 2;
        cur_height /= 2;
    }
}

void calculate_pyramide(pyramide_t& pyr, uint8_t* plane, int stride) {
    for (int l = 0; l < pyr.num_levels; l++) {
        auto& level = pyr.levels[l];

        for (int y = 0; y < level.height; y++)
            memcpy(level.layer_origin + y * level.stride, plane + y * stride, level.width);

        make_vert_border <16, true >(level.layer_origin, level.height, level.stride);
        make_vert_border <16, false>(level.layer_origin, level.height, level.stride);
        make_horiz_border<16, true >(level.layer_origin, level.width,  level.borders.left, level.borders.right, level.stride);
        make_horiz_border<16, false>(level.layer_origin, level.width,  level.borders.left, level.borders.right, level.stride);
    }
}
