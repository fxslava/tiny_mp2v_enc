// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.

#include <thread>
#include <chrono>
#include <iostream>
#include "sample_args.h"
#include "bitstream.h"
#include "core/decoder.h"

#if defined(WIN32)
#include <ittnotify.h>
#endif

void stream_writer_func(mp2v_decoder_c* mp2v_decoder, std::string output_file) {
    FILE* fp = fopen(output_file.c_str(), "wb");

    frame_c* frame = nullptr;
    mp2v_decoder->get_decoded_frame(frame);

    while (frame) {
        /*
        for (int i = 0; i < 3; i++) {
            uint8_t* plane = frame->get_planes(i);
            for (int y = 0; y < frame->get_height(i); y++, plane += frame->get_strides(i))
                fwrite(plane, 1, frame->get_width(i), fp);
        }*/

        mp2v_decoder->release_frame(frame);
        mp2v_decoder->get_decoded_frame(frame);
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

    std::string *bitstream_file, *output_file;
    std::vector<arg_desc_t> args_desc{
        { "-v", "Input MPEG2 elementary bitsream file", ARG_TYPE_TEXT, &bitstream_file },
        { "-o", "Output YUV stream", ARG_TYPE_TEXT, &output_file }
    };
    args_parser cmd_parser(args_desc);
    cmd_parser.parse(argc, argv);

    if (bitstream_file) {
        bitstream_reader_c stream_reader(*bitstream_file);
        mp2v_decoder_c mp2v_decoder(&stream_reader);

        std::thread stream_writer(stream_writer_func, &mp2v_decoder, *output_file);
        mp2v_decoder.decoder_init(&config);

        const auto start = std::chrono::system_clock::now();

#if defined(WIN32)
        __itt_resume();
        mp2v_decoder.decode();
        __itt_pause();
#else
        mp2v_decoder.decode();
#endif

        const auto end = std::chrono::system_clock::now();
        auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        printf("Time = %.2f ms\n", static_cast<double>(elapsed_ms.count()));

        stream_writer.join();
    }
}