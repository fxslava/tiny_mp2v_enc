// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include "decoder.h"
#include "scan.h"
#include "idct.h"

frame_c::frame_c(int width, int height, int chroma_format) {
    m_stride = (width + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
    m_width = width;
    m_height = height;

    switch (chroma_format) {
    case chroma_format_420:
        m_chroma_stride = ((m_stride >> 1) + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
        m_chroma_width = m_width >> 1;
        m_chroma_height = m_height >> 1;
        break;
    case chroma_format_422:
        m_chroma_stride = ((m_stride >> 1) + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
        m_chroma_width = m_width >> 1;
        m_chroma_height = m_height;
        break;
    case chroma_format_444:
        m_chroma_stride = m_stride;
        m_chroma_width = m_width;
        m_chroma_height = m_height;
        break;
    }

    planes[0] = new uint8_t[m_height * m_stride];
    planes[1] = new uint8_t[m_chroma_height * m_chroma_stride];
    planes[2] = new uint8_t[m_chroma_height * m_chroma_stride];
}

frame_c::~frame_c() {
    for (int i = 0; i < 3; i++)
        if (planes[i]) delete[] planes[i];
}

bool mp2v_decoder_c::decoder_init(decoder_config_t* config) {
    int pool_size = config->frames_pool_size;
    int width = config->width;
    int height = config->height;
    int chroma_format = config->chroma_format;

    for (int i = 0; i < pool_size; i++)
        frames_pool.push_back(new frame_c(width, height, chroma_format));

    return true;
}
bool mp2v_decoder_c::decode() {
    return video_sequence_decoder.decode();
}

bool mp2v_sequence_decoder_c::decode() {
    return parse();
}

bool mp2v_sequence_decoder_c::parse_picture_data() {
    frame_c* frame = m_owner->frames_pool.back();

    /* Decode sequence parameters*/
    mp2v_decoded_picture_c pic(m_bs, this, frame);
    pic.block_count = block_count_tbl[m_sequence_extension.chroma_format];

    parse_picture_header(m_bs, pic.m_picture_header);
    parse_picture_coding_extension(m_bs, pic.m_picture_coding_extension);
    parse_extension_and_user_data(after_picture_coding_extension, &pic);
    pic.decode();

    m_owner->frames_pool.push_back(frame);
    m_owner->output_frames.push(frame);
    return true;
}

bool mp2v_decoded_picture_c::decode() {
    if (m_quant_matrix_extension) {
        for (int i = 0; i < 64; i++) {
            if (m_quant_matrix_extension->load_intra_quantiser_matrix)
                quantiser_matrices[0][i] = m_quant_matrix_extension->intra_quantiser_matrix[g_scan[0][i]];
            if (m_quant_matrix_extension->load_non_intra_quantiser_matrix)
                quantiser_matrices[1][i] = m_quant_matrix_extension->non_intra_quantiser_matrix[g_scan[0][i]];
            if (m_quant_matrix_extension->load_intra_quantiser_matrix)
                quantiser_matrices[2][i] = m_quant_matrix_extension->chroma_intra_quantiser_matrix[g_scan[0][i]];
            if (m_quant_matrix_extension->load_intra_quantiser_matrix)
                quantiser_matrices[3][i] = m_quant_matrix_extension->chroma_non_intra_quantiser_matrix[g_scan[0][i]];
        }
    }
    auto &sext = m_sequence->m_sequence_extension;
    decode_macroblock_func = select_decode_macroblock(sext.chroma_format, m_picture_coding_extension.q_scale_type, m_picture_coding_extension.alternate_scan);
    return parse_picture();
}

bool mp2v_decoded_picture_c::parse_slice() {
    mp2v_decoded_slice_c slice(m_bs, this, decode_macroblock_func, m_frame);
    slice.init_slice();
    slice.parse_slice();
    m_slices.push_back(slice);
    return true;
}

bool mp2v_decoded_slice_c::decode_slice() {
    int slice_vertical_position = slice.slice_start_code & 0xff;
    if (vertical_size_value > 2800)
        mb_row = (slice.slice_vertical_position_extension << 7) + slice_vertical_position - 1;
    else
        mb_row = slice_vertical_position - 1;
    mb_col = 0;
    cur_quantiser_scale_code = slice.quantiser_scale_code;

    return true;
}

bool mp2v_decoded_slice_c::decode_blocks(mb_data_t& mb_data) {

    mp2v_decoded_picture_c* pic = reinterpret_cast<mp2v_decoded_picture_c*>(m_pic);
    auto& pcext = m_pic->m_picture_coding_extension;

    uint8_t* yuv[3];
    int stride = m_frame->m_stride;
    int chroma_stride = m_frame->m_chroma_stride;
    yuv[0] = m_frame->planes[0] + mb_row * 16 * stride + mb_col * 16;
    switch (chroma_format) {
    case chroma_format_420:
        yuv[1] = m_frame->planes[1] + mb_row * 8 * chroma_stride + mb_col * 8;
        yuv[2] = m_frame->planes[2] + mb_row * 8 * chroma_stride + mb_col * 8;
        break;
    case chroma_format_422:
        yuv[1] = m_frame->planes[1] + mb_row * 16 * chroma_stride + mb_col * 8;
        yuv[2] = m_frame->planes[2] + mb_row * 16 * chroma_stride + mb_col * 8;
        break;
    case chroma_format_444:
        yuv[1] = m_frame->planes[1] + mb_row * 16 * chroma_stride + mb_col * 16;
        yuv[2] = m_frame->planes[2] + mb_row * 16 * chroma_stride + mb_col * 16;
        break;
    }

    decode_mb_func(yuv, stride, chroma_stride, mb_data, pic->quantiser_matrices, pcext.intra_dc_precision, cur_quantiser_scale_code);

    mb_col += mb_data.mb.macroblock_address_increment;
    macroblocks.push_back(mb_data);
    return true;
}