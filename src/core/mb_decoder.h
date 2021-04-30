// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "parser.h"

typedef void (*decode_macroblock_func_t)(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code);

decode_macroblock_func_t select_decode_macroblock(int chroma_format, bool q_scale_type, bool alt_scan);