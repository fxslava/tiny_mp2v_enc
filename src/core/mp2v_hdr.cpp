#include "mp2v_hdr.h"
#include "misc.hpp"

bool parse_sequence_header(bitstream_reader_c* m_bs, sequence_header_t &sh) {
    sh.sequence_header_code = m_bs->read_next_bits(32);
    sh.horizontal_size_value = m_bs->read_next_bits(12);
    sh.vertical_size_value = m_bs->read_next_bits(12);
    sh.aspect_ratio_information = m_bs->read_next_bits(4);
    sh.frame_rate_code = m_bs->read_next_bits(4);
    sh.bit_rate_value = m_bs->read_next_bits(18);
    m_bs->skip_bits(1);
    sh.vbv_buffer_size_value = m_bs->read_next_bits(10);
    sh.constrained_parameters_flag = m_bs->read_next_bits(1);
    sh.load_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (sh.load_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, sh.intra_quantiser_matrix);
    sh.load_non_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (sh.load_non_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, sh.non_intra_quantiser_matrix);
    local_find_start_code(m_bs);
    return true;
}

bool parse_sequence_extension(bitstream_reader_c* m_bs, sequence_extension_t &sext) {
    sext.extension_start_code = m_bs->read_next_bits(32);
    sext.extension_start_code_identifier = m_bs->read_next_bits(4);
    sext.profile_and_level_indication = m_bs->read_next_bits(8);
    sext.progressive_sequence = m_bs->read_next_bits(1);
    sext.chroma_format = m_bs->read_next_bits(2);
    sext.horizontal_size_extension = m_bs->read_next_bits(2);
    sext.vertical_size_extension = m_bs->read_next_bits(2);
    sext.bit_rate_extension = m_bs->read_next_bits(12);
    m_bs->skip_bits(1);
    sext.vbv_buffer_size_extension = m_bs->read_next_bits(8);
    sext.low_delay = m_bs->read_next_bits(1);
    sext.frame_rate_extension_n = m_bs->read_next_bits(2);
    sext.frame_rate_extension_d = m_bs->read_next_bits(5);
    local_find_start_code(m_bs);
    return true;
}

bool parse_sequence_display_extension(bitstream_reader_c* m_bs, sequence_display_extension_t & sdext) {
    sdext.extension_start_code_identifier = m_bs->read_next_bits(4);
    sdext.video_format = m_bs->read_next_bits(3);
    sdext.colour_description = m_bs->read_next_bits(1);
    if (sdext.colour_description) {
        sdext.colour_primaries = m_bs->read_next_bits(8);
        sdext.transfer_characteristics = m_bs->read_next_bits(8);
        sdext.matrix_coefficients = m_bs->read_next_bits(8);
    }
    sdext.display_horizontal_size = m_bs->read_next_bits(14);
    m_bs->skip_bits(1);
    sdext.display_vertical_size = m_bs->read_next_bits(14);
    local_find_start_code(m_bs);
    return true;
}

bool parse_sequence_scalable_extension(bitstream_reader_c* m_bs, sequence_scalable_extension_t &ssext) {
    ssext.extension_start_code_identifier = m_bs->read_next_bits(4);
    ssext.scalable_mode = m_bs->read_next_bits(2);
    ssext.layer_id = m_bs->read_next_bits(4);
    if (ssext.scalable_mode == scalable_mode_spatial_scalability) {
        ssext.lower_layer_prediction_horizontal_size = m_bs->read_next_bits(14);
        m_bs->skip_bits(1);
        ssext.lower_layer_prediction_vertical_size = m_bs->read_next_bits(14);
        ssext.horizontal_subsampling_factor_m = m_bs->read_next_bits(5);
        ssext.horizontal_subsampling_factor_n = m_bs->read_next_bits(5);
        ssext.vertical_subsampling_factor_m = m_bs->read_next_bits(5);
        ssext.vertical_subsampling_factor_n = m_bs->read_next_bits(5);
    }
    if (ssext.scalable_mode == scalable_mode_temporal_scalability) {
        ssext.picture_mux_enable = m_bs->read_next_bits(1);
        if (ssext.picture_mux_enable)
            ssext.mux_to_progressive_sequence = m_bs->read_next_bits(1);
        ssext.picture_mux_order = m_bs->read_next_bits(3);
        ssext.picture_mux_factor = m_bs->read_next_bits(3);
    }
    local_find_start_code(m_bs);
    return true;
}

