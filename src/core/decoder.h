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

class mp2v_sequence_decoder_c : public video_sequence_c {
public:
    mp2v_sequence_decoder_c(bitstream_reader_i* bitstream) : video_sequence_c(bitstream) {};
    bool decode();
    virtual bool parse_picture_data();

private:
    std::vector<frame_c*> frames_pool;
};

class mp2v_decoded_picture_c : public mp2v_picture_c {
public:
    mp2v_decoded_picture_c(bitstream_reader_i* bitstream, mp2v_sequence_decoder_c* sequence) : mp2v_picture_c(bitstream, sequence) {};
    bool decode();
    bool parse_slice();
};

class mp2v_decoded_slice_c : public mp2v_slice_c {
public:
    mp2v_decoded_slice_c(bitstream_reader_i* bitstream, mp2v_decoded_picture_c* pic) : mp2v_slice_c(bitstream, pic) {};
    bool decode();
    bool decode_blocks(mb_data_t& mb_data);
};

class mp2v_decoder_c {
public:
    mp2v_decoder_c(bitstream_reader_i* bitstream) : video_sequence_decoder(bitstream) {}
    bool decoder_init(decoder_config_t* config);
    bool decode();

private:
    mp2v_sequence_decoder_c video_sequence_decoder;
    std::vector<frame_c*> frames_pool;
};