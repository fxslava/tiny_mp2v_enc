// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#include <string.h>
#include "mb_decoder.h"
#include "decoder.h"
#include "mp2v_hdr.h"
#include "mp2v_vlc.h"
#include "misc.hpp"

#define CHECK(p) { if (!(p)) return false; }

//[chroma_format]
uint32_t block_count_tbl[4] = { 0 /*invalid chroma format*/, 6, 8, 12 };

struct spatial_temporal_weights_classes_t {
    uint8_t spatial_temporal_weight_fract[2]; // 0 - 0.0, 1 - 0.5, 2 - 1.0
    uint8_t spatial_temporal_weight_class;
    uint8_t spatial_temporal_integer_weight;
};

//[spatial_temporal_weight_code_table_index][spatial_temporal_weight_code]
spatial_temporal_weights_classes_t local_spatial_temporal_weights_classes_tbl[4][4] = {
    {{{1, 1}, 1, 0}, {{1, 1}, 1, 0}, {{1, 1}, 1, 0}, {{1, 1}, 1, 0} },
    {{{0, 2}, 3, 1}, {{0, 1}, 1, 0}, {{1, 2}, 3, 0}, {{1, 1}, 1, 0} },
    {{{2, 0}, 2, 1}, {{1, 0}, 1, 0}, {{2, 1}, 2, 0}, {{1, 1}, 1, 0} },
    {{{2, 0}, 2, 1}, {{2, 1}, 2, 0}, {{1, 2}, 3, 0}, {{1, 1}, 1, 0} }
};

uint8_t color_component_index[12] = { 0, 0, 0, 0, 1, 2, 1, 2, 1, 2, 1, 2 };

