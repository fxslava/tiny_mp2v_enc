// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.

#include <iostream>
#include "sample_args.h"
#include "bitstream.h"
#include "core/decoder.h"

class bitstream_file_reader: public bitstream_reader_i {
private:
    void read32() {
        buffer <<= 32;
        uint32_t tmp;
        fread(&tmp, sizeof(uint32_t), 1, bitstream);
        buffer |= (uint64_t)_byteswap_ulong(tmp);
        buffer_idx -= 32;
    }
    void read64() {
        uint64_t tmp;
        fread(&tmp, sizeof(uint64_t), 1, bitstream);
        buffer = _byteswap_uint64(tmp);
        buffer_idx = 0;
    }
    void update_buffer() {
        if (buffer_idx == 64)
            read64();
        else if (buffer_idx >= 32)
            read32();
    }
public:
    bitstream_file_reader(std::string filename) : 
        bitstream(nullptr),
        buffer(0),
        buffer_idx(64)
    {
        bitstream = fopen(filename.c_str(), "rb");
    }
    ~bitstream_file_reader() {
        fclose(bitstream);
    }

    uint32_t get_next_bits(int len) {
        update_buffer();
        uint64_t mask = (1ll << len) - 1;
        uint64_t tmp = buffer << buffer_idx;
        return (tmp >> (64 - len)) & mask;
    }

    uint32_t read_next_bits(int len) {
        uint32_t tmp = get_next_bits(len);
        buffer_idx += len;
        return tmp;
    }

    bool seek_pattern(uint32_t pattern, int len) {
        do {
            update_buffer();
            int range = 64 - buffer_idx - len;
            uint64_t tmp = buffer << buffer_idx;
            uint64_t mask = (1ll << len) - 1;
            int offset = 64 - len;
            for (int pos = 0; pos < range; pos++) {
                if ((uint32_t)((tmp >> offset) & mask) == pattern) {
                    buffer_idx += pos;
                    return true;
                }
                tmp <<= 1;
            }
            buffer_idx += range;
        } while (!feof(bitstream));
        return false;
    }

    void skip_bits(int len) {
        buffer_idx += len;
    }

private:
    FILE* bitstream = nullptr;
    uint64_t buffer = 0;
    uint32_t buffer_idx = 64;
};

void write_frame(frame_c* frame, char* fname) {
    FILE* fp = fopen(fname, "ab");
    for (int i = 0; i < 3; i++) {
        uint8_t* plane = frame->get_planes(i);
        for (int y = 0; y < frame->get_height(i); y++, plane += frame->get_strides(i))
            fwrite(plane, 1, frame->get_width(i), fp);
    }
    fclose(fp);
}

int main(int argc, char* argv[])
{
    decoder_config_t config;
    config.width = 1920;
    config.height = 1088;
    config.chroma_format = 2;
    config.frames_pool_size = 16;

    std::string *bitstream_file;
    std::vector<arg_desc_t> args_desc{{ "-v", "Input MPEG2 elementary bitsream file", ARG_TYPE_TEXT, &bitstream_file }};
    args_parser cmd_parser(args_desc);
    cmd_parser.parse(argc, argv);

    if (bitstream_file) {
        bitstream_file_reader stream_reader(*bitstream_file);
        mp2v_decoder_c mp2v_decoder(&stream_reader);
        frame_c* frame = nullptr;

        mp2v_decoder.decoder_init(&config);
        mp2v_decoder.decode();

        for (int i = 0; i < 4; i++) {
            mp2v_decoder.get_decoded_frame(frame);
            write_frame(frame, "test.yuv");
        }
    }
}