// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include "parser.h"
#include "mp2v_hdr.h"
#include "mp2v_vlc_tbl.h"

#define CHECK(p) { if (!(p)) return false; }

static uint8_t local_find_start_code(bitstream_reader_i* bs) {
    bs->seek_pattern(vlc_start_code.value, vlc_start_code.len);
    return (uint8_t)bs->read_next_bits(8);
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

bool video_sequence_c::parse_sequence_header() {
    m_sequence_header->sequence_header_code = m_bs->read_next_bits(32);
    m_sequence_header->horizontal_size_value = m_bs->read_next_bits(12);
    m_sequence_header->vertical_size_value = m_bs->read_next_bits(12);
    m_sequence_header->aspect_ratio_information = m_bs->read_next_bits(4);
    m_sequence_header->frame_rate_code = m_bs->read_next_bits(4);
    m_sequence_header->bit_rate_value = m_bs->read_next_bits(18);
    m_bs->skip_bits(1);
    m_sequence_header->vbv_buffer_size_value = m_bs->read_next_bits(10);
    m_sequence_header->constrained_parameters_flag = m_bs->read_next_bits(1);
    m_sequence_header->load_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (m_sequence_header->load_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, m_sequence_header->intra_quantiser_matrix);
    m_sequence_header->load_non_intra_quantiser_matrix = m_bs->read_next_bits(1);
    if (m_sequence_header->load_non_intra_quantiser_matrix)
        local_copy_array<uint8_t, 64>(m_bs, m_sequence_header->non_intra_quantiser_matrix);

    return true;
}

bool video_sequence_c::parse_sequence_extension() {
    m_sequence_extension->extension_start_code = m_bs->read_next_bits(32);
    m_sequence_extension->extension_start_code_identifier = m_bs->read_next_bits(4);
    m_sequence_extension->profile_and_level_indication = m_bs->read_next_bits(8);
    m_sequence_extension->progressive_sequence = m_bs->read_next_bits(1);
    m_sequence_extension->chroma_format = m_bs->read_next_bits(2);
    m_sequence_extension->horizontal_size_extension = m_bs->read_next_bits(2);
    m_sequence_extension->vertical_size_extension = m_bs->read_next_bits(2);
    m_sequence_extension->bit_rate_extension = m_bs->read_next_bits(12);
    m_bs->skip_bits(1);
    m_sequence_extension->vbv_buffer_size_extension = m_bs->read_next_bits(8);
    m_sequence_extension->low_delay = m_bs->read_next_bits(1);
    m_sequence_extension->frame_rate_extension_n = m_bs->read_next_bits(2);
    m_sequence_extension->frame_rate_extension_d = m_bs->read_next_bits(5);

    return true;
}

bool video_sequence_c::parse_extension_and_user_data(extension_after_code_e after_code) {
    while ((local_next_start_code(m_bs) == extension_start_code) || (local_next_start_code(m_bs) == user_data_start_code)) {
        if ((after_code != after_group_of_picture_header) && (local_next_start_code(m_bs) == extension_start_code));
            //extension_data(i);
            if (local_next_start_code(m_bs) == user_data_start_code);
                //user_data();
    }
    return true;
}

bool video_sequence_c::parse_extension_data(extension_after_code_e after_code) {
    while (local_next_start_code(m_bs) == extension_start_code) {
        m_bs->skip_bits(32); //extension_start_code 32 bslbf
        if (after_code == after_sequence_extension) {
            uint8_t ext_id = m_bs->read_next_bits(4);
            switch (ext_id)
            {
            case sequence_display_extension_id:
                break;
            case sequence_scalable_extension_id:
                break;
            default:
                return false; // Unsupported extension id
            }
        }
        if (after_code == after_picture_coding_extension) {
            uint8_t ext_id = m_bs->read_next_bits(4);
            switch (ext_id)
            {
            case quant_matrix_extension_id:
                break;
            case copiright_extension_id:
                break;
            case picture_display_extension_id:
                break;
            case picture_spatial_scalable_extension_id:
                break;
            case picture_temporal_scalable_extension_id:
                break;
            default:
                return false; // Unsupported extension id
            }
        }
    }
}

bool video_sequence_c::parse() {
    CHECK(local_next_start_code(m_bs) == sequence_header_code);
    CHECK(parse_sequence_header());

    if (local_next_start_code(m_bs) == extension_start_code) {
        parse_sequence_extension();
    }
    else
        return false; // MPEG1 not support
    return true;
}

bool mp2v_parser_c::parse() {
    CHECK(m_video_sequence.parse());
    return true;
}