frame_c::frame_c(int width, int height, int chroma_format) {
    m_stride[0] = (width + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
    m_width [0] = width;
    m_height[0] = height;

    switch (chroma_format) {
    case chroma_format_420:
        m_stride[1] = ((m_stride[0] >> 1) + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
        m_width [1] = m_width[0] >> 1;
        m_height[1] = m_height[0] >> 1;
        break;
    case chroma_format_422:
        m_stride[1] = ((m_stride[0] >> 1) + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
        m_width [1] = m_width[0] >> 1;
        m_height[1] = m_height[0];
        break;
    case chroma_format_444:
        m_stride[1] = m_stride[0];
        m_width [1] = m_width [0];
        m_height[1] = m_height[0];
        break;
    }
    m_stride[2] = m_stride[1];
    m_width [2] = m_width [1];
    m_height[2] = m_height[1];

    for (int i = 0; i < 3; i++)
        m_planes[i] = (uint8_t*)_aligned_malloc(m_height[i] * m_stride[i], 32);
}

frame_c::~frame_c() {
    for (int i = 0; i < 3; i++)
        if (m_planes[i]) _aligned_free(m_planes[i]);
}

mp2v_slice_c::mp2v_slice_c(bitstream_reader_c* bitstream, mp2v_picture_c* pic, frame_c* frame) :
    m_bs(bitstream), m_pic(pic), m_frame(frame), m_slice{ 0 }
{
    // headers
    auto* seq = m_pic->m_dec;
    auto& pcext = m_pic->m_picture_coding_extension;
    auto* pssext = m_pic->m_picture_spatial_scalable_extension;
    auto& ph = m_pic->m_picture_header;
    auto& se = seq->m_sequence_extension;
    auto& sh = seq->m_sequence_header;

    // copy useful parameters from headers
    m_f_code[0][0] = pcext.f_code[0][0];
    m_f_code[0][1] = pcext.f_code[0][1];
    m_f_code[1][0] = pcext.f_code[1][0];
    m_f_code[1][1] = pcext.f_code[1][1];
    m_picture_structure = pcext.picture_structure;
    m_picture_coding_type = ph.picture_coding_type;
    m_concealment_motion_vectors = pcext.concealment_motion_vectors;
    m_chroma_format = se.chroma_format;
    m_vertical_size_value = sh.vertical_size_value;
    m_intra_vlc_format = pcext.intra_vlc_format;
    m_block_count = m_pic->block_count;
    m_dct_dc_pred_reset_value = predictor_reset_value[pcext.intra_dc_precision];

    // if picture spatial scalable extension exist then store temporal weight code table index
    if (pssext)
        m_spatial_temporal_weight_code_table_index = pssext->spatial_temporal_weight_code_table_index;
}

static void make_macroblock_yuv_ptrs(uint8_t* (&yuv)[3], frame_c* frame, int mb_row, int stride, int chroma_stride, int chroma_format) {
    yuv[0] = frame->get_planes(0) + mb_row * 16 * stride;// +mb_col * 16;
    switch (chroma_format) {
    case chroma_format_420:
        yuv[1] = frame->get_planes(1) + mb_row * 8 * chroma_stride;// +mb_col * 8;
        yuv[2] = frame->get_planes(2) + mb_row * 8 * chroma_stride;// +mb_col * 8;
        break;
    case chroma_format_422:
        yuv[1] = frame->get_planes(1) + mb_row * 16 * chroma_stride;// +mb_col * 8;
        yuv[2] = frame->get_planes(2) + mb_row * 16 * chroma_stride;// +mb_col * 8;
        break;
    case chroma_format_444:
        yuv[1] = frame->get_planes(1) + mb_row * 16 * chroma_stride;// +mb_col * 16;
        yuv[2] = frame->get_planes(2) + mb_row * 16 * chroma_stride;// +mb_col * 16;
        break;
    }
}

bool mp2v_slice_c::decode_slice() {
    auto* seq = m_pic->m_dec;

    // decode slice header
    m_slice.slice_start_code = m_bs->read_next_bits(32);
    if (m_vertical_size_value > 2800)
        m_slice.slice_vertical_position_extension = m_bs->read_next_bits(3);
    if (seq->m_sequence_scalable_extension && seq->m_sequence_scalable_extension->scalable_mode == scalable_mode_data_partitioning)
        m_slice.priority_breakpoint = m_bs->read_next_bits(7);
    m_slice.quantiser_scale_code = m_bs->read_next_bits(5);
    if (m_bs->get_next_bits(1) == 1) {
        m_slice.slice_extension_flag = m_bs->read_next_bits(1);
        m_slice.intra_slice = m_bs->read_next_bits(1);
        m_slice.slice_picture_id_enable = m_bs->read_next_bits(1);
        m_slice.slice_picture_id = m_bs->read_next_bits(6);
        while (m_bs->get_next_bits(1) == 1) {
            m_bs->skip_bits(9);
        }
    }

    // calculate row position of the slice
    int mb_row = 0;
    int slice_vertical_position = m_slice.slice_start_code & 0xff;
    if (m_vertical_size_value > 2800)
        mb_row = (m_slice.slice_vertical_position_extension << 7) + slice_vertical_position - 1;
    else
        mb_row = slice_vertical_position - 1;

    // fill cache
    auto refs = m_pic->m_dec->ref_frames;
    macroblock_context_cache_t cache;
    memcpy(cache.W, m_pic->quantiser_matrices, sizeof(cache.W));
    memcpy(cache.f_code, m_f_code, sizeof(cache.f_code));
    memset(cache.PMVs, 0, sizeof(cache.PMVs));
    for (auto& pred : cache.dct_dc_pred) pred = m_dct_dc_pred_reset_value;
    cache.spatial_temporal_weight_code_table_index = 0;
    cache.luma_stride      = m_frame->m_stride[0];
    cache.chroma_stride    = m_frame->m_stride[1];
    cache.intra_dc_prec    = m_pic->m_picture_coding_extension.intra_dc_precision;
    cache.intra_vlc_format = m_intra_vlc_format;
    cache.previous_mb_type = 0;
                 make_macroblock_yuv_ptrs(cache.yuv_planes[REF_TYPE_SRC], m_frame, mb_row, cache.luma_stride, cache.chroma_stride, m_chroma_format);
    if (refs[0]) make_macroblock_yuv_ptrs(cache.yuv_planes[REF_TYPE_L0 ], refs[0], mb_row, cache.luma_stride, cache.chroma_stride, m_chroma_format);
    if (refs[1]) make_macroblock_yuv_ptrs(cache.yuv_planes[REF_TYPE_L1 ], refs[1], mb_row, cache.luma_stride, cache.chroma_stride, m_chroma_format);
    if (m_pic->m_picture_coding_extension.q_scale_type) {
        if (m_slice.quantiser_scale_code < 9)       cache.quantiser_scale =  m_slice.quantiser_scale_code;
        else if (m_slice.quantiser_scale_code < 17) cache.quantiser_scale = (m_slice.quantiser_scale_code - 4) << 1;
        else if (m_slice.quantiser_scale_code < 25) cache.quantiser_scale = (m_slice.quantiser_scale_code - 10) << 2;
        else                                        cache.quantiser_scale = (m_slice.quantiser_scale_code - 17) << 3; }
    else                                            cache.quantiser_scale =  m_slice.quantiser_scale_code << 1;

    // decode macroblocks
    m_bs->skip_bits(1); /* with the value '0' */
    do {
        m_pic->m_parse_macroblock_func(m_bs, cache);

#ifdef _DEBUG
        m_macroblocks.push_back(cache.mb);
#endif
    } while (m_bs->get_next_bits(23) != 0);
    local_find_start_code(m_bs);
    return true;
}

bool mp2v_picture_c::decode_picture() {
    if (m_quant_matrix_extension) {
        for (int i = 0; i < 64; i++) {
            if (m_quant_matrix_extension->load_intra_quantiser_matrix)
                quantiser_matrices[0][g_scan_trans[0][i]] = m_quant_matrix_extension->intra_quantiser_matrix[i];
            if (m_quant_matrix_extension->load_non_intra_quantiser_matrix)
                quantiser_matrices[1][g_scan_trans[0][i]] = m_quant_matrix_extension->non_intra_quantiser_matrix[i];
            if (m_quant_matrix_extension->load_chroma_intra_quantiser_matrix)
                quantiser_matrices[2][g_scan_trans[0][i]] = m_quant_matrix_extension->chroma_intra_quantiser_matrix[i];
            if (m_quant_matrix_extension->load_chroma_non_intra_quantiser_matrix)
                quantiser_matrices[3][g_scan_trans[0][i]] = m_quant_matrix_extension->chroma_non_intra_quantiser_matrix[i];
        }
    }
    auto& sext = m_dec->m_sequence_extension;
    auto& pcext = m_picture_coding_extension;
    auto& ph = m_picture_header;

    m_parse_macroblock_func = select_parse_macroblock_func(
        ph.picture_coding_type,
        pcext.picture_structure,
        pcext.frame_pred_frame_dct,
        pcext.concealment_motion_vectors,
        sext.chroma_format,
        pcext.q_scale_type,
        pcext.alternate_scan);

    do {
        mp2v_slice_c slice(m_bs, this, m_frame);
        slice.decode_slice();
#ifdef _DEBUG
        m_slices.push_back(slice);
#endif
    } while (local_next_start_code(m_bs) >= slice_start_code_min && local_next_start_code(m_bs) <= slice_start_code_max);
    local_find_start_code(m_bs);
    return true;
}

#ifdef _DEBUG
void mp2v_picture_c::dump_mvs(const char* dump_filename) {
    FILE* fp = fopen(dump_filename, "w");
    int y0 = 0;
    for (auto slice : m_slices) {
        y0 += 16;
        int x0 = 0;
        for (auto mb : slice.m_macroblocks) {
            x0 += 16 * mb.macroblock_address_increment;
            if (mb.macroblock_type & macroblock_motion_forward_bit) {
                int x1 = x0 + mb.MVs[0][0][0];
                int y1 = y0 + mb.MVs[0][0][1];
                fprintf(fp, "%d\t%d\n", x0, y0);
                fprintf(fp, "%d\t%d\tx:%d y:%d\n", x1, y1, mb.MVs[0][0][0], mb.MVs[0][0][1]);
                fprintf(fp, "\n");
            }
            if (mb.macroblock_type & macroblock_motion_backward_bit) {
                int x1 = x0 + mb.MVs[0][1][0];
                int y1 = y0 + mb.MVs[0][1][1];
                fprintf(fp, "%d\t%d\n", x0, y0);
                fprintf(fp, "%d\t%d\tx:%d y:%d\n", x1, y1, mb.MVs[0][0][0], mb.MVs[0][0][1]);
                fprintf(fp, "\n");
            }
        }
    }
    fclose(fp);
}
#endif

bool mp2v_decoder_c::decode_extension_and_user_data(extension_after_code_e after_code, mp2v_picture_c* pic) {
    while ((local_next_start_code(m_bs) == extension_start_code) || (local_next_start_code(m_bs) == user_data_start_code)) {
        if ((after_code != after_group_of_picture_header) && (local_next_start_code(m_bs) == extension_start_code))
            decode_extension_data(after_code, pic);
        if (local_next_start_code(m_bs) == user_data_start_code)
            decode_user_data();
    }
    return true;
}

bool mp2v_decoder_c::decode_user_data() {
    CHECK(local_read_start_code(m_bs) == user_data_start_code);
    while (m_bs->get_next_bits(vlc_start_code.len) != vlc_start_code.value) {
        uint8_t data = m_bs->read_next_bits(8);
        user_data.push_back(data);
    }
    local_find_start_code(m_bs);
    return true;
}

bool mp2v_decoder_c::decode_extension_data(extension_after_code_e after_code, mp2v_picture_c* pic) {
    while (local_next_start_code(m_bs) == extension_start_code) {
        m_bs->skip_bits(32); //extension_start_code 32 bslbf
        if (after_code == after_sequence_extension) {
            uint8_t ext_id = m_bs->get_next_bits(4);
            switch (ext_id)
            {
            case sequence_display_extension_id:
                parse_sequence_display_extension(m_bs, *(m_sequence_display_extension = new sequence_display_extension_t));
                break;
            case sequence_scalable_extension_id:
                parse_sequence_scalable_extension(m_bs, *(m_sequence_scalable_extension = new sequence_scalable_extension_t));
                break;
            default:
                m_bs->skip_bits(vlc_start_code.len);
                local_find_start_code(m_bs); // Unsupported extension id (skip)
                break;
            }
        }
        if (after_code == after_picture_coding_extension) {
            uint8_t ext_id = m_bs->get_next_bits(4);
            switch (ext_id)
            {
            case quant_matrix_extension_id:
                parse_quant_matrix_extension(m_bs, *(pic->m_quant_matrix_extension = new quant_matrix_extension_t));
                break;
            case copiright_extension_id:
                parse_copyright_extension(m_bs, *(pic->m_copyright_extension = new copyright_extension_t));
                break;
            case picture_display_extension_id:
                parse_picture_display_extension(m_bs, *(pic->m_picture_display_extension = new picture_display_extension_t), m_sequence_extension, pic->m_picture_coding_extension);
                break;
            case picture_spatial_scalable_extension_id:
                parse_picture_spatial_scalable_extension(m_bs, *(pic->m_picture_spatial_scalable_extension = new picture_spatial_scalable_extension_t));
                break;
            case picture_temporal_scalable_extension_id:
                parse_picture_temporal_scalable_extension(m_bs, *(pic->m_picture_temporal_scalable_extension = new picture_temporal_scalable_extension_t));
                break;
            case picture_camera_parameters_extension_id:
                //parse_camera_parameters_extension();
                break;
            default:
                m_bs->skip_bits(vlc_start_code.len);
                local_find_start_code(m_bs); // Unsupported extension id (skip)
                break;
            }
        }
    }
    return true;
}

bool mp2v_decoder_c::decode() {
    CHECK(local_next_start_code(m_bs) == sequence_header_code);
    CHECK(parse_sequence_header(m_bs, m_sequence_header));

    if (local_next_start_code(m_bs) == extension_start_code) {
        parse_sequence_extension(m_bs, m_sequence_extension);
        do {
            decode_extension_and_user_data(after_sequence_extension, nullptr);
            do {
                if (local_next_start_code(m_bs) == group_start_code) {
                    parse_group_of_pictures_header(m_bs, *(m_group_of_pictures_header = new group_of_pictures_header_t));
                    decode_extension_and_user_data(after_group_of_picture_header, nullptr);
                }
                decode_picture_data();
//#ifdef _DEBUG
                // remove this when end of stream issue was resolved
                static int pic_num = 0;
                pic_num++;
                if (pic_num > 98) {
                    push_frame(nullptr);
                    return true;
                }
//#endif
            } while ((local_next_start_code(m_bs) == picture_start_code) || (local_next_start_code(m_bs) == group_start_code));
            if (local_next_start_code(m_bs) != sequence_end_code) {
                parse_sequence_header(m_bs, m_sequence_header);
                parse_sequence_extension(m_bs, m_sequence_extension);
            }
        } while (local_next_start_code(m_bs) != sequence_end_code);
        push_frame(nullptr);
    }
    else {
        push_frame(nullptr);
        return false; // MPEG1 not support
    }

    push_frame(nullptr);
    return true;
}

bool mp2v_decoder_c::decode_picture_data() {
    frame_c* frame = nullptr;
    m_frames_pool.pop(frame);

    /* Decode sequence parameters*/
    mp2v_picture_c pic(m_bs, this, frame);
    pic.block_count = block_count_tbl[m_sequence_extension.chroma_format];

    parse_picture_header(m_bs, pic.m_picture_header);
    parse_picture_coding_extension(m_bs, pic.m_picture_coding_extension);
    decode_extension_and_user_data(after_picture_coding_extension, &pic);

    if (pic.m_picture_header.picture_coding_type == picture_coding_type_pred || pic.m_picture_header.picture_coding_type == picture_coding_type_intra) {
        ref_frames[0] = ref_frames[1];
        ref_frames[1] = frame;
    }

    pic.decode_picture();

#ifdef _DEBUG
    static int pic_num = 0;
    if (pic_num == 6)
        pic.dump_mvs("dump_mvs.txt");
    pic_num++;
#endif

    push_frame(frame);
    return true;
}

bool mp2v_decoder_c::decoder_init(decoder_config_t* config) {
    int pool_size = config->frames_pool_size;
    int width = config->width;
    int height = config->height;
    int chroma_format = config->chroma_format;

    for (int i = 0; i < pool_size; i++)
        m_frames_pool.push(new frame_c(width, height, chroma_format));

    return true;
}