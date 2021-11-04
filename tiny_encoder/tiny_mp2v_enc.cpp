// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.

#include <thread>
#include <chrono>
#include <iostream>
#include "sample_args.h"
#include "core/encoder.h"

frame_t read_422_yuv(FILE* fp, int width, int height) {
    frame_t res;

    int stride = (width + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
    int chroma_stride = ((stride / 2) + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
    res.planes[0] = new uint8_t[stride * height];
    res.planes[1] = new uint8_t[chroma_stride * height];
    res.planes[2] = new uint8_t[chroma_stride * height];
    res.strides[0] = stride;
    res.strides[1] = chroma_stride;
    res.strides[2] = chroma_stride;

    uint8_t* luma_plane = res.planes[0];
    uint8_t* u_plane    = res.planes[1];
    uint8_t* v_plane    = res.planes[2];
    for (int y = 0; y < height; y++, luma_plane += stride)
        fread(luma_plane, 1, width, fp);

    for (int y = 0; y < height; y++, u_plane += chroma_stride)
        fread(u_plane, 1, width / 2, fp);
    for (int y = 0; y < height; y++, v_plane += chroma_stride)
        fread(v_plane, 1, width / 2, fp);

    return res;
}

int main(int argc, char* argv[])
{
    std::string* input_yuv = nullptr, * output_bitsream = nullptr;
    int width, height;
    args_parser cmd_parser({
        { "-i", "Input YUV stream", ARG_TYPE_TEXT, &input_yuv },
        { "-w", "Width", ARG_TYPE_INT, &width },
        { "-h", "Height", ARG_TYPE_INT, &height },
        { "-o", "Output MPEG2 elementary bitsream file", ARG_TYPE_TEXT, &output_bitsream }
        }, argc, argv);

    if (input_yuv) {
        mp2v_encoder_c encoder;

        FILE* fp = fopen(input_yuv->c_str(), "rb");
        while (!feof(fp))
            encoder.put_frame(read_422_yuv(fp, width, height));
        fclose(fp);

        encoder.flush();

        if (output_bitsream) {
            uint32_t* bitstream;
            size_t size;
            encoder.output_bitsream(bitstream, size);
            FILE* fout = fopen(output_bitsream->c_str(), "wb");
            fwrite(bitstream, 1, size * 4, fout);
            fclose(fout);
        }
    }
}