// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <vector>
#include "mp2v_hdr.h"
#include "api/bitstream.h"

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
};

class picture_c;

class slice_c {
public:
    slice_t slice;
public:
    slice_c(bitstream_reader_i* bitstream, picture_c* pic) : m_bs(bitstream), m_pic(pic), slice{0} {};
    bool parse_slice();
    bool parse_modes();
    bool parse_macroblock();
private:
    bitstream_reader_i* m_bs;
    picture_c* m_pic;
    std::vector<macroblock_t> mb_data;
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