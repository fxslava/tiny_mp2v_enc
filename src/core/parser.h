// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <vector>
#include <concurrent_queue.h>
#include "mp2v_hdr.h"
#include "api/bitstream.h"

using namespace concurrency;

enum extension_after_code_e {
    after_sequence_extension = 0,
    after_group_of_picture_header,
    after_picture_coding_extension
};

class mp2v_picture_c;
class video_sequence_c;

struct mb_data_t {
    macroblock_t mb;
    bool pattern_code[12];
    int16_t QFS[12][64];
};

class mp2v_slice_c {
    friend class mp2v_decoded_slice_c;
public:
    mp2v_slice_c(bitstream_reader_i* bitstream, mp2v_picture_c* pic);
    bool parse_slice();
    bool parse_macroblock();
    virtual bool on_decode_slice();
    virtual bool on_decode_macroblock(mb_data_t& mb_data);

private:
    void reset_dct_dc_predictors();

private:
    uint32_t m_spatial_temporal_weight_code_table_index = 0;
    uint32_t m_vertical_size_value = 0;
    uint32_t m_chroma_format = 0;
    uint32_t m_f_code[2][2] = { { 0 } };
    uint32_t m_intra_vlc_format = 0;
    uint16_t m_dct_dc_pred_reset_value = 0;
    uint16_t m_dct_dc_pred[3] = { 0 };
    uint16_t m_block_count = 0;
    parse_macroblock_func_t m_parse_macroblock_func = nullptr;
    bitstream_reader_i* m_bs = nullptr;
    mp2v_picture_c* m_pic = nullptr;
    std::vector<mb_data_t>  m_macroblocks;

public:
    slice_t m_slice = { 0 };
};

class mp2v_picture_c {
    friend class mp2v_slice_c;
public:
    mp2v_picture_c(bitstream_reader_i* bitstream, video_sequence_c* sequence) : m_bs(bitstream), m_sequence(sequence) {};
    bool parse_picture();

protected:
    bitstream_reader_i* m_bs;
    std::vector<mp2v_slice_c> m_slices;
    video_sequence_c* m_sequence;

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
};

class video_sequence_c {
public:
    video_sequence_c(bitstream_reader_i* bitstream) : m_bs(bitstream) {};
    bool parse();
    bool parse_user_data();
    bool parse_extension_and_user_data(extension_after_code_e after_code, mp2v_picture_c* pic);
    bool parse_extension_data(extension_after_code_e after_code, mp2v_picture_c* pic);
    virtual bool parse_picture_data();

    bool get_parsed_picture(mp2v_picture_c*& pic) {
        return m_pictures_queue.try_pop(pic);
    }
protected:
    bitstream_reader_i* m_bs;

    // stream data
    std::vector<mp2v_picture_c> m_pictures;
    concurrent_queue<mp2v_picture_c*> m_pictures_queue;

public:
    std::vector<uint8_t> user_data;
    // headers
    sequence_header_t m_sequence_header = { 0 }; //mandatory
    sequence_extension_t m_sequence_extension = { 0 }; //mandatory
    sequence_display_extension_t* m_sequence_display_extension = nullptr;
    sequence_scalable_extension_t* m_sequence_scalable_extension = nullptr;
    group_of_pictures_header_t* m_group_of_pictures_header = nullptr;
};

class mp2v_parser_c {
public:
    mp2v_parser_c(bitstream_reader_i* bitstream) : 
        m_bs(bitstream), 
        m_video_sequence(bitstream) {};
    bool parse();
    bool get_parsed_picture(mp2v_picture_c*& pic) {
        return m_video_sequence.get_parsed_picture(pic);
    }

private:
    bitstream_reader_i* m_bs;
    video_sequence_c m_video_sequence;
};

extern uint32_t block_count_tbl[4];