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

uint16_t predictor_reset_value[4] = { 128, 256, 512, 1024 };

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
        m_planes[i] = new uint8_t[m_height[i] * m_stride[i]];
}

frame_c::~frame_c() {
    for (int i = 0; i < 3; i++)
        if (m_planes[i]) delete[] m_planes[i];
}

MP2V_INLINE static void parse_mb_pattern(macroblock_t& mb, bool (&pattern_code)[12], int chroma_format) {
    bool macroblock_intra = mb.macroblock_type & macroblock_intra_bit;
    bool macroblock_pattern = mb.macroblock_type & macroblock_pattern_bit;
    uint32_t coded_block_pattern_1 = mb.coded_block_pattern_1;
    uint32_t coded_block_pattern_2 = mb.coded_block_pattern_2;
    uint32_t cbp = mb.coded_block_pattern_420;

    memset(pattern_code, 0, sizeof(pattern_code));
    for (int i = 0; i < 12; i++) {
        if (macroblock_intra)
            pattern_code[i] = true;
    }
    if (macroblock_pattern) {
        for (int i = 0; i < 6; i++)
            if (cbp & (1 << (5 - i))) pattern_code[i] = true;
        if (chroma_format == chroma_format_422)
            for (int i = 6; i < 8; i++)
                if (coded_block_pattern_1 & (1 << (7 - i))) pattern_code[i] = true;
        if (chroma_format == chroma_format_444)
            for (int i = 6; i < 12; i++)
                if (coded_block_pattern_2 & (1 << (11 - i))) pattern_code[i] = true;
    }
}

template<bool use_dct_one_table>
MP2V_INLINE static void read_first_coefficient(bitstream_reader_c* bs, uint32_t& run, int32_t& level) {
    if (bs->get_next_bits(6) == 0b000001) {
        bs->skip_bits(6);
        run = bs->read_next_bits(6);
        level = bs->read_next_bits(12);
        if (level & 0b100000000000)
            level |= 0xfffff000;
    }
    else {
        if (bs->get_next_bits(1) == 1) {
            int code = bs->read_next_bits(2);
            level = (code == 2) ? 1 : -1;
            run = 0;
        }
        else {
            coeff_t c = use_dct_one_table ? get_coeff_one(bs) : get_coeff_zero(bs);
            int s = bs->read_next_bits(1);
            level = s ? -c.level : c.level;
            run = c.run;
        }
    }
}

template<bool use_dct_one_table>
MP2V_INLINE static void read_block_coefficients(bitstream_reader_c* bs, uint32_t& n, int16_t QFS[64]) {
    bool eob_not_read = true;
    while (eob_not_read) {
        //<decode VLC, decode Escape coded coefficient if required>
        int run;
        int signed_level;
        int eob = use_dct_one_table ? bs->get_next_bits(4) : bs->get_next_bits(2);
        int eob_code = use_dct_one_table ? 6 : 2;

        if (eob != eob_code) {
            if (bs->get_next_bits(6) == 0b000001) {
                bs->skip_bits(6);
                run = bs->read_next_bits(6);
                signed_level = bs->read_next_bits(12);
                if (signed_level & 0b100000000000)
                    signed_level |= 0xfffff000;
            }
            else {
                coeff_t coeff = use_dct_one_table ? get_coeff_one(bs) : get_coeff_zero(bs);
                int s = bs->read_next_bits(1);
                run = coeff.run;
                signed_level = s ? -coeff.level : coeff.level;
            }

            n += run;
            QFS[n] = signed_level;
            n++;
        }
        else { //<decoded VLC indicates End of block>
            if (use_dct_one_table)
                bs->skip_bits(4);
            else 
                bs->skip_bits(2);

            eob_not_read = 0;
        }
    }
}

