// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "mp2v_hdr.h"

struct mb_data_t {
    macroblock_t mb;
    bool pattern_code[12];
    int16_t MVs[2][2][2];
    int16_t QFS[12][64];
};

typedef void (*decode_macroblock_func_t)(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t &mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code, uint8_t* ref0[3], uint8_t* ref1[3]);

decode_macroblock_func_t select_decode_macroblock(int chroma_format, bool q_scale_type, bool alt_scan);