bool parse_group_of_pictures_header(bitstream_reader_c* m_bs, group_of_pictures_header_t &gph) {
    gph.group_start_code = m_bs->read_next_bits(32);
    gph.time_code = m_bs->read_next_bits(25);
    gph.closed_gop = m_bs->read_next_bits(1);
    gph.broken_link = m_bs->read_next_bits(1);
    local_find_start_code(m_bs);
    return true;
}

bool parse_picture_header(bitstream_reader_c* m_bs, picture_header_t & ph) {
    ph.picture_start_code = m_bs->read_next_bits(32);
    ph.temporal_reference = m_bs->read_next_bits(10);
    ph.picture_coding_type = m_bs->read_next_bits(3);
    ph.vbv_delay = m_bs->read_next_bits(16);
    if (ph.picture_coding_type == 2 || ph.picture_coding_type == 3) {
        ph.full_pel_forward_vector = m_bs->read_next_bits(1);
        ph.forward_f_code = m_bs->read_next_bits(3);
    }
    if (ph.picture_coding_type == 3) {
        ph.full_pel_backward_vector = m_bs->read_next_bits(1);
        ph.backward_f_code = m_bs->read_next_bits(3);
    }
    // skip all extra_information_picture
    while (m_bs->get_next_bits(1) == 1)
        m_bs->skip_bits(9); // skip extra_bit_picture and extra_information_picture
    m_bs->skip_bits(1); // skip extra_bit_picture

    local_find_start_code(m_bs);
    return true;
}

bool parse_picture_coding_extension(bitstream_reader_c* m_bs, picture_coding_extension_t &pcext) {
    pcext.extension_start_code = m_bs->read_next_bits(32);
    pcext.extension_start_code_identifier = m_bs->read_next_bits(4);
    pcext.f_code[0][0] = m_bs->read_next_bits(4);
    pcext.f_code[0][1] = m_bs->read_next_bits(4);
    pcext.f_code[1][0] = m_bs->read_next_bits(4);
    pcext.f_code[1][1] = m_bs->read_next_bits(4);
    pcext.intra_dc_precision = m_bs->read_next_bits(2);
    pcext.picture_structure = m_bs->read_next_bits(2);
    pcext.top_field_first = m_bs->read_next_bits(1);
    pcext.frame_pred_frame_dct = m_bs->read_next_bits(1);
    pcext.concealment_motion_vectors = m_bs->read_next_bits(1);
    pcext.q_scale_type = m_bs->read_next_bits(1);
    pcext.intra_vlc_format = m_bs->read_next_bits(1);
    pcext.alternate_scan = m_bs->read_next_bits(1);
    pcext.repeat_first_field = m_bs->read_next_bits(1);
    pcext.chroma_420_type = m_bs->read_next_bits(1);
    pcext.progressive_frame = m_bs->read_next_bits(1);
    pcext.composite_display_flag = m_bs->read_next_bits(1);
    if (pcext.composite_display_flag) {
        pcext.v_axis = m_bs->read_next_bits(1);
        pcext.field_sequence = m_bs->read_next_bits(3);
        pcext.sub_carrier = m_bs->read_next_bits(1);
        pcext.burst_amplitude = m_bs->read_next_bits(7);
        pcext.sub_carrier_phase = m_bs->read_next_bits(8);
    }
    local_find_start_code(m_bs);
    return true;
}

