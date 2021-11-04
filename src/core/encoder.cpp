#include "encoder.h"

int frame_rate_code_tbl[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

static int create_time_code(int frame_num, float frame_rate, bool drop_flag = false) {
    int pictures = frame_num % 60;
    int total_seconds  = (int)floorf((float)frame_num / frame_rate);
    int seconds = total_seconds % 60;
    int total_minutes = total_seconds / 60;
    int minutes = total_minutes % 60;
    int total_hours = total_minutes / 60;
    int hours = total_hours % 24;
    return pictures + (seconds << 6) + (1 << 12) + (minutes << 13) + (hours << 19) + (drop_flag ? (1 << 24) : 0);
}

static int create_time_code(int frame_num, bool drop_flag = false) {
    int pictures = frame_num % 60;
    return pictures + (1 << 12) + (drop_flag ? (1 << 24) : 0);
}

void mp2v_encoder_c::default_settings() {
    mp2v_config_t config = {
        1920, 1088,
        1920, 1080,
        chroma_format_422,
        8, // num_threads
        {50000000, 50000000, 1088},
        {false, frame_rate_60}
    };
    init_encoder(config);
}

bool mp2v_encoder_c::validate_settings(mp2v_config_t& config) {
    return true;
}

void mp2v_encoder_c::init_encoder(mp2v_config_t& config) {

    int bit_rate = (config.rc_config.avg_bitrate + 399) / 400;

    seq_hdr.horizontal_size_value = config.coding_width & 0xfff;
    seq_hdr.vertical_size_value = config.coding_height & 0xfff;
    seq_hdr.aspect_ratio_information = 3; // 16:9
    seq_hdr.frame_rate_code = frame_rate_code_tbl[config.latency_config.frame_rate]; //60 FPS
    seq_hdr.bit_rate_value = bit_rate & 0x0003ffff; // lower 18 bit of bitrate
    seq_hdr.vbv_buffer_size_value = config.rc_config.vbv_buffer_size & 0x3ff;
    seq_hdr.constrained_parameters_flag = 0; //This flag (used in ISO/IEC 11172-2) has no meaning in this Specification and shall have the value '0'.
    seq_hdr.load_intra_quantiser_matrix = 0;
    seq_hdr.load_non_intra_quantiser_matrix = 0;

    seq_ext.profile_and_level_indication = 130; // 4:2:2 @ High level
    seq_ext.progressive_sequence = 0;
    seq_ext.chroma_format = config.chroma_format;
    seq_ext.horizontal_size_extension = config.coding_width >> 12;
    seq_ext.vertical_size_extension = config.coding_height >> 12;
    seq_ext.bit_rate_extension = bit_rate >> 18; // upper 12 bit of bitrate
    seq_ext.vbv_buffer_size_extension = config.rc_config.vbv_buffer_size >> 10;
    seq_ext.low_delay = config.latency_config.low_delay;
    seq_ext.frame_rate_extension_n = 0;
    seq_ext.frame_rate_extension_d = 0;

    seq_display_ext.video_format = 0; // component
    seq_display_ext.colour_description = 0;
    seq_display_ext.display_horizontal_size = config.display_width;
    seq_display_ext.display_vertical_size = config.display_height;
}

void mp2v_encoder_c::encode_video_sequence() {
    
    write_sequence_header(&m_bs, seq_hdr);
    write_sequence_extension(&m_bs, seq_ext);
    write_sequence_display_extension(&m_bs, seq_display_ext);

    int frame_num = 0;
    frame_t frame;
    while (frame_queue.pop(frame) != ThreadSafeQ<frame_t>::QueueResult::CLOSED) {
        group_of_pictures_header_t gop;
        gop.broken_link = 0;
        gop.closed_gop = 1;
        gop.time_code = create_time_code(frame_num++);
        write_group_of_pictures_header(&m_bs, gop);
        encode_I_frame(frame);
    }
    
    m_bs.write_bits(START_CODE(sequence_end_code), 32);
}

void mp2v_encoder_c::encode_I_frame(frame_t frame) {

}
