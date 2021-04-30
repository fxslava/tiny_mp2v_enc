// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "parser.h"
#include <vector>
#include <concurrent_queue.h>
#include "mb_decoder.h"

using namespace concurrency;

constexpr int CACHE_LINE = 64;

struct decoder_config_t {
    int width;
    int height;
    int chroma_format;
    int frames_pool_size;
};

class frame_c {
    friend class mp2v_decoder_c;
    friend class mp2v_decoded_slice_c;
public:
    frame_c(int width, int height, int chroma_format);
    ~frame_c();

    uint8_t* get_planes (int plane_idx) { return planes[plane_idx]; }
    int      get_strides(int plane_idx) { 
        switch (plane_idx) {
        case 0: return m_stride;
        case 1: return m_chroma_stride;
        case 2: return m_chroma_stride;
        };
    }
    int      get_width  (int plane_idx) {
        switch (plane_idx) {
        case 0: return m_width;
        case 1: return m_chroma_width;
        case 2: return m_chroma_width;
        };
    }
    int      get_height (int plane_idx) {
        switch (plane_idx) {
        case 0: return m_height;
        case 1: return m_chroma_height;
        case 2: return m_chroma_height;
        };
    }
private:
    int m_width = 0;
    int m_height = 0;
    int m_stride = 0;
    int m_chroma_stride = 0;
    int m_chroma_width = 0;
    int m_chroma_height = 0;
    uint8_t* planes[3] = { 0 };
};

class mp2v_decoder_c;

class mp2v_sequence_decoder_c : public video_sequence_c {
public:
    mp2v_sequence_decoder_c(bitstream_reader_i* bitstream, mp2v_decoder_c* owner) : video_sequence_c(bitstream), m_owner(owner){};
    bool decode();
    virtual bool parse_picture_data();
private:
    mp2v_decoder_c* m_owner;
};

class mp2v_decoded_picture_c : public mp2v_picture_c {
    friend class mp2v_decoded_slice_c;
public:
    mp2v_decoded_picture_c(bitstream_reader_i* bitstream, mp2v_sequence_decoder_c* sequence, frame_c* frame) : mp2v_picture_c(bitstream, sequence), m_frame(frame){};
    bool decode();
    bool parse_slice();

private:
    uint16_t quantiser_matrices[4][64];

private:
    decode_macroblock_func_t decode_macroblock_func;
    frame_c* m_frame;
};

class mp2v_decoded_slice_c : public mp2v_slice_c {
public:
    mp2v_decoded_slice_c(bitstream_reader_i* bitstream, mp2v_decoded_picture_c* pic, decode_macroblock_func_t dec_mb_func, frame_c* frame) :
        mp2v_slice_c(bitstream, pic), decode_mb_func(dec_mb_func), m_frame(frame){};
    bool decode_slice();
    bool decode_blocks(mb_data_t& mb_data);

private:
    decode_macroblock_func_t decode_mb_func;
    frame_c* m_frame;
    int cur_quantiser_scale_code = 0;
    int mb_row = 0;
    int mb_col = 0;
};

class mp2v_decoder_c {
    friend mp2v_sequence_decoder_c;
public:
    mp2v_decoder_c(bitstream_reader_i* bitstream) : video_sequence_decoder(bitstream, this) {}
    bool decoder_init(decoder_config_t* config);
    bool decode();
    bool get_decoded_frame(frame_c*& frame) {
        return output_frames.try_pop(frame);
    }

private:
    mp2v_sequence_decoder_c video_sequence_decoder;
    std::vector<frame_c*> frames_pool;
    concurrent_queue<frame_c*> output_frames;
};