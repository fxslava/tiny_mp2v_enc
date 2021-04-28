// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <vector>
#include <concurrent_queue.h>
#include "mp2v_hdr.h"
#include "api/bitstream.h"

using namespace concurrency;

enum mv_format_e {
    Field = 0,
    Frame
};

enum prediction_type_e {
    Field_based,
    Frame_based,
    Dual_Prime,
    MC16x8
};

enum extension_after_code_e {
    after_sequence_extension = 0,
    after_group_of_picture_header,
    after_picture_coding_extension
};

class picture_c;
class video_sequence_c;

struct mb_data_t {
    macroblock_t mb;
    // decoded parameters
    uint32_t spatial_temporal_weight_class;
    uint32_t spatial_temporal_integer_weight;
    uint32_t spatial_temporal_weight_fract[2];
    uint32_t motion_vector_count;
    uint32_t dmv;
    mv_format_e mv_format;
    prediction_type_e prediction_type;
    bool pattern_code[12];
    int16_t QFS[64];
};

class slice_c {
    friend class mp2v_decoder_c;
public:
    slice_t slice;
public:
    slice_c(bitstream_reader_i* bitstream, picture_c* pic) : m_bs(bitstream), m_pic(pic), slice{0} {};
    bool init_slice();
    bool parse_modes(macroblock_t &mb);
    bool parse_coded_block_pattern(macroblock_t& mb);
    bool parse_motion_vectors(mb_data_t& mb, int s);
    bool parse_motion_vector(mb_data_t& mb_data, int r, int s);
    bool parse_slice();
    bool parse_macroblock();

    void decode_mb_modes(mb_data_t& mb_data);
    void decode_mb_pattern(mb_data_t& mb_data);
private:
    // local context from headers
    uint32_t vertical_size_value = 0;
    uint32_t picture_coding_type = 0;
    uint32_t spatial_temporal_weight_code_table_index = 0;
    uint32_t picture_structure = 0;
    uint32_t f_code[2][2] = { { 0 } };
    uint32_t concealment_motion_vectors = 0;
    uint32_t chroma_format = 0;
    uint32_t frame_pred_frame_dct = 0;
    uint32_t intra_vlc_format = 0;
    bool m_use_dct_one_table = false;
    int block_count = 0;
    uint16_t dct_dc_pred_reset_value = 0;
    uint16_t dct_dc_pred[3];
private:
    bitstream_reader_i* m_bs;
    picture_c* m_pic;
    std::vector<mb_data_t> macroblocks;
};

class picture_c {
    friend class slice_c;
    friend class mp2v_decoder_c;
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

public:
    picture_c(bitstream_reader_i* bitstream, video_sequence_c* sequence) : m_bs(bitstream), m_sequence(sequence) {};
    bool parse_picture();
    video_sequence_c* get_seq() { return m_sequence; }

private:
    bitstream_reader_i* m_bs;
    std::vector<slice_c> m_slices;
    video_sequence_c* m_sequence;
};

class video_sequence_c {
public:
    std::vector<uint8_t> user_data;
public:
    // headers
    sequence_header_t m_sequence_header = { 0 }; //mandatory
    sequence_extension_t m_sequence_extension = { 0 }; //mandatory
    sequence_display_extension_t* m_sequence_display_extension = nullptr;
    sequence_scalable_extension_t* m_sequence_scalable_extension = nullptr;
    group_of_pictures_header_t* m_group_of_pictures_header = nullptr;

public:
    video_sequence_c(bitstream_reader_i* bitstream) : m_bs(bitstream) {};
    bool parse();
    bool parse_user_data();
    bool parse_extension_and_user_data(extension_after_code_e after_code, picture_c* pic);
    bool parse_extension_data(extension_after_code_e after_code, picture_c* pic);
    bool parse_picture_data();

    bool get_parsed_picture(picture_c*& pic) {
        return m_pictures_queue.try_pop(pic);
    }
private:
    bitstream_reader_i* m_bs;

    // stream data
    std::vector<picture_c> m_pictures;
    concurrent_queue<picture_c*> m_pictures_queue;
};

class mp2v_parser_c {
public:
    mp2v_parser_c(bitstream_reader_i* bitstream) : 
        m_bs(bitstream), 
        m_video_sequence(bitstream) {};
    bool parse();
    bool get_parsed_picture(picture_c*& pic) {
        return m_video_sequence.get_parsed_picture(pic);
    }

private:
    bitstream_reader_i* m_bs;
    video_sequence_c m_video_sequence;
};