template<bool use_dct_one_table>
static bool parse_block(bitstream_reader_c* bs, mb_data_t& mb_data, int i, uint16_t* dct_dc_pred) {
    auto& mb = mb_data.mb;
    uint32_t n = 0;

    int cc = color_component_index[i];

    if (mb_data.pattern_code[i]) 
    {
        memset(mb_data.QFS[i], 0, sizeof(int16_t) * 64);

        if (mb.macroblock_type & macroblock_intra_bit) {
            uint16_t dct_dc_differential;
            uint16_t dct_dc_size;
            if (i < 4) {
                dct_dc_size = get_dct_size_luminance(bs);
                if (dct_dc_size != 0)
                    dct_dc_differential = bs->read_next_bits(dct_dc_size);
            }
            else {
                dct_dc_size = get_dct_size_chrominance(bs);
                if (dct_dc_size != 0)
                    dct_dc_differential = bs->read_next_bits(dct_dc_size);
            }
            n++;

            int16_t dct_diff;
            if (dct_dc_size == 0) {
                dct_diff = 0;
            }
            else {
                uint16_t half_range = 1 << (dct_dc_size - 1);
                if (dct_dc_differential >= half_range)
                    dct_diff = dct_dc_differential;
                else
                    dct_diff = (dct_dc_differential + 1) - (2 * half_range);
            }
            mb_data.QFS[i][0] = dct_dc_pred[cc] + dct_diff;
            dct_dc_pred[cc] = mb_data.QFS[i][0];
        }
        else {
            int32_t level;
            read_first_coefficient<use_dct_one_table>(bs, n, level);
            mb_data.QFS[i][n] = level;
            n++;
        }
        read_block_coefficients<use_dct_one_table>(bs, n, mb_data.QFS[i]);
    }

    return true;
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

static void inc_macroblock_yuv_ptrs(uint8_t* (&yuv)[3], int chroma_format) {
    yuv[0] += 16;
    switch (chroma_format) {
    case chroma_format_420:
        yuv[1] += 8;
        yuv[2] += 8;
        break;
    case chroma_format_422:
        yuv[1] += 8;
        yuv[2] += 8;
        break;
    case chroma_format_444:
        yuv[1] += 16;
        yuv[2] += 16;
        break;
    }
}

bool mp2v_slice_c::decode_macroblock(uint8_t* (&yuv)[3], uint8_t* (&yuv_l0)[3], uint8_t* (&yuv_l1)[3]) {
    if (m_picture_coding_type == picture_coding_type_pred) {
        m_cur_mb_data = { 0 };
        m_cur_mb_data.mb.prediction_type = Frame_based;
        m_cur_mb_data.mb.motion_vector_count = 1;
    }

    auto refs = m_pic->m_dec->ref_frames;
    mp2v_picture_c* pic = reinterpret_cast<mp2v_picture_c*>(m_pic);
    auto& pcext = m_pic->m_picture_coding_extension;
    auto& mb = m_cur_mb_data.mb;
    int stride = m_frame->m_stride[0];
    int chroma_stride = m_frame->m_stride[1];

    m_pic->m_parse_macroblock_func(m_bs, mb, 0, m_f_code, m_PMV, m_cur_mb_data.MVs);
    parse_mb_pattern(mb, m_cur_mb_data.pattern_code, m_chroma_format);

    // Decode coefficients
    bool m_use_dct_one_table = (m_intra_vlc_format == 1) && (mb.macroblock_type & macroblock_intra_bit);
    if (mb.macroblock_type & macroblock_intra_bit)
    {
        for (int i = 0; i < m_block_count; i++) {
            if (m_use_dct_one_table)
                parse_block<true>(m_bs, m_cur_mb_data, i, m_dct_dc_pred);
            else
                parse_block<false>(m_bs, m_cur_mb_data, i, m_dct_dc_pred);
        }
    }
    else {
        if (mb.macroblock_type & macroblock_pattern_bit) {
            for (int i = 0; i < 6; i++)
                if (mb.coded_block_pattern_420 & (1 << (5 - i))) {
                    if (m_use_dct_one_table)
                        parse_block<true>(m_bs, m_cur_mb_data, i, m_dct_dc_pred);
                    else
                        parse_block<false>(m_bs, m_cur_mb_data, i, m_dct_dc_pred);
                }
            if (m_chroma_format == chroma_format_422)
                for (int i = 6; i < 8; i++)
                    if (mb.coded_block_pattern_1 & (1 << (7 - i))) {
                        if (m_use_dct_one_table)
                            parse_block<true>(m_bs, m_cur_mb_data, i, m_dct_dc_pred);
                        else
                            parse_block<false>(m_bs, m_cur_mb_data, i, m_dct_dc_pred);
                    }
            if (m_chroma_format == chroma_format_444)
                for (int i = 6; i < 12; i++)
                    if (mb.coded_block_pattern_2 & (1 << (11 - i))) {
                        if (m_use_dct_one_table)
                            parse_block<true>(m_bs, m_cur_mb_data, i, m_dct_dc_pred);
                        else
                            parse_block<false>(m_bs, m_cur_mb_data, i, m_dct_dc_pred);
                    }
        }
    }

    // decode skipped macroblocks
    //mb_data_t skipped_mb_data = { 0 };
    if ((mb.macroblock_address_increment > 1) && (m_picture_coding_type == picture_coding_type_bidir)) {
        m_cur_mb_data.mb.macroblock_type = mb.macroblock_type & (macroblock_motion_forward_bit | macroblock_motion_backward_bit);
        m_cur_mb_data.mb.prediction_type = Frame_based;
        m_cur_mb_data.mb.motion_vector_count = 1;
    }
    for (int i = 0; i < mb.macroblock_address_increment; i++) {
        if (i == (mb.macroblock_address_increment - 1)) break;

        //update motion vectors predictors
        if (m_picture_coding_type == picture_coding_type_pred) {
            m_cur_mb_data.mb.prediction_type = Frame_based;
            memset(m_cur_mb_data.MVs, 0, sizeof(m_cur_mb_data.MVs));
            memset(m_PMV, 0, sizeof(m_PMV));
        }

        m_pic->m_decode_macroblock_func(yuv, stride, chroma_stride, m_cur_mb_data, pic->quantiser_matrices, pcext.intra_dc_precision, cur_quantiser_scale_code, yuv_l0, yuv_l1);

        inc_macroblock_yuv_ptrs(yuv, m_chroma_format);
        inc_macroblock_yuv_ptrs(yuv_l0, m_chroma_format);
        inc_macroblock_yuv_ptrs(yuv_l1, m_chroma_format);
    }

    if (!(mb.macroblock_type & macroblock_intra_bit) || mb.macroblock_address_increment > 1)
        for (auto& pred : m_dct_dc_pred)
            pred = m_dct_dc_pred_reset_value;

    // Update motion vectors predictors conditions (Table 7-9 � Updating of motion vector predictors in frame pictures)
    if (mb.prediction_type == Frame_based) {
        if (mb.macroblock_type & macroblock_intra_bit)
            for (int t : { 0, 1 }) m_PMV[1][0][t] = m_PMV[0][0][t];
        if ((mb.macroblock_type & macroblock_motion_forward_bit) && (mb.macroblock_type & macroblock_motion_backward_bit) && !(mb.macroblock_type & macroblock_intra_bit))
            for (int t : { 0, 1 }) {
                m_PMV[1][0][t] = m_PMV[0][0][t];
                m_PMV[1][1][t] = m_PMV[0][1][t];
            }
        if ((mb.macroblock_type & macroblock_motion_forward_bit) && !(mb.macroblock_type & macroblock_motion_backward_bit) && !(mb.macroblock_type & macroblock_intra_bit))
            for (int t : { 0, 1 }) m_PMV[1][0][t] = m_PMV[0][0][t];
        if (!(mb.macroblock_type & macroblock_motion_forward_bit) && (mb.macroblock_type & macroblock_motion_backward_bit) && !(mb.macroblock_type & macroblock_intra_bit))
            for (int t : { 0, 1 }) m_PMV[1][1][t] = m_PMV[0][1][t];
    }
    if (mb.prediction_type == Dual_Prime)
        if ((mb.macroblock_type & macroblock_motion_forward_bit) && !(mb.macroblock_type & macroblock_motion_backward_bit) && !(mb.macroblock_type & macroblock_intra_bit))
            for (int t : { 0, 1 }) m_PMV[1][0][t] = m_PMV[0][0][t];

    if (((mb.macroblock_type & macroblock_intra_bit) && !m_concealment_motion_vectors) ||
        ((m_picture_coding_type == picture_coding_type_pred) && !(mb.macroblock_type & macroblock_intra_bit) && !(mb.macroblock_type & macroblock_motion_forward_bit)))
    {
        memset(m_cur_mb_data.MVs, 0, sizeof(m_cur_mb_data.MVs));
        memset(m_PMV, 0, sizeof(m_PMV));
    }

    m_pic->m_decode_macroblock_func(yuv, stride, chroma_stride, m_cur_mb_data, pic->quantiser_matrices, pcext.intra_dc_precision, cur_quantiser_scale_code, yuv_l0, yuv_l1);

    inc_macroblock_yuv_ptrs(yuv, m_chroma_format);
    inc_macroblock_yuv_ptrs(yuv_l0, m_chroma_format);
    inc_macroblock_yuv_ptrs(yuv_l1, m_chroma_format);
    return true;
}

bool mp2v_slice_c::decode_slice() {
    auto* seq = m_pic->m_dec;
    for (auto& pred : m_dct_dc_pred)
        pred = m_dct_dc_pred_reset_value;

    // reset motion vectors predictors
    memset(&m_cur_mb_data, 0, sizeof(m_cur_mb_data));
    memset(m_PMV, 0, sizeof(m_PMV));

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

    // calculate row|col position of the slice
    int slice_vertical_position = m_slice.slice_start_code & 0xff;
    if (m_vertical_size_value > 2800)
        mb_row = (m_slice.slice_vertical_position_extension << 7) + slice_vertical_position - 1;
    else
        mb_row = slice_vertical_position - 1;
    cur_quantiser_scale_code = m_slice.quantiser_scale_code;

    // calcultae yuv ptrs
    auto refs = m_pic->m_dec->ref_frames;
    int stride = m_frame->m_stride[0];
    int chroma_stride = m_frame->m_stride[1];
    uint8_t *yuv[3], *ref0[3], *ref1[3];
    make_macroblock_yuv_ptrs(yuv, m_frame, mb_row, stride, chroma_stride, m_chroma_format);
    if (refs[0])
        make_macroblock_yuv_ptrs(ref0, refs[0], mb_row, stride, chroma_stride, m_chroma_format);
    if (refs[1])
        make_macroblock_yuv_ptrs(ref1, refs[1], mb_row, stride, chroma_stride, m_chroma_format);

    macroblock_context_cache_t cache;
    memcpy(cache.W, m_pic->quantiser_matrices, sizeof(cache.W));
    memcpy(cache.f_code, m_f_code, sizeof(cache.f_code));
    memcpy(cache.PMVs, m_PMV, sizeof(cache.PMVs));
    memcpy(cache.dct_dc_pred, m_dct_dc_pred, sizeof(cache.dct_dc_pred));
    memcpy(cache.yuv_planes[REF_TYPE_SRC], yuv, sizeof(yuv));
    memcpy(cache.yuv_planes[REF_TYPE_L0], ref0, sizeof(yuv));
    memcpy(cache.yuv_planes[REF_TYPE_L1], ref1, sizeof(yuv));
    cache.spatial_temporal_weight_code_table_index = 0;
    cache.luma_stride = stride;
    cache.chroma_stride = chroma_stride;
    cache.quantiser_scale_code = cur_quantiser_scale_code;
    cache.intra_dc_prec = m_pic->m_picture_coding_extension.intra_dc_precision;

    // decode macroblocks
    m_bs->skip_bits(1); /* with the value '0' */
    do {
        decode_macroblock(yuv, ref0, ref1);

#ifdef _DEBUG
        m_macroblocks.push_back(m_cur_mb_data);
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

    m_decode_macroblock_func = select_decode_macroblock(sext.chroma_format, m_picture_coding_extension.q_scale_type, m_picture_coding_extension.alternate_scan);
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
            x0 += 16 * mb.mb.macroblock_address_increment;
            if (mb.mb.macroblock_type & macroblock_motion_forward_bit) {
                int x1 = x0 + mb.MVs[0][0][0];
                int y1 = y0 + mb.MVs[0][0][1];
                fprintf(fp, "%d\t%d\n", x0, y0);
                fprintf(fp, "%d\t%d\tx:%d y:%d\n", x1, y1, mb.MVs[0][0][0], mb.MVs[0][0][1]);
                fprintf(fp, "\n");
            }
            if (mb.mb.macroblock_type & macroblock_motion_backward_bit) {
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