// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include "mp2v_hdr.h"

enum reference_type_t  {
    REF_TYPE_SRC = 0,
    REF_TYPE_L0  = 1,
    REF_TYPE_L1  = 2,
};

struct macroblock_context_cache_t {
    uint16_t W[4][64];
    uint32_t f_code[2][2]; 
    int16_t  PMVs[2][2][2];
    uint16_t dct_dc_pred[3];
    uint8_t* yuv_planes[3][3];
    int spatial_temporal_weight_code_table_index;
    int luma_stride;
    int chroma_stride;
    int quantiser_scale_code;
    int intra_dc_prec;
};

struct mb_data_t {
    macroblock_t mb;
    bool pattern_code[12];
    int16_t MVs[2][2][2];
    int16_t QFS[12][64];
};

typedef void (*decode_macroblock_func_t)(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t &mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code, uint8_t* ref0[3], uint8_t* ref1[3]);
typedef bool (*parse_macroblock_func_t)(bitstream_reader_c* m_bs, macroblock_t& mb, int spatial_temporal_weight_code_table_index, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2]);

decode_macroblock_func_t select_decode_macroblock(int chroma_format, bool q_scale_type, bool alt_scan);
parse_macroblock_func_t select_parse_macroblock_func(uint8_t picture_coding_type, uint8_t picture_structure, uint8_t frame_pred_frame_dct, uint8_t concealment_motion_vectors, uint8_t chroma_format, bool q_scale_type, bool alt_scan);