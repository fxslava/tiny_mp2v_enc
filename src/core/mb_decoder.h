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
    int quantiser_scale;
    int intra_dc_prec;
    int intra_vlc_format;

#ifdef _DEBUG
    macroblock_t mb;
#endif
};

typedef bool (*parse_macroblock_func_t)(bitstream_reader_c* m_bs, macroblock_context_cache_t &cache);

parse_macroblock_func_t select_parse_macroblock_func(uint8_t picture_coding_type, uint8_t picture_structure, uint8_t frame_pred_frame_dct, uint8_t concealment_motion_vectors, uint8_t chroma_format, bool q_scale_type, bool alt_scan);
