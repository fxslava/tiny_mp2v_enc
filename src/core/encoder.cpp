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

static void make_macroblock_yuv_ptrs(uint8_t* (&yuv)[3], frame_t frame, int mb_row, int chroma_format) {
    yuv[0] = frame.planes[0] + mb_row * 16 * frame.strides[0];
    switch (chroma_format) {
    case chroma_format_420:
        yuv[1] = frame.planes[1] + mb_row * 8 * frame.strides[1];
        yuv[2] = frame.planes[2] + mb_row * 8 * frame.strides[2];
        break;
    case chroma_format_422:
        yuv[1] = frame.planes[1] + mb_row * 16 * frame.strides[1];
        yuv[2] = frame.planes[2] + mb_row * 16 * frame.strides[2];
        break;
    case chroma_format_444:
        yuv[1] = frame.planes[1] + mb_row * 16 * frame.strides[1];
        yuv[2] = frame.planes[2] + mb_row * 16 * frame.strides[2];
        break;
    }
}

void mp2v_encoder_c::default_settings() {
    mp2v_config_t config = {
        1920, 1080,
        chroma_format_422,
        8, // num_threads
        intra_dc_prec_9bit,
        {rc_mode_const_q, 50000000, 50000000, 1088, 4, 15},
        {false, frame_rate_60}
    };
    init_encoder(config);
}

bool mp2v_encoder_c::validate_settings(mp2v_config_t& config) {
    return true;
}

void mp2v_encoder_c::init_encoder(mp2v_config_t& config) {

    memcpy(&cfg, &config, sizeof(mp2v_config_t));

    int bit_rate = (config.rc_config.avg_bitrate + 399) / 400;
    coding_width = (config.width + 15) & ~15;
    coding_height = (config.height + 15) & ~15;

    seq_hdr.horizontal_size_value = config.width & 0xfff;
    seq_hdr.vertical_size_value = config.height & 0xfff;
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
    seq_ext.horizontal_size_extension = config.width >> 12;
    seq_ext.vertical_size_extension = config.height >> 12;
    seq_ext.bit_rate_extension = bit_rate >> 18; // upper 12 bit of bitrate
    seq_ext.vbv_buffer_size_extension = config.rc_config.vbv_buffer_size >> 10;
    seq_ext.low_delay = config.latency_config.low_delay;
    seq_ext.frame_rate_extension_n = 0;
    seq_ext.frame_rate_extension_d = 0;

    seq_display_ext.video_format = 0; // component
    seq_display_ext.colour_description = 0;
    seq_display_ext.display_horizontal_size = config.width;
    seq_display_ext.display_vertical_size = config.height;

    mb_height = (seq_ext.progressive_sequence) ? (config.height + 15) / 16 : 2 * ((config.height + 31) / 32);
    mb_width = (config.width + 15) / 16;
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
    
    m_bs.align();
    m_bs.write_bits(START_CODE(sequence_end_code), 32);
    m_bs.flush();
}

void mp2v_encoder_c::encode_I_frame(frame_t frame) {
    picture_header_t ph;
    ph.temporal_reference = 0;
    ph.picture_coding_type = picture_coding_type_intra;
    ph.vbv_delay = 0xffff;
    write_picture_header(&m_bs, ph);

    picture_coding_extension_t pic_coding_ext;
    pic_coding_ext.f_code[0][0] = 15;
    pic_coding_ext.f_code[0][1] = 15;
    pic_coding_ext.f_code[1][0] = 15;
    pic_coding_ext.f_code[1][1] = 15;
    pic_coding_ext.intra_dc_precision = cfg.intra_dc_prec;
    pic_coding_ext.picture_structure = picture_structure_framepic;
    pic_coding_ext.top_field_first = 1;
    pic_coding_ext.frame_pred_frame_dct = 0;
    pic_coding_ext.concealment_motion_vectors = 0;
    pic_coding_ext.q_scale_type = 1;
    pic_coding_ext.intra_vlc_format = 1;
    pic_coding_ext.alternate_scan = 1;
    pic_coding_ext.repeat_first_field = 0;
    pic_coding_ext.chroma_420_type = (chroma_format_420 == cfg.chroma_format) ? 1 : 0;
    pic_coding_ext.progressive_frame = 0;
    pic_coding_ext.composite_display_flag = 0;
    write_picture_coding_extension(&m_bs, pic_coding_ext);

    int qp = cfg.rc_config.intra_qp;

    for (int y = 0; y < mb_height; y++) {
        uint8_t* yuv_ptrs[3];
        make_macroblock_yuv_ptrs(yuv_ptrs, frame, y, cfg.chroma_format);
        encode_slice(y, yuv_ptrs, frame.strides, qp);
    }
}

void mp2v_encoder_c::encode_slice(int row_idx, uint8_t* yuv_ptrs[3], int strides[3], int qp) {
    slice_t slice;
    slice.slice_start_code = (row_idx & 0x7f) + 1;
    slice.slice_vertical_position_extension = (row_idx >> 7);
    slice.quantiser_scale_code = qp;
    slice.slice_extension_flag = 0;

    write_slice_header(&m_bs, slice, seq_hdr, nullptr);
}
