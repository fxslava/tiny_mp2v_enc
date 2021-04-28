// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "parser.h"
#include <vector>

constexpr int CACHE_LINE = 64;

struct decoder_config_t {
    int width;
    int height;
    int chroma_format;
    int frames_pool_size;
};

class frame_c {
    friend class mp2v_decoder_c;
public:
    frame_c(int width, int height, int chroma_format);
    ~frame_c();
private:
    int m_width = 0;
    int m_height = 0;
    int m_stride = 0;
    int m_chroma_stride = 0;
    int m_chroma_width = 0;
    int m_chroma_height = 0;
    uint8_t* luma_plane = nullptr;
    uint8_t* chroma_planes[2] = { 0 };
};

class mp2v_decoder_c {
public:
    bool decoder_init(decoder_config_t* config);
    bool decode_slice(slice_c& slice);
    bool decode(picture_c* pic);

private:
    std::vector<frame_c*> frames_pool;
};