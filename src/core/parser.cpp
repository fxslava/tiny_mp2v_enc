// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include "parser.h"
#include "mp2v_hdr.h"
#include "mp2v_vlc.h"

#define CHECK(p) { if (!(p)) return false; }

static uint8_t local_find_start_code(bitstream_reader_i* bs) {
    bs->seek_pattern(vlc_start_code.value, vlc_start_code.len);
    return (uint8_t)(bs->get_next_bits(32) & 0xff);
}

static uint8_t local_next_start_code(bitstream_reader_i* bs) {
    uint32_t full_start_code = bs->get_next_bits(32);
    return full_start_code & 0xff;
}

static uint8_t local_read_start_code(bitstream_reader_i* bs) {
    uint32_t full_start_code = bs->read_next_bits(32);
    return full_start_code & 0xff;
}

template <typename T, int count>
static void local_copy_array(bitstream_reader_i* bs, T* dst) {
    for (int i = 0; i < count; i++) {
        dst[i] = bs->read_next_bits(sizeof(T) * 8);
    }
}

bool slice_c::parse_modes() {
    macroblock_modes_t modes;
    auto& ctx = m_pic->m_ctx;
    auto& pcext = ctx.picture_coding_extension;

    // macroblock_type 1 - 9 vlclbf
    if (/*(spatial_temporal_weight_code_flag = = 1) && (spatial_temporal_weight_code_table_index != '00')*/1) {
        modes.spatial_temporal_weight_code = m_bs->read_next_bits(2);
    }
    if (/*macroblock_motion_forward || macroblock_motion_backward*/1) {
        if (pcext->picture_structure == picture_structure_framepic) {
            if (/*frame_pred_frame_dct == 0*/1)
                modes.frame_motion_type = m_bs->read_next_bits(2);
        }
        else {
            modes.field_motion_type = m_bs->read_next_bits(2);
        }
    }
    if ((pcext->picture_structure == picture_structure_framepic) /*&& (frame_pred_frame_dct == 0) && (macroblock_intra || macoblock_pattern)*/) {
        modes.dct_type = m_bs->read_next_bits(1);
    }
    return true;
}

bool slice_c::parse_macroblock() {
    macroblock_t mb;
    mb.macroblock_address_increment = 0;
    while (m_bs->get_next_bits(vlc_macroblock_escape_code.len) == vlc_macroblock_escape_code.value) {
        m_bs->skip_bits(vlc_macroblock_escape_code.len);
        mb.macroblock_address_increment += 33;
    }
    mb.macroblock_address_increment += get_macroblock_address_increment(m_bs);
    parse_modes();
    if (/*macroblock_quant*/1)
        mb.quantiser_scale_code = m_bs->read_next_bits(5);
    if (/*macroblock_motion_forward || (macroblock_intra && concealment_motion_vectors)*/1);
        //motion_vectors(0);
    if (/*macroblock_motion_backward*/1);
        //motion_vectors(1);
    if (/*macroblock_intra && concealment_motion_vectors*/1)
        m_bs->skip_bits(1);
    if (/*macroblock_pattern*/1);
        //coded_block_pattern();
    /*for (i = 0; i < block_count; i + +) {
        block(i)
    }*/
    return true;
}

bool slice_c::parse_slice() {
    auto& ctx = m_pic->m_ctx;
    auto* sh = ctx.sequence_header;

    slice.slice_start_code = m_bs->read_next_bits(32);
    if (sh->vertical_size_value > 2800)
        slice.slice_vertical_position_extension = m_bs->read_next_bits(3);
    if (ctx.sequence_scalable_extension && ctx.sequence_scalable_extension->scalable_mode == scalable_mode_data_partitioning)
        slice.priority_breakpoint = m_bs->read_next_bits(7);
    slice.quantiser_scale_code = m_bs->read_next_bits(5);
    if (m_bs->get_next_bits(1) == 1) {
        slice.slice_extension_flag = m_bs->read_next_bits(1);
        slice.intra_slice = m_bs->read_next_bits(1);
        slice.slice_picture_id_enable = m_bs->read_next_bits(1);
        slice.slice_picture_id = m_bs->read_next_bits(6);
        while (m_bs->get_next_bits(1) == 1) {
            m_bs->skip_bits(9);
        }
    }
    m_bs->skip_bits(1); /* with the value '0' */
    do {
        parse_macroblock();
    } while (m_bs->get_next_bits(23) != 0);
    local_find_start_code(m_bs);
    return true;
}