bool parse_quant_matrix_extension(bitstream_reader_c* m_bs, quant_matrix_extension_t &qmext) {
    qmext.extension_start_code_identifier = m_bs->read_next_bits(4);
    qmext.load_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (qmext.load_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, qmext.intra_quantiser_matrix);
        //local_load_quantiser_matrix(m_bs, qmext.intra_quantiser_matrix);
    qmext.load_non_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (qmext.load_non_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, qmext.non_intra_quantiser_matrix);
        //local_load_quantiser_matrix(m_bs, qmext.non_intra_quantiser_matrix);
    qmext.load_chroma_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (qmext.load_chroma_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, qmext.chroma_intra_quantiser_matrix);
        //local_load_quantiser_matrix(m_bs, qmext.chroma_intra_quantiser_matrix);
    qmext.load_chroma_non_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (qmext.load_chroma_non_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, qmext.chroma_non_intra_quantiser_matrix);
        //local_load_quantiser_matrix(m_bs, qmext.chroma_non_intra_quantiser_matrix);
    local_find_start_code(m_bs);
    return true;
}

bool parse_picture_display_extension(bitstream_reader_c* m_bs, picture_display_extension_t &pdext, sequence_extension_t &sext, picture_coding_extension_t &pcext) {
    /* calculta number_of_frame_centre_offsets */
    int number_of_frame_centre_offsets;
    if (sext.progressive_sequence == 1) {
        if (pcext.repeat_first_field == 1) {
            if (pcext.top_field_first == 1)
                number_of_frame_centre_offsets = 3;
            else
                number_of_frame_centre_offsets = 2;
        }
        else
            number_of_frame_centre_offsets = 1;
    }
    else {
        if (pcext.picture_structure == picture_structure_topfield || pcext.picture_structure == picture_structure_botfield)
            number_of_frame_centre_offsets = 1;
        else {
            if (pcext.repeat_first_field == 1)
                number_of_frame_centre_offsets = 3;
            else
                number_of_frame_centre_offsets = 2;
        }
    }

    // parse
    pdext.extension_start_code_identifier = m_bs->read_next_bits(4);
    for (int i = 0; i < number_of_frame_centre_offsets; i++) {
        pdext.frame_centre_horizontal_offset[i] = m_bs->read_next_bits(16);
        m_bs->skip_bits(1);
        pdext.frame_centre_vertical_offset[i] = m_bs->read_next_bits(16);
        m_bs->skip_bits(1);
    }
    local_find_start_code(m_bs);
    return true;
}

bool parse_picture_temporal_scalable_extension(bitstream_reader_c* m_bs, picture_temporal_scalable_extension_t& ptsext) {
    ptsext.extension_start_code_identifier = m_bs->read_next_bits(4);
    ptsext.reference_select_code = m_bs->read_next_bits(2);
    ptsext.forward_temporal_reference = m_bs->read_next_bits(10);
    m_bs->skip_bits(1);
    ptsext.backward_temporal_reference = m_bs->read_next_bits(10);
    local_find_start_code(m_bs);
    return true;
}
bool parse_picture_spatial_scalable_extension(bitstream_reader_c* m_bs, picture_spatial_scalable_extension_t& pssext) {
    pssext.extension_start_code_identifier = m_bs->read_next_bits(4);
    pssext.lower_layer_temporal_reference = m_bs->read_next_bits(10);
    m_bs->skip_bits(1);
    pssext.lower_layer_horizontal_offset = m_bs->read_next_bits(15);
    m_bs->skip_bits(1);
    pssext.lower_layer_vertical_offset = m_bs->read_next_bits(15);
    pssext.spatial_temporal_weight_code_table_index = m_bs->read_next_bits(2);
    pssext.lower_layer_progressive_frame = m_bs->read_next_bits(1);
    pssext.lower_layer_deinterlaced_field_select = m_bs->read_next_bits(1);
    local_find_start_code(m_bs);
    return true;
}

bool parse_copyright_extension(bitstream_reader_c* m_bs, copyright_extension_t& crext) {
    crext.extension_start_code_identifier = m_bs->read_next_bits(4);
    crext.copyright_flag = m_bs->read_next_bits(1);
    crext.copyright_identifier = m_bs->read_next_bits(8);
    crext.original_or_copy = m_bs->read_next_bits(1);
    crext.reserved = m_bs->read_next_bits(7);
    m_bs->skip_bits(1);
    crext.copyright_number_1 = m_bs->read_next_bits(20);
    m_bs->skip_bits(1);
    crext.copyright_number_2 = m_bs->read_next_bits(22);
    m_bs->skip_bits(1);
    crext.copyright_number_3 = m_bs->read_next_bits(22);
    local_find_start_code(m_bs);
    return true;
}

