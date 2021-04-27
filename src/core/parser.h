// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <vector>
#include "mp2v_hdr.h"
#include "api/bitstream.h"

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

struct parsed_context_t {
    sequence_header_t* sequence_header;
    sequence_extension_t* sequence_extension;
    sequence_display_extension_t* sequence_display_extension;
    sequence_scalable_extension_t* sequence_scalable_extension;
    group_of_pictures_header_t* group_of_pictures_header;
    picture_header_t* picture_header;
    picture_coding_extension_t* picture_coding_extension;
    quant_matrix_extension_t* quant_matrix_extension;
    picture_display_extension_t* picture_display_extension;
    picture_temporal_scalable_extension_t* picture_temporal_scalable_extension;
    picture_spatial_scalable_extension_t* picture_spatial_scalable_extension;
    copyright_extension_t* copyright_extension;
    int block_count;
};

class picture_c;

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
};

class slice_c {
public:
    slice_t slice;
public:
    slice_c(bitstream_reader_i* bitstream, picture_c* pic) : m_bs(bitstream), m_pic(pic), slice{0} {};
    bool init_slice();
    bool parse_slice();
    bool parse_modes(macroblock_t &mb);
    bool parse_coded_block_pattern(macroblock_t& mb);
    bool parse_macroblock();
    bool parse_motion_vectors(mb_data_t& mb, int s);
    bool parse_motion_vector(mb_data_t& mb_data, int r, int s);
    bool parse_block(mb_data_t& mb_data, int i);

    void decode_mb_modes(mb_data_t mb_data);
    void decode_mb_pattern(mb_data_t mb_data);
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
    bool use_dct_one_table = false;
private:
    bitstream_reader_i* m_bs;
    picture_c* m_pic;
    std::vector<mb_data_t> macroblocks;
};

class picture_c {
    friend slice_c;
public:
    picture_c(bitstream_reader_i* bitstream, parsed_context_t& ctx) : m_bs(bitstream), m_ctx(ctx) {};
    bool parse_picture();
private:
    bitstream_reader_i* m_bs;
    parsed_context_t m_ctx;
    std::vector<slice_c> m_slices;
};

class video_sequence_c {
public:
    std::vector<uint8_t> user_data;

public:
    video_sequence_c(bitstream_reader_i* bitstream) : m_bs(bitstream) {};
    bool parse();
    bool parse_user_data();
    bool parse_sequence_header();
    bool parse_sequence_extension();
    bool parse_sequence_display_extension();
    bool parse_sequence_scalable_extension();
    bool parse_group_of_pictures_header();
    bool parse_picture_header();
    bool parse_picture_coding_extension();
    bool parse_quant_matrix_extension();
    bool parse_picture_display_extension();
    bool parse_picture_temporal_scalable_extension();
    bool parse_picture_spatial_scalable_extension();
    bool parse_copyright_extension();
    bool parse_extension_and_user_data(extension_after_code_e after_code);
    bool parse_extension_data(extension_after_code_e after_code);
    bool parse_picture_data();

private:
    bitstream_reader_i* m_bs;
    // headers
    std::vector<sequence_header_t> m_sequence_header;
    std::vector<sequence_extension_t> m_sequence_extension;
    std::vector<sequence_display_extension_t> m_sequence_display_extension;
    std::vector<sequence_scalable_extension_t> m_sequence_scalable_extension;
    std::vector<group_of_pictures_header_t> m_group_of_pictures_header;
    std::vector<picture_header_t> m_picture_header;
    std::vector<picture_coding_extension_t> m_picture_coding_extension;
    std::vector<quant_matrix_extension_t> m_quant_matrix_extension;
    std::vector<picture_display_extension_t> m_picture_display_extension;
    std::vector<picture_temporal_scalable_extension_t> m_picture_temporal_scalable_extension;
    std::vector<picture_spatial_scalable_extension_t> m_picture_spatial_scalable_extension;
    std::vector<copyright_extension_t> m_copyright_extension;
    // stream data
    std::vector<picture_c> m_pictures;
};

class mp2v_parser_c {
public:
    mp2v_parser_c(bitstream_reader_i* bitstream) : 
        m_bs(bitstream), 
        m_video_sequence(bitstream) {};
    bool parse();

private:
    bitstream_reader_i* m_bs;
    video_sequence_c m_video_sequence;
};