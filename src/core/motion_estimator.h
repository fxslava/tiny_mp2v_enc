#pragma once

constexpr int MAX_NUM_PYRAMIDE_LEVELS = 5;

struct borders_t {
    int left;
    int right;
    int top;
    int bottom;
};

struct pyramide_level_t {
    int width;
    int height;
    int stride;
    borders_t borders;
    uint8_t* layer_origin;
    uint8_t* layer_data;
};

struct pyramide_t {
    int num_levels;
    pyramide_level_t levels[MAX_NUM_PYRAMIDE_LEVELS];
};

void create_pyramide(pyramide_t& pyr, int width, int height, int num_levels, int border_width = 16);
void calculate_pyramide(pyramide_t& pyr, uint8_t* plane, int stride);
