#include "mp2v_hdr.h"
#include "misc.hpp"

bool write_sequence_header(bitstream_writer_c* m_bs, sequence_header_t &sh) {
    m_bs->align();
    m_bs->write_bits(START_CODE(sequence_header_code), 32);
    m_bs->write_bits(sh.horizontal_size_value, 12);
    m_bs->write_bits(sh.vertical_size_value, 12);
    m_bs->write_bits(sh.aspect_ratio_information, 4);
    m_bs->write_bits(sh.frame_rate_code, 4);
    m_bs->write_bits(sh.bit_rate_value, 18);
    m_bs->one_bit();
    m_bs->write_bits(sh.vbv_buffer_size_value, 10);
    m_bs->write_bits(sh.constrained_parameters_flag, 1);
    m_bs->write_bits(sh.load_intra_quantiser_matrix, 1);
    if (sh.load_intra_quantiser_matrix)
        local_write_array<uint8_t, 64>(m_bs, sh.intra_quantiser_matrix);
    m_bs->write_bits(sh.load_non_intra_quantiser_matrix, 1);
    if (sh.load_non_intra_quantiser_matrix)
        local_write_array<uint8_t, 64>(m_bs, sh.non_intra_quantiser_matrix);
    return true;
}

bool write_sequence_extension(bitstream_writer_c* m_bs, sequence_extension_t &sext) {
    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(sequence_extension_id, 4);
    m_bs->write_bits(sext.profile_and_level_indication, 8);
    m_bs->write_bits(sext.progressive_sequence, 1);
    m_bs->write_bits(sext.chroma_format, 2);
    m_bs->write_bits(sext.horizontal_size_extension, 2);
    m_bs->write_bits(sext.vertical_size_extension, 2);
    m_bs->write_bits(sext.bit_rate_extension, 12);
    m_bs->one_bit();
    m_bs->write_bits(sext.vbv_buffer_size_extension, 8);
    m_bs->write_bits(sext.low_delay, 1);
    m_bs->write_bits(sext.frame_rate_extension_n, 2);
    m_bs->write_bits(sext.frame_rate_extension_d, 5);
    return true;
}

bool write_sequence_display_extension(bitstream_writer_c* m_bs, sequence_display_extension_t & sdext) {
    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(sequence_display_extension_id, 4);
    m_bs->write_bits(sdext.video_format, 3);
    m_bs->write_bits(sdext.colour_description, 1);
    if (sdext.colour_description) {
        m_bs->write_bits(sdext.colour_primaries, 8);
        m_bs->write_bits(sdext.transfer_characteristics, 8);
        m_bs->write_bits(sdext.matrix_coefficients, 8);
    }
    m_bs->write_bits(sdext.display_horizontal_size, 14);
    m_bs->one_bit();
    m_bs->write_bits(sdext.display_vertical_size, 14);
    return true;
}

bool write_sequence_scalable_extension(bitstream_writer_c* m_bs, sequence_scalable_extension_t &ssext) {
    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(sequence_scalable_extension_id, 4);
    m_bs->write_bits(ssext.scalable_mode, 2);
    m_bs->write_bits(ssext.layer_id, 4);
    if (ssext.scalable_mode == scalable_mode_spatial_scalability) {
        m_bs->write_bits(ssext.lower_layer_prediction_horizontal_size, 14);
        m_bs->one_bit();
        m_bs->write_bits(ssext.lower_layer_prediction_vertical_size, 14);
        m_bs->write_bits(ssext.horizontal_subsampling_factor_m, 5);
        m_bs->write_bits(ssext.horizontal_subsampling_factor_n, 5);
        m_bs->write_bits(ssext.vertical_subsampling_factor_m, 5);
        m_bs->write_bits(ssext.vertical_subsampling_factor_n, 5);
    }
    if (ssext.scalable_mode == scalable_mode_temporal_scalability) {
        m_bs->write_bits(ssext.picture_mux_enable, 1);
        if (ssext.picture_mux_enable)
            m_bs->write_bits(ssext.mux_to_progressive_sequence, 1);
        m_bs->write_bits(ssext.picture_mux_order, 3);
        m_bs->write_bits(ssext.picture_mux_factor, 3);
    }
    return true;
}

bool write_group_of_pictures_header(bitstream_writer_c* m_bs, group_of_pictures_header_t &gph) {
    m_bs->align();
    m_bs->write_bits(START_CODE(group_start_code), 32);
    m_bs->write_bits(gph.time_code, 25);
    m_bs->write_bits(gph.closed_gop, 1);
    m_bs->write_bits(gph.broken_link, 1);
    return true;
}

bool write_picture_header(bitstream_writer_c* m_bs, picture_header_t & ph) {
    m_bs->align();
    m_bs->write_bits(START_CODE(picture_start_code), 32);
    m_bs->write_bits(ph.temporal_reference, 10);
    m_bs->write_bits(ph.picture_coding_type, 3);
    m_bs->write_bits(ph.vbv_delay, 16);
    if (ph.picture_coding_type == 2 || ph.picture_coding_type == 3) {
        m_bs->write_bits(ph.full_pel_forward_vector, 1);
        m_bs->write_bits(ph.forward_f_code, 3);
    }
    if (ph.picture_coding_type == 3) {
        m_bs->write_bits(ph.full_pel_backward_vector, 1);
        m_bs->write_bits(ph.backward_f_code, 3);
    }
    m_bs->zero_bit(); // skip all extra_information_picture
    return true;
}

