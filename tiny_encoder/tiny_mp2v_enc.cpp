// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.

#include <thread>
#include <chrono>
#include <iostream>
#include "sample_args.h"
#include "core/encoder.h"

int main(int argc, char* argv[])
{
    std::string* input_yuv = nullptr, * output_file = nullptr;
    args_parser cmd_parser({
        { "-v", "Input YUV stream", ARG_TYPE_TEXT, &input_yuv },
        { "-o", "Output MPEG2 elementary bitsream file", ARG_TYPE_TEXT, &output_bitsream }
        }, argc, argv);

    if (input_yuv) {
    }
}