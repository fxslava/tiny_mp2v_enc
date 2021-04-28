// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include "parser.h"
#include "mp2v_hdr.h"
#include "mp2v_vlc.h"
#include "misc.hpp"

#define CHECK(p) { if (!(p)) return false; }

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

template<bool use_dct_one_table>
static void read_first_coefficient(bitstream_reader_i* bs, uint32_t &run, int32_t &level) {
    if (use_dct_one_table) {
        coeff_t c =  get_coeff_one(bs);
        int s = bs->read_next_bits(1);
        level = s ? -c.level : c.level;
        run = c.run;
    }
    else {
        if (bs->get_next_bits(1) == 1) {
            int code = bs->read_next_bits(2);
            level = (code == 2) ? 1 : -1;
            run = 0;
        }
        else {
            coeff_t c = get_coeff_zero(bs);
            int s = bs->read_next_bits(1);
            level = s ? -c.level : c.level;
            run = c.run;
        }
    }
}

template<bool use_dct_one_table>
static void read_block_coefficients(bitstream_reader_i* bs, int& n, int16_t QFS[64]) {
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

            for (int m = 0; m < run; m++) {
                QFS[n] = 0;
                n++;
            }
            QFS[n] = signed_level;
            n++;
        }
        else { //<decoded VLC indicates End of block>
            if (use_dct_one_table)
                bs->skip_bits(4);
            else 
                bs->skip_bits(2);

            eob_not_read = 0;
            while (n < 64) {
                QFS[n] = 0;
                n++;
            }
        }
    }
}

template<bool use_dct_one_table>
static bool parse_block(bitstream_reader_i* bs, mb_data_t& mb_data, int i, uint16_t* dct_dc_pred) {
    auto& mb = mb_data.mb;
    int n = 0;

    int cc = color_component_index[i];

    if (mb_data.pattern_code[i]) {
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
            mb_data.QFS[0] = dct_dc_pred[cc] + dct_diff;
            dct_dc_pred[cc] = mb_data.QFS[0];
        }
        else {
            uint32_t run;
            int32_t level;
            read_first_coefficient<use_dct_one_table>(bs, run, level);
            mb_data.QFS[0] = level;
            n += run;
        }
        read_block_coefficients<use_dct_one_table>(bs, n, mb_data.QFS);
    }

    return true;
}

bool slice_c::init_slice() {
    auto* seq = m_pic->get_seq();
    auto& pcext = m_pic->m_picture_coding_extension;
    auto* pssext = m_pic->m_picture_spatial_scalable_extension;
    auto& ph = m_pic->m_picture_header;
    auto& se = seq->m_sequence_extension;
    auto& sh = seq->m_sequence_header;

    picture_structure = pcext.picture_structure;
    f_code[0][0] = pcext.f_code[0][0];
    f_code[0][1] = pcext.f_code[0][1];
    f_code[1][0] = pcext.f_code[1][0];
    f_code[1][1] = pcext.f_code[1][1];
    concealment_motion_vectors = pcext.concealment_motion_vectors;
    frame_pred_frame_dct = pcext.frame_pred_frame_dct;
    chroma_format = se.chroma_format;
    vertical_size_value = sh.vertical_size_value;
    picture_coding_type = ph.picture_coding_type;
    intra_vlc_format = pcext.intra_vlc_format;
    block_count = m_pic->block_count;

    dct_dc_pred_reset_value = predictor_reset_value[pcext.intra_dc_precision];

    if (pssext)
        spatial_temporal_weight_code_table_index = pssext->spatial_temporal_weight_code_table_index;
    return true;
}