template<int picture_coding_type, int picture_structure, int frame_pred_frame_dct>
static bool parse_modes(bitstream_reader_c* m_bs, macroblock_t& mb, int spatial_temporal_weight_code_table_index) {
    mb.macroblock_type = get_macroblock_type(m_bs, picture_coding_type);
    if ((mb.macroblock_type & spatial_temporal_weight_code_flag_bit) && (spatial_temporal_weight_code_table_index != 0)) {
        mb.spatial_temporal_weight_code = m_bs->read_next_bits(2);
    }
    if ((mb.macroblock_type & macroblock_motion_forward_bit) || (mb.macroblock_type & macroblock_motion_backward_bit)) {
        if (picture_structure == picture_structure_framepic) {
            mb.frame_motion_type = 2; // Frame-based
            if (frame_pred_frame_dct == 0)
                mb.frame_motion_type = m_bs->read_next_bits(2);
        }
        else {
            mb.frame_motion_type = 1; // Field-based
            mb.field_motion_type = m_bs->read_next_bits(2);
        }
    }
    if ((picture_structure == picture_structure_framepic) && (frame_pred_frame_dct == 0) &&
        ((mb.macroblock_type & macroblock_intra_bit) || (mb.macroblock_type & macroblock_pattern_bit))) {
        mb.dct_type = m_bs->read_next_bits(1);
    }

    // decode modes
    if (mb.macroblock_type & macroblock_intra_bit)
    {
        mb.motion_vector_count = 0;
        mb.dmv = 0;
        if (picture_structure == picture_structure_framepic) {
            mb.mv_format = Frame;
            mb.prediction_type = Frame_based;
        }
        else {
            mb.mv_format = Field;
            mb.prediction_type = Field_based;
        }
    }
    else
    {
        mb.motion_vector_count = 1;
        mb.dmv = 0;
        if (picture_structure == picture_structure_framepic) {
            switch (mb.frame_motion_type) {
            case 1:
                mb.mv_format = Field;
                mb.motion_vector_count = 2;
                mb.prediction_type = Field_based;
                break;
            case 2:
                mb.mv_format = Frame;
                mb.prediction_type = Frame_based;
                break;
            case 3:
                mb.mv_format = Field;
                mb.prediction_type = Dual_Prime;
                mb.dmv = 1;
                break;
            }
        }
        else {
            switch (mb.field_motion_type) {
            case 1:
                mb.mv_format = Field;
                mb.prediction_type = Field_based;
                break;
            case 2:
                mb.mv_format = Field;
                mb.motion_vector_count = 2;
                mb.prediction_type = MC16x8;
                break;
            case 3:
                mb.mv_format = Field;
                mb.prediction_type = Dual_Prime;
                mb.dmv = 1;
                break;
            }
        }
    }
    return true;
}

template<int chroma_format>
bool parse_coded_block_pattern(bitstream_reader_c* m_bs, macroblock_t& mb) {
    mb.coded_block_pattern_420 = get_coded_block_pattern(m_bs);
    if (chroma_format == chroma_format_422)
        mb.coded_block_pattern_1 = m_bs->read_next_bits(2);
    if (chroma_format == chroma_format_444)
        mb.coded_block_pattern_2 = m_bs->read_next_bits(6);
    return false;
}

template<uint8_t picture_structure, int r, int s, int t, bool residual>
MP2V_INLINE void update_motion_predictor(uint32_t f_code[2][2], int32_t motion_code[2][2][2], uint32_t motion_residual[2][2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2], mv_format_e mv_format) {
    int r_size = f_code[s][t] - 1;
    int f = 1 << r_size;
    int high = (16 * f) - 1;
    int low = ((-16) * f);
    int range = (32 * f);

    int delta;
    if (residual) {
        delta = ((labs(motion_code[r][s][t]) - 1) * f) + motion_residual[r][s][t] + 1;
        if (motion_code[r][s][t] < 0)
            delta = -delta;
    }
    else
        delta = motion_code[r][s][t];

    int prediction = PMV[r][s][t];
    if ((mv_format == Field) && (t == 1) && (picture_structure == picture_structure_framepic))
        prediction = PMV[r][s][t] >> 1;

    MVs[r][s][t] = prediction + delta;

    if (MVs[r][s][t] < low)  MVs[r][s][t] += range;
    if (MVs[r][s][t] > high) MVs[r][s][t] -= range;

    if ((mv_format == Field) && (t == 1) && (picture_structure == picture_structure_framepic))
        PMV[r][s][t] = MVs[r][s][t] * 2;
    else
        PMV[r][s][t] = MVs[r][s][t];
}

template<uint8_t picture_structure, int r, int s>
MP2V_INLINE bool parse_motion_vector(bitstream_reader_c* m_bs, macroblock_t& mb, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2], mv_format_e mv_format) {
    mb.motion_code[r][s][0] = get_motion_code(m_bs);
    if ((f_code[s][0] != 1) && (mb.motion_code[r][s][0] != 0)) {
        mb.motion_residual[r][s][0] = m_bs->read_next_bits(f_code[s][0] - 1);
        update_motion_predictor<picture_structure, r, s, 0, true>(f_code, mb.motion_code, mb.motion_residual, PMV, MVs, mv_format);
    }
    else
        update_motion_predictor<picture_structure, r, s, 0, false>(f_code, mb.motion_code, mb.motion_residual, PMV, MVs, mv_format);

    if (mb.dmv == 1)
        mb.dmvector[0] = get_dmvector(m_bs);
    mb.motion_code[r][s][1] = get_motion_code(m_bs);
    if ((f_code[s][1] != 1) && (mb.motion_code[r][s][1] != 0)) {
        mb.motion_residual[r][s][1] = m_bs->read_next_bits(f_code[s][1] - 1);
        update_motion_predictor<picture_structure, r, s, 1, true>(f_code, mb.motion_code, mb.motion_residual, PMV, MVs, mv_format);
    }
    else
        update_motion_predictor<picture_structure, r, s, 1, false>(f_code, mb.motion_code, mb.motion_residual, PMV, MVs, mv_format);

    if (mb.dmv == 1)
        mb.dmvector[1] = get_dmvector(m_bs);
    return true;
}

template <uint8_t picture_structure, int s>
MP2V_INLINE bool parse_motion_vectors(bitstream_reader_c* m_bs, macroblock_t& mb, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2], mv_format_e mv_format) {
    if (mb.motion_vector_count == 1) {
        if ((mb.mv_format == Field) && (mb.dmv != 1))
            mb.motion_vertical_field_select[0][s] = m_bs->read_next_bits(1);
        parse_motion_vector<picture_structure, 0,s>(m_bs, mb, f_code, PMV, MVs, mv_format);
    }
    else {
        mb.motion_vertical_field_select[0][s] = m_bs->read_next_bits(1);
        parse_motion_vector<picture_structure, 0,s>(m_bs, mb, f_code, PMV, MVs, mv_format);
        mb.motion_vertical_field_select[1][s] = m_bs->read_next_bits(1);
        parse_motion_vector<picture_structure, 1,s>(m_bs, mb, f_code, PMV, MVs, mv_format);
    }
    return true;
}

template<uint8_t picture_coding_type,        //3 bit (I, P, B)
         uint8_t picture_structure,          //2 bit (top|bottom field, frame)
         uint8_t frame_pred_frame_dct,       //1 bit // only with picture_structure == frame
         uint8_t concealment_motion_vectors, //1 bit // only with picture_coding_type == I
         uint8_t chroma_format>              //2 bit (420, 422, 444)
bool parse_macroblock_template(bitstream_reader_c* m_bs, macroblock_t& mb, int spatial_temporal_weight_code_table_index, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2]) {
    mb.macroblock_address_increment = 0;
    while (m_bs->get_next_bits(vlc_macroblock_escape_code.len) == vlc_macroblock_escape_code.value) {
        m_bs->skip_bits(vlc_macroblock_escape_code.len);
        mb.macroblock_address_increment += 33;
    }
    mb.macroblock_address_increment += get_macroblock_address_increment(m_bs);

    parse_modes<picture_coding_type, picture_structure, frame_pred_frame_dct>(m_bs, mb, spatial_temporal_weight_code_table_index);
    if (mb.macroblock_type & macroblock_quant_bit)
        mb.quantiser_scale_code = m_bs->read_next_bits(5);
    if ((mb.macroblock_type & macroblock_motion_forward_bit) || ((mb.macroblock_type & macroblock_intra_bit) && concealment_motion_vectors))
        parse_motion_vectors<picture_coding_type, 0>(m_bs, mb, f_code, PMV, MVs, mb.mv_format);
    if ((mb.macroblock_type & macroblock_motion_backward_bit) != 0)
        parse_motion_vectors<picture_coding_type, 1>(m_bs, mb, f_code, PMV, MVs, mb.mv_format);
    if (((mb.macroblock_type & macroblock_intra_bit) != 0) && concealment_motion_vectors)
        m_bs->skip_bits(1);
    if (mb.macroblock_type & macroblock_pattern_bit)
        parse_coded_block_pattern<chroma_format>(m_bs, mb);
    return true;
}

#define DEF_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(prefix, pct, ps, fpfdct, cmv) \
static bool parse_##prefix##_420_macroblock(bitstream_reader_c* m_bs, macroblock_t& mb, int spatial_temporal_weight_code_table_index, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2]) { \
    return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_420>(m_bs, mb, spatial_temporal_weight_code_table_index, f_code, PMV, MVs); \
} \
static bool parse_##prefix##_422_macroblock(bitstream_reader_c* m_bs, macroblock_t& mb, int spatial_temporal_weight_code_table_index, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2]) { \
    return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_422>(m_bs, mb, spatial_temporal_weight_code_table_index, f_code, PMV, MVs); \
} \
static bool parse_##prefix##_444_macroblock(bitstream_reader_c* m_bs, macroblock_t& mb, int spatial_temporal_weight_code_table_index, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2]) { \
    return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_444>(m_bs, mb, spatial_temporal_weight_code_table_index, f_code, PMV, MVs); \
}

#define DEF_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(prefix, pct, cmv) \
DEF_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(prefix##_frame, pct, picture_structure_framepic, 0, cmv) \
DEF_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(prefix##_frame_dct, pct, picture_structure_framepic, 1, cmv) \
DEF_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(prefix##_field, pct, picture_structure_topfield, 0, cmv)

DEF_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(i, picture_coding_type_intra, 0);
DEF_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(i_cmv, picture_coding_type_intra, 1);
DEF_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(p, picture_coding_type_pred, 0);
DEF_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(b, picture_coding_type_bidir, 0);

#define SEL_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(prefix, pct, ps, fpfdct, cmv) \
    switch (chroma_format) { \
        case chroma_format_420: return parse_##prefix##_420_macroblock; \
        case chroma_format_422: return parse_##prefix##_422_macroblock; \
        case chroma_format_444: return parse_##prefix##_444_macroblock; \
    }

#define SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(prefix, pct, cmv) \
    if (picture_structure == picture_structure_framepic) { \
        if (frame_pred_frame_dct) \
            SEL_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(prefix##_frame_dct, pct, picture_structure_framepic, 1, cmv) \
        else \
            SEL_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(prefix##_frame, pct, picture_structure_framepic, 0, cmv) \
    } else \
        SEL_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(prefix##_field, pct, picture_structure_topfield, 0, cmv)

parse_macroblock_func_t select_parse_macroblock_func(uint8_t picture_coding_type, uint8_t picture_structure, uint8_t frame_pred_frame_dct, uint8_t concealment_motion_vectors, uint8_t chroma_format) 
{
    switch (picture_coding_type) {
    case picture_coding_type_intra:
        if (concealment_motion_vectors) {
            SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(i_cmv, picture_coding_type_intra, 1)
        }
        else {
            SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(i, picture_coding_type_intra, 0)
        }
    case picture_coding_type_pred:
        SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(p, picture_coding_type_pred, 0)
    case picture_coding_type_bidir:
        SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(b, picture_coding_type_bidir, 0)
    default:
        return 0;
    };
};