bool picture_c::parse_picture() {
    do {
        m_slices.emplace_back(m_bs, this);
        m_slices.back().parse_slice();
    } while (local_next_start_code(m_bs) >= slice_start_code_min && local_next_start_code(m_bs) <= slice_start_code_max);
    local_find_start_code(m_bs);
    return true;
}

bool video_sequence_c::parse_sequence_header() {
    m_sequence_header.emplace_back();
    auto& sh = m_sequence_header.back();
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

bool video_sequence_c::parse_sequence_extension() {
    m_sequence_extension.emplace_back();
    auto& sext = m_sequence_extension.back();
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

bool video_sequence_c::parse_sequence_display_extension() {
    m_sequence_display_extension.emplace_back();
    auto& sdext = m_sequence_display_extension.back();
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

bool video_sequence_c::parse_sequence_scalable_extension() {
    m_sequence_scalable_extension.emplace_back();
    auto& ssext = m_sequence_scalable_extension.back();
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

bool video_sequence_c::parse_group_of_pictures_header() {
    m_group_of_pictures_header.emplace_back();
    auto& gph = m_group_of_pictures_header.back();
    gph.group_start_code = m_bs->read_next_bits(32);
    gph.time_code = m_bs->read_next_bits(25);
    gph.closed_gop = m_bs->read_next_bits(1);
    gph.broken_link = m_bs->read_next_bits(1);
    local_find_start_code(m_bs);
    return true;
}

bool video_sequence_c::parse_picture_header() {
    m_picture_header.emplace_back();
    auto& ph = m_picture_header.back();
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

bool video_sequence_c::parse_picture_coding_extension() {
    m_picture_coding_extension.emplace_back();
    auto& pcext = m_picture_coding_extension.back();
    pcext.extension_start_code = m_bs->read_next_bits(32);
    pcext.extension_start_code_identifier = m_bs->read_next_bits(4);
    pcext.f_code[0] = m_bs->read_next_bits(4);
    pcext.f_code[1] = m_bs->read_next_bits(4);
    pcext.f_code[2] = m_bs->read_next_bits(4);
    pcext.f_code[3] = m_bs->read_next_bits(4);
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

bool video_sequence_c::parse_quant_matrix_extension() {
    m_quant_matrix_extension.emplace_back();
    auto& qmext = m_quant_matrix_extension.back();
    qmext.extension_start_code_identifier = m_bs->read_next_bits(4);
    qmext.load_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (qmext.load_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, qmext.intra_quantiser_matrix);
    qmext.load_non_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (qmext.load_non_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, qmext.non_intra_quantiser_matrix);
    qmext.load_chroma_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (qmext.load_chroma_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, qmext.chroma_intra_quantiser_matrix);
    qmext.load_chroma_non_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (qmext.load_chroma_non_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, qmext.chroma_non_intra_quantiser_matrix);
    local_find_start_code(m_bs);
    return true;
}

bool video_sequence_c::parse_picture_display_extension() {
    /* calculta number_of_frame_centre_offsets */
    auto& sext = m_sequence_extension.back();
    auto& pcext = m_picture_coding_extension.back();
    int number_of_frame_centre_offsets;
    if (sext.progressive_sequence == 1) {
        if (pcext.repeat_first_field == 1) {
            if (pcext.top_field_first == 1)
                number_of_frame_centre_offsets = 3;
            else
                number_of_frame_centre_offsets = 2;
        } else
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
    m_picture_display_extension.emplace_back();
    auto& pdext = m_picture_display_extension.back();
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

bool video_sequence_c::parse_picture_temporal_scalable_extension() {
    m_picture_temporal_scalable_extension.emplace_back();
    auto& ptsext = m_picture_temporal_scalable_extension.back();
    ptsext.extension_start_code_identifier = m_bs->read_next_bits(4);
    ptsext.reference_select_code = m_bs->read_next_bits(2);
    ptsext.forward_temporal_reference = m_bs->read_next_bits(10);
    m_bs->skip_bits(1);
    ptsext.backward_temporal_reference = m_bs->read_next_bits(10);
    local_find_start_code(m_bs);
    return true;
}
bool video_sequence_c::parse_picture_spatial_scalable_extension() {
    m_picture_spatial_scalable_extension.emplace_back();
    auto& pssext = m_picture_spatial_scalable_extension.back();
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

bool video_sequence_c::parse_copyright_extension() {
    m_copyright_extension.emplace_back();
    auto& crext = m_copyright_extension.back();
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

bool video_sequence_c::parse_extension_and_user_data(extension_after_code_e after_code) {
    while ((local_next_start_code(m_bs) == extension_start_code) || (local_next_start_code(m_bs) == user_data_start_code)) {
        if ((after_code != after_group_of_picture_header) && (local_next_start_code(m_bs) == extension_start_code))
            parse_extension_data(after_code);
        if (local_next_start_code(m_bs) == user_data_start_code)
            parse_user_data();
    }
    return true;
}

bool video_sequence_c::parse_user_data() {
    CHECK(local_read_start_code(m_bs) == user_data_start_code);
    while (m_bs->get_next_bits(vlc_start_code.len) != vlc_start_code.value) {
        uint8_t data = m_bs->read_next_bits(8);
        user_data.push_back(data);
    }
    local_find_start_code(m_bs);
    return true;
}

bool video_sequence_c::parse_extension_data(extension_after_code_e after_code) {
    while (local_next_start_code(m_bs) == extension_start_code) {
        m_bs->skip_bits(32); //extension_start_code 32 bslbf
        if (after_code == after_sequence_extension) {
            uint8_t ext_id = m_bs->get_next_bits(4);
            switch (ext_id)
            {
            case sequence_display_extension_id:
                parse_sequence_display_extension();
                break;
            case sequence_scalable_extension_id:
                parse_sequence_scalable_extension();
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
                parse_quant_matrix_extension();
                break;
            case copiright_extension_id:
                parse_copyright_extension();
                break;
            case picture_display_extension_id:
                parse_picture_display_extension();
                break;
            case picture_spatial_scalable_extension_id:
                parse_picture_spatial_scalable_extension();
                break;
            case picture_temporal_scalable_extension_id:
                parse_picture_temporal_scalable_extension();
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

bool video_sequence_c::parse() {
    CHECK(local_next_start_code(m_bs) == sequence_header_code);
    CHECK(parse_sequence_header());

    if (local_next_start_code(m_bs) == extension_start_code) {
        parse_sequence_extension();
        do {
            parse_extension_and_user_data(after_sequence_extension);
            do {
                if (local_next_start_code(m_bs) == group_start_code) {
                    parse_group_of_pictures_header();
                    parse_extension_and_user_data(after_group_of_picture_header);
                }
                parse_picture_header();
                parse_picture_coding_extension();
                parse_extension_and_user_data(after_picture_coding_extension);
                parse_picture_data();
            } while ((local_next_start_code(m_bs) == picture_start_code) || (local_next_start_code(m_bs) == group_start_code));
            if (local_next_start_code(m_bs) != sequence_end_code) {
                parse_sequence_header();
                parse_sequence_extension();
            }
        } while (local_next_start_code(m_bs) != sequence_end_code);
    }
    else
        return false; // MPEG1 not support

    return true;
}

bool video_sequence_c::parse_picture_data() {
    parsed_context_t ctx;
    ctx.sequence_header = m_sequence_header.size() != 0 ? &m_sequence_header.back() : nullptr;
    ctx.sequence_extension = m_sequence_extension.size() != 0 ? &m_sequence_extension.back() : nullptr;
    ctx.sequence_display_extension = m_sequence_display_extension.size() != 0 ? &m_sequence_display_extension.back() : nullptr;
    ctx.sequence_scalable_extension = m_sequence_scalable_extension.size() != 0 ? &m_sequence_scalable_extension.back() : nullptr;
    ctx.group_of_pictures_header = m_group_of_pictures_header.size() != 0 ? &m_group_of_pictures_header.back() : nullptr;
    ctx.picture_header = m_picture_header.size() != 0 ? &m_picture_header.back() : nullptr;
    ctx.picture_coding_extension = m_picture_coding_extension.size() != 0 ? &m_picture_coding_extension.back() : nullptr;
    ctx.quant_matrix_extension = m_quant_matrix_extension.size() != 0 ? &m_quant_matrix_extension.back() : nullptr;
    ctx.picture_display_extension = m_picture_display_extension.size() != 0 ? &m_picture_display_extension.back() : nullptr;
    ctx.picture_temporal_scalable_extension = m_picture_temporal_scalable_extension.size() != 0 ? &m_picture_temporal_scalable_extension.back() : nullptr;
    ctx.picture_spatial_scalable_extension = m_picture_spatial_scalable_extension.size() != 0 ? &m_picture_spatial_scalable_extension.back() : nullptr;
    ctx.copyright_extension = m_copyright_extension.size() != 0 ? &m_copyright_extension.back() : nullptr;
    m_pictures.emplace_back(m_bs, ctx);
    m_pictures.back().parse_picture();
    return true;
}

bool mp2v_parser_c::parse() {
    CHECK(m_video_sequence.parse());
    return true;
}