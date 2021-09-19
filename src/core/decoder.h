// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <vector>
#include <deque>
#include <mutex>
#include <condition_variable>

#include "common/queue.hpp"
#include "mp2v_hdr.h"
#include "api/bitstream.h"
#include "mb_decoder.h"

constexpr int CACHE_LINE = 64;

class mp2v_picture_c;
class mp2v_decoder_c;

enum extension_after_code_e {
    after_sequence_extension = 0,
    after_group_of_picture_header,
    after_picture_coding_extension
};

struct decoder_config_t {
    int width;
    int height;
    int chroma_format;
    int frames_pool_size;
};

class frame_c {
    friend class mp2v_slice_c;
public:
    frame_c(int width, int height, int chroma_format);
    ~frame_c();

    uint8_t* get_planes (int plane_idx) { return m_planes[plane_idx]; }
    int      get_strides(int plane_idx) { return m_stride[plane_idx]; }
    int      get_width  (int plane_idx) { return m_width [plane_idx]; }
    int      get_height (int plane_idx) { return m_height[plane_idx]; }
private:
    uint32_t m_width [3] = { 0 };
    uint32_t m_height[3] = { 0 };
    uint32_t m_stride[3] = { 0 };
    uint8_t* m_planes[3] = { 0 };
};

class mp2v_slice_c {
public:
    mp2v_slice_c(bitstream_reader_c* bitstream, mp2v_picture_c* pic, frame_c* frame);
    bool decode_slice();

private:
    uint32_t m_spatial_temporal_weight_code_table_index = 0;
    uint32_t m_vertical_size_value = 0;
    uint32_t m_chroma_format = 0;
    uint32_t m_f_code[2][2] = { { 0 } };
    int16_t  m_PMV[2][2][2] = { { 0 } };
    uint32_t m_intra_vlc_format = 0;
    uint32_t m_picture_structure = 0;
    uint32_t m_picture_coding_type = 0;
    uint32_t m_concealment_motion_vectors = 0;
    uint16_t m_dct_dc_pred_reset_value = 0;
    uint16_t m_dct_dc_pred[3] = { 0 };
    uint16_t m_block_count = 0;
    bitstream_reader_c* m_bs = nullptr;
    mp2v_picture_c* m_pic = nullptr;

    frame_c* m_frame;
    int cur_quantiser_scale_code = 0;
    int mb_row = 0;

public:
    slice_t m_slice = { 0 };

#ifdef _DEBUG
    std::vector<mb_data_t> m_macroblocks;
#endif
};

class mp2v_picture_c {
    friend class mp2v_slice_c;
public:
    mp2v_picture_c(bitstream_reader_c* bitstream, mp2v_decoder_c* decoder, frame_c* frame) : m_bs(bitstream), m_dec(decoder), m_frame(frame) {};
    bool decode_picture();

private:
    bitstream_reader_c* m_bs;
    mp2v_decoder_c* m_dec;
    uint16_t quantiser_matrices[4][64];
    parse_macroblock_func_t m_parse_macroblock_func = nullptr;
    frame_c* m_frame;
public:
    // headers
    picture_header_t m_picture_header = { 0 }; //mandatory
    picture_coding_extension_t m_picture_coding_extension = { 0 }; //mandatory
    quant_matrix_extension_t* m_quant_matrix_extension = nullptr;
    copyright_extension_t* m_copyright_extension = nullptr;
    picture_display_extension_t* m_picture_display_extension = nullptr;
    picture_spatial_scalable_extension_t* m_picture_spatial_scalable_extension = nullptr;
    picture_temporal_scalable_extension_t* m_picture_temporal_scalable_extension = nullptr;
    int block_count = 0;

#ifdef _DEBUG
    std::vector<mp2v_slice_c> m_slices;
    void dump_mvs(const char* dump_filename);
#endif
};

class mp2v_decoder_c {
    friend class mp2v_slice_c;
public:
    mp2v_decoder_c(bitstream_reader_c* bitstream) : m_bs(bitstream), m_frames_pool(16), m_output_frames(16) {};
    bool decoder_init(decoder_config_t* config);
    bool decode();
    bool decode_user_data();
    bool decode_extension_and_user_data(extension_after_code_e after_code, mp2v_picture_c* pic);
    bool decode_extension_data(extension_after_code_e after_code, mp2v_picture_c* pic);
    bool decode_picture_data();

    void get_decoded_frame(frame_c*& frame) {
        m_output_frames.pop(frame);
    }
    void release_frame(frame_c* frame) {
        m_frames_pool.push(frame);
    }
    void push_frame(frame_c* frame) {
        m_output_frames.push(frame);
    }
protected:
    bitstream_reader_c* m_bs;

    // stream data
    frame_c* ref_frames[2] = { 0 };
    ThreadSafeQ<frame_c*> m_frames_pool;
    ThreadSafeQ<frame_c*> m_output_frames;

public:
    std::vector<uint8_t> user_data;
    // headers
    sequence_header_t m_sequence_header = { 0 }; //mandatory
    sequence_extension_t m_sequence_extension = { 0 }; //mandatory
    sequence_display_extension_t* m_sequence_display_extension = nullptr;
    sequence_scalable_extension_t* m_sequence_scalable_extension = nullptr;
    group_of_pictures_header_t* m_group_of_pictures_header = nullptr;
};

extern uint32_t block_count_tbl[4];