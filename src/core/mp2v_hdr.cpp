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