bool write_picture_coding_extension(bitstream_writer_c* m_bs, picture_coding_extension_t &pcext) {
    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(picture_coding_extension_id, 4);
    m_bs->write_bits(pcext.f_code[0][0], 4);
    m_bs->write_bits(pcext.f_code[0][1], 4);
    m_bs->write_bits(pcext.f_code[1][0], 4);
    m_bs->write_bits(pcext.f_code[1][1], 4);
    m_bs->write_bits(pcext.intra_dc_precision, 2);
    m_bs->write_bits(pcext.picture_structure, 2);
    m_bs->write_bits(pcext.top_field_first, 1);
    m_bs->write_bits(pcext.frame_pred_frame_dct, 1);
    m_bs->write_bits(pcext.concealment_motion_vectors, 1);
    m_bs->write_bits(pcext.q_scale_type, 1);
    m_bs->write_bits(pcext.intra_vlc_format, 1);
    m_bs->write_bits(pcext.alternate_scan, 1);
    m_bs->write_bits(pcext.repeat_first_field, 1);
    m_bs->write_bits(pcext.chroma_420_type, 1);
    m_bs->write_bits(pcext.progressive_frame, 1);
    m_bs->write_bits(pcext.composite_display_flag, 1);
    if (pcext.composite_display_flag) {
        m_bs->write_bits(pcext.v_axis, 1);
        m_bs->write_bits(pcext.field_sequence, 3);
        m_bs->write_bits(pcext.sub_carrier, 1);
        m_bs->write_bits(pcext.burst_amplitude, 7);
        m_bs->write_bits(pcext.sub_carrier_phase, 8);
    }
    return true;
}

bool write_quant_matrix_extension(bitstream_writer_c* m_bs, quant_matrix_extension_t &qmext) {
    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(quant_matrix_extension_id, 4);
    m_bs->write_bits(qmext.load_intra_quantiser_matrix, 1);
    if (qmext.load_intra_quantiser_matrix)
        local_write_array<uint8_t, 64>(m_bs, qmext.intra_quantiser_matrix);
    m_bs->write_bits(qmext.load_non_intra_quantiser_matrix, 1);
    if (qmext.load_non_intra_quantiser_matrix)
        local_write_array<uint8_t, 64>(m_bs, qmext.non_intra_quantiser_matrix);
    m_bs->write_bits(qmext.load_chroma_intra_quantiser_matrix, 1);
    if (qmext.load_chroma_intra_quantiser_matrix)
        local_write_array<uint8_t, 64>(m_bs, qmext.chroma_intra_quantiser_matrix);
    m_bs->write_bits(qmext.load_chroma_non_intra_quantiser_matrix, 1);
    if (qmext.load_chroma_non_intra_quantiser_matrix)
        local_write_array<uint8_t, 64>(m_bs, qmext.chroma_non_intra_quantiser_matrix);
    return true;
}

bool write_picture_display_extension(bitstream_writer_c* m_bs, picture_display_extension_t &pdext, sequence_extension_t &sext, picture_coding_extension_t &pcext) {
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

    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(picture_display_extension_id, 4);
    for (int i = 0; i < number_of_frame_centre_offsets; i++) {
        m_bs->write_bits(pdext.frame_centre_horizontal_offset[i], 16);
        m_bs->one_bit();
        m_bs->write_bits(pdext.frame_centre_vertical_offset[i], 16);
        m_bs->one_bit();
    }
    return true;
}

bool write_picture_temporal_scalable_extension(bitstream_writer_c* m_bs, picture_temporal_scalable_extension_t& ptsext) {
    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(picture_temporal_scalable_extension_id, 4);
    m_bs->write_bits(ptsext.reference_select_code, 2);
    m_bs->write_bits(ptsext.forward_temporal_reference, 10);
    m_bs->one_bit();
    m_bs->write_bits(ptsext.backward_temporal_reference, 10);
    return true;
}
bool write_picture_spatial_scalable_extension(bitstream_writer_c* m_bs, picture_spatial_scalable_extension_t& pssext) {
    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(picture_spatial_scalable_extension_id, 4);
    m_bs->write_bits(pssext.lower_layer_temporal_reference, 10);
    m_bs->one_bit();
    m_bs->write_bits(pssext.lower_layer_horizontal_offset, 15);
    m_bs->one_bit();
    m_bs->write_bits(pssext.lower_layer_vertical_offset, 15);
    m_bs->write_bits(pssext.spatial_temporal_weight_code_table_index, 2);
    m_bs->write_bits(pssext.lower_layer_progressive_frame, 1);
    m_bs->write_bits(pssext.lower_layer_deinterlaced_field_select, 1);
    return true;
}

bool write_copyright_extension(bitstream_writer_c* m_bs, copyright_extension_t& crext) {
    m_bs->write_bits(START_CODE(extension_start_code), 32);
    m_bs->write_bits(copiright_extension_id, 4);
    m_bs->write_bits(crext.copyright_flag, 1);
    m_bs->write_bits(crext.copyright_identifier, 8);
    m_bs->write_bits(crext.original_or_copy, 1);
    m_bs->write_bits(crext.reserved, 7);
    m_bs->one_bit();
    m_bs->write_bits(crext.copyright_number_1, 20);
    m_bs->one_bit();
    m_bs->write_bits(crext.copyright_number_2, 22);
    m_bs->one_bit();
    m_bs->write_bits(crext.copyright_number_3, 22);
    return true;
}
