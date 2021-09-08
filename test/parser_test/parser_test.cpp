// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <thread>
#include "sample_args.h"
#include "bitstream.h"
#include "core/decoder.h"

void stream_writer_func(mp2v_decoder_c* mp2v_decoder) {
    FILE* fp = fopen("test.yuv", "wb");

    frame_c* frame = nullptr;
    while (!mp2v_decoder->get_decoded_frame(frame))
        mp2v_decoder->wait_for_frame();

    while (frame) {

        for (int i = 0; i < 3; i++) {
            uint8_t* plane = frame->get_planes(i);
            for (int y = 0; y < frame->get_height(i); y++, plane += frame->get_strides(i))
                fwrite(plane, 1, frame->get_width(i), fp);
        }

        mp2v_decoder->release_frame(frame);

        while (!mp2v_decoder->get_decoded_frame(frame))
            mp2v_decoder->wait_for_frame();
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
        bitstream_reader_c stream_reader(*bitstream_file);
        mp2v_decoder_c mp2v_decoder(&stream_reader);

        std::thread stream_writer(stream_writer_func, &mp2v_decoder);
        mp2v_decoder.decoder_init(&config);
        mp2v_decoder.decode();
        stream_writer.join();
    }
}