void slice_c::decode_mb_modes(mb_data_t& mb_data) {
    auto& mb = mb_data.mb;
    if (!(mb.macroblock_type & macroblock_intra_bit) || mb.macroblock_address_increment > 1) {
        // reset DCT DC predictor
        dct_dc_pred[0] = dct_dc_pred_reset_value;
        dct_dc_pred[1] = dct_dc_pred_reset_value;
        dct_dc_pred[2] = dct_dc_pred_reset_value;
    }

    if (m_pic->m_picture_spatial_scalable_extension) {
        auto weight_class = local_spatial_temporal_weights_classes_tbl[spatial_temporal_weight_code_table_index][mb.spatial_temporal_weight_code];
        mb_data.spatial_temporal_weight_class = weight_class.spatial_temporal_weight_class;
        mb_data.spatial_temporal_weight_fract[0] = weight_class.spatial_temporal_weight_fract[0];
        mb_data.spatial_temporal_weight_fract[1] = weight_class.spatial_temporal_weight_fract[1];
        mb_data.spatial_temporal_integer_weight = weight_class.spatial_temporal_integer_weight;
    }
    if (mb.macroblock_type & macroblock_intra_bit)
    {
        mb_data.motion_vector_count = 0;
        mb_data.dmv = 0;
    }
    else
    {
        mb_data.motion_vector_count = 1;
        if(mb.macroblock_type & macroblock_motion_backward_bit);
            mb_data.motion_vector_count = 2;
        mb_data.dmv = 0;
        if (picture_structure == picture_structure_framepic) {
            switch (mb.frame_motion_type) {
            case 1:
                mb_data.mv_format = Field;
                mb_data.prediction_type = Field_based;
                break;
            case 2:
                mb_data.mv_format = Frame;
                mb_data.prediction_type = Frame_based;
                break;
            case 3:
                mb_data.mv_format = Field;
                mb_data.prediction_type = Dual_Prime;
                mb_data.dmv = 1;
                break;
            }
        }
        else {
            switch (mb.field_motion_type) {
            case 1:
                mb_data.mv_format = Field;
                mb_data.prediction_type = Field_based;
                break;
            case 2:
                mb_data.mv_format = Field;
                mb_data.prediction_type = MC16x8;
                break;
            case 3:
                mb_data.mv_format = Field;
                mb_data.prediction_type = Dual_Prime;
                mb_data.dmv = 1;
                break;
            }
        }
    }

    m_use_dct_one_table = (intra_vlc_format == 1) && (mb.macroblock_type & macroblock_intra_bit);
}

void slice_c::decode_mb_pattern(mb_data_t &mb_data) {
    auto& mb = mb_data.mb;
    bool macroblock_intra = mb.macroblock_type & macroblock_intra_bit;
    bool macroblock_pattern = mb.macroblock_type & macroblock_pattern_bit;
    uint32_t coded_block_pattern_1 = mb.coded_block_pattern_1;
    uint32_t coded_block_pattern_2 = mb.coded_block_pattern_2;
    uint32_t cbp = mb.coded_block_pattern_420;

    memset(mb_data.pattern_code, 0, sizeof(mb_data.pattern_code));
    for (int i = 0; i < 12; i++) {
        if (macroblock_intra)
            mb_data.pattern_code[i] = true;
    }
    if (macroblock_pattern) {
        for (int i = 0; i < 6; i++)
            if (cbp & (1 << (5 - i))) mb_data.pattern_code[i] = true;
        if (chroma_format == chroma_format_422)
            for (int i = 6; i < 8; i++)
                if (coded_block_pattern_1 & (1 << (7 - i))) mb_data.pattern_code[i] = true;
        if (chroma_format == chroma_format_444)
            for (int i = 6; i < 12; i++)
                if (coded_block_pattern_2 & (1 << (11 - i))) mb_data.pattern_code[i] = true;
    }
}

bool slice_c::parse_modes(macroblock_t &mb) {
    mb.macroblock_type = get_macroblock_type(m_bs, picture_coding_type);
    if ((mb.macroblock_type & spatial_temporal_weight_code_flag_bit) && (spatial_temporal_weight_code_table_index != 0)) {
        mb.spatial_temporal_weight_code = m_bs->read_next_bits(2);
    }
    if ((mb.macroblock_type & macroblock_motion_forward_bit) || (mb.macroblock_type & macroblock_motion_backward_bit)) {
        if (picture_structure == picture_structure_framepic) {
            if (frame_pred_frame_dct == 0)
                mb.frame_motion_type = m_bs->read_next_bits(2);
        }
        else {
            mb.field_motion_type = m_bs->read_next_bits(2);
        }
    }
    if ((picture_structure == picture_structure_framepic) && (frame_pred_frame_dct == 0) && 
        ((mb.macroblock_type & macroblock_intra_bit) || (mb.macroblock_type & macroblock_pattern_bit))) {
        mb.dct_type = m_bs->read_next_bits(1);
    }
    return true;
}

bool slice_c::parse_coded_block_pattern(macroblock_t& mb) {
    mb.coded_block_pattern_420 = get_coded_block_pattern(m_bs);
    if (chroma_format == chroma_format_422)
        mb.coded_block_pattern_1 = m_bs->read_next_bits(2);
    if (chroma_format == chroma_format_444)
        mb.coded_block_pattern_2 = m_bs->read_next_bits(6);
    return false;
}

bool slice_c::parse_motion_vector(mb_data_t& mb_data, int r, int s) {
    auto& mb = mb_data.mb;
    mb.motion_code[r][s][0] = get_motion_code(m_bs);
    if ((f_code[s][0] != 1) && (mb.motion_code[r][s][0] != 0))
        mb.motion_residual[r][s][0] = m_bs->read_next_bits(f_code[s][0] - 1);
    if (mb_data.dmv == 1)
        mb.dmvector[0] = get_dmvector(m_bs);
    mb.motion_code[r][s][1] = get_motion_code(m_bs);
    if ((f_code[s][1] != 1) && (mb.motion_code[r][s][1] != 0))
        mb.motion_residual[r][s][1] = m_bs->read_next_bits(f_code[s][1] - 1);
    if (mb_data.dmv == 1)
        mb.dmvector[1] = get_dmvector(m_bs);
    return true;
}

bool slice_c::parse_motion_vectors(mb_data_t& mb_data, int s) {
    auto& mb = mb_data.mb;
    if (mb_data.motion_vector_count == 1) {
        if ((mb_data.mv_format == Field) && (mb_data.dmv != 1))
            mb.motion_vertical_field_select[0][s] = m_bs->read_next_bits(1);
        parse_motion_vector(mb_data, 0, s);
    }
    else {
        mb.motion_vertical_field_select[0][s] = m_bs->read_next_bits(1);
        parse_motion_vector(mb_data, 0, s);
        mb.motion_vertical_field_select[1][s] = m_bs->read_next_bits(1);
        parse_motion_vector(mb_data, 1, s);
    }
    return true;
}

bool slice_c::parse_macroblock() {
    mb_data_t mb_data = { 0 };
    auto& mb = mb_data.mb;

    mb.macroblock_address_increment = 0;
    while (m_bs->get_next_bits(vlc_macroblock_escape_code.len) == vlc_macroblock_escape_code.value) {
        m_bs->skip_bits(vlc_macroblock_escape_code.len);
        mb.macroblock_address_increment += 33;
    }
    mb.macroblock_address_increment += get_macroblock_address_increment(m_bs);

    parse_modes(mb);
    decode_mb_modes(mb_data);

    if (mb.macroblock_type & macroblock_quant_bit)
        mb.quantiser_scale_code = m_bs->read_next_bits(5);
    if ((mb.macroblock_type & macroblock_motion_forward_bit) || ((mb.macroblock_type & macroblock_intra_bit) && concealment_motion_vectors))
        parse_motion_vectors(mb_data, 0);
    if ((mb.macroblock_type & macroblock_motion_backward_bit) != 0)
        parse_motion_vectors(mb_data, 1);
    if ((mb.macroblock_type & macroblock_intra_bit != 0) && concealment_motion_vectors)
        m_bs->skip_bits(1);
    if (mb.macroblock_type & macroblock_pattern_bit)
        parse_coded_block_pattern(mb);
    decode_mb_pattern(mb_data);

    // TODO: try to unroll loop
    if (m_use_dct_one_table)
        for (int i = 0; i < block_count; i++)
            parse_block<true>(m_bs, mb_data, i, dct_dc_pred);
    else
        for (int i = 0; i < block_count; i++)
            parse_block<true>(m_bs, mb_data, i, dct_dc_pred);

    macroblocks.push_back(mb_data);
    return true;
}

bool slice_c::parse_slice() {
    auto* seq = m_pic->get_seq();

    // reset predictor
    dct_dc_pred[0] = dct_dc_pred_reset_value;
    dct_dc_pred[1] = dct_dc_pred_reset_value;
    dct_dc_pred[2] = dct_dc_pred_reset_value;

    slice.slice_start_code = m_bs->read_next_bits(32);
    if (vertical_size_value > 2800)
        slice.slice_vertical_position_extension = m_bs->read_next_bits(3);
    if (seq->m_sequence_scalable_extension && seq->m_sequence_scalable_extension->scalable_mode == scalable_mode_data_partitioning)
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
        auto& slice = m_slices.back();
        slice.init_slice();
        slice.parse_slice();
    } while (local_next_start_code(m_bs) >= slice_start_code_min && local_next_start_code(m_bs) <= slice_start_code_max);
    local_find_start_code(m_bs);
    return true;
}

bool video_sequence_c::parse_extension_and_user_data(extension_after_code_e after_code, picture_c* pic) {
    while ((local_next_start_code(m_bs) == extension_start_code) || (local_next_start_code(m_bs) == user_data_start_code)) {
        if ((after_code != after_group_of_picture_header) && (local_next_start_code(m_bs) == extension_start_code))
            parse_extension_data(after_code, pic);
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

bool video_sequence_c::parse_extension_data(extension_after_code_e after_code, picture_c* pic) {
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

bool video_sequence_c::parse() {
    CHECK(local_next_start_code(m_bs) == sequence_header_code);
    CHECK(parse_sequence_header(m_bs, m_sequence_header));

    if (local_next_start_code(m_bs) == extension_start_code) {
        parse_sequence_extension(m_bs, m_sequence_extension);
        do {
            parse_extension_and_user_data(after_sequence_extension, nullptr);
            do {
                if (local_next_start_code(m_bs) == group_start_code) {
                    parse_group_of_pictures_header(m_bs, *(m_group_of_pictures_header = new group_of_pictures_header_t));
                    parse_extension_and_user_data(after_group_of_picture_header, nullptr);
                }
                parse_picture_data();
                return true; // remove it after test complete
            } while ((local_next_start_code(m_bs) == picture_start_code) || (local_next_start_code(m_bs) == group_start_code));
            if (local_next_start_code(m_bs) != sequence_end_code) {
                parse_sequence_header(m_bs, m_sequence_header);
                parse_sequence_extension(m_bs, m_sequence_extension);
            }
        } while (local_next_start_code(m_bs) != sequence_end_code);
    }
    else
        return false; // MPEG1 not support

    return true;
}

//[chroma_format]
uint32_t block_count_tbl[4] = { 0 /*invalid chroma format*/, 6, 8, 12 };

bool video_sequence_c::parse_picture_data() {
    /* Decode sequence parameters*/
    picture_c pic(m_bs, this);
    pic.block_count = block_count_tbl[m_sequence_extension.chroma_format];

    parse_picture_header(m_bs, pic.m_picture_header);
    parse_picture_coding_extension(m_bs, pic.m_picture_coding_extension);
    parse_extension_and_user_data(after_picture_coding_extension, &pic);
    pic.parse_picture();

    m_pictures.push_back(pic);
    m_pictures_queue.push(&m_pictures.back());
    return true;
}

bool mp2v_parser_c::parse() {
    CHECK(m_video_sequence.parse());
    return true;
}