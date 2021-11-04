#pragma once
#include "common/cpu.hpp"
#include "common/queue.hpp"
#include <vector>
#include <thread>
#include "mp2v_hdr.h"
#include "bitstream_writer.h"

enum frame_rate_e {
    frame_rate_23 = 0, // 23.976
    frame_rate_24,
    frame_rate_25,
    frame_rate_29,     // 29.97
    frame_rate_30,
    frame_rate_50,
    frame_rate_59,     // 59.94
    frame_rate_60
};

struct mp2v_rc_config_t {
    int max_bitrate;
    int avg_bitrate;
    int vbv_buffer_size;
};

struct mp2v_latency_config_t {
    bool low_delay;
    frame_rate_e frame_rate;
};

struct mp2v_config_t {
    int coding_width;
    int coding_height;
    int display_width;
    int display_height;
    int chroma_format;
    int num_threads;
    mp2v_rc_config_t rc_config;
    mp2v_latency_config_t latency_config;
};

struct frame_t {
    uint8_t* planes[3];
    int strides[3];
};

class mp2v_encoder_c {
public:
    mp2v_encoder_c() : encode_thread(encode_proc, this) { default_settings(); };

    void default_settings();
    bool validate_settings(mp2v_config_t& config);
    void init_encoder(mp2v_config_t& config);

    void put_frame(frame_t frame) {
        frame_queue.push(frame);
    }
    void flush() {
        frame_queue.close();
        if (encode_thread.joinable())
            encode_thread.join();
    }

    void output_bitsream(uint32_t*& buffer, size_t& size) {
        m_bs.output_bitsream(buffer, size);
    }

private:
    void encode_I_frame(frame_t frame);
    void encode_video_sequence();
    static void encode_proc(mp2v_encoder_c* enc) {
        enc->encode_video_sequence();
    }

private:
    bitstream_writer_c m_bs;
    sequence_header_t seq_hdr;
    sequence_extension_t seq_ext;
    sequence_display_extension_t seq_display_ext;

    ThreadSafeQ<frame_t> frame_queue;
    std::thread encode_thread;
};
