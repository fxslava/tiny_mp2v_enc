// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include "mb_decoder.h"
#include "scan.h"
#include "quant.h"
#include "idct.h"

template<typename pixel_t, bool alt_scan, bool intra>
void decode_block_template(pixel_t* plane, uint32_t stride, int16_t QFS[64], uint16_t W_i[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_prec) {
    int16_t QF[64];
    int16_t F[64];

    // inverse scan
    if (alt_scan)
        inverse_alt_scan_c(QF, QFS);
    else
        inverse_scan_c(QF, QFS);

    // dequantization
    if (!intra)
        inverse_quant_c(F, QF, W, quantizer_scale);
    else
        inverse_quant_intra_c(F, QF, W_i, quantizer_scale, intra_dc_prec);

    //idct
    inverse_dct(plane, F, stride);
}

template<
    int chroma_format, 
    bool q_scale_type, 
    bool alt_scan>
void decode_macroblock_template(
    uint8_t* yuv_planes[3], 
    int stride,
    int chroma_stride,
    mb_data_t mb_data, 
    uint16_t W[4][64],
    uint8_t intra_dc_prec,
    int &quant_scale_code) {

    uint8_t quantiser_scale_code = (mb_data.mb.macroblock_type & macroblock_quant_bit) ? mb_data.mb.quantiser_scale_code : quant_scale_code;
    quant_scale_code = quantiser_scale_code;

    uint8_t quantizer_scale;
    if (q_scale_type) {
        if (quantiser_scale_code < 9)
            quantizer_scale = quantiser_scale_code;
        else if (quantiser_scale_code < 17)
            quantizer_scale = (quantiser_scale_code - 4) << 1;
        else if (quantiser_scale_code < 25)
            quantizer_scale = (quantiser_scale_code - 10) << 2;
        else
            quantizer_scale = (quantiser_scale_code - 17) << 3;
    }
    else
        quantizer_scale = quantiser_scale_code << 1;

    bool mb_intra = mb_data.mb.macroblock_type & macroblock_intra_bit;

    if (mb_intra) {
        if (mb_data.pattern_code[0])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[0], stride, mb_data.QFS[0], W[0], W[1], quantizer_scale, intra_dc_prec);
        if (mb_data.pattern_code[1])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[0] + 8, stride, mb_data.QFS[1], W[0], W[1], quantizer_scale, intra_dc_prec);
        if (mb_data.pattern_code[2])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[0] + 8 * stride, stride, mb_data.QFS[2], W[0], W[1], quantizer_scale, intra_dc_prec);
        if (mb_data.pattern_code[3])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[0] + 8 * (stride + 1), stride, mb_data.QFS[3], W[0], W[1], quantizer_scale, intra_dc_prec);

        if (chroma_format == 1) {
            if (mb_data.pattern_code[4])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[1], chroma_stride, mb_data.QFS[4], W[0], W[1], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[5])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[2], chroma_stride, mb_data.QFS[5], W[0], W[1], quantizer_scale, intra_dc_prec);
        }
        if (chroma_format == 2) {
            if (mb_data.pattern_code[4])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[1], chroma_stride, mb_data.QFS[4], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[5])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[1] + 8 * chroma_stride, chroma_stride, mb_data.QFS[5], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[6])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[2], chroma_stride, mb_data.QFS[6], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[7])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[2] + 8 * chroma_stride, chroma_stride, mb_data.QFS[7], W[2], W[3], quantizer_scale, intra_dc_prec);
        }
        if (chroma_format == 3) {
            if (mb_data.pattern_code[4])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[1], stride, mb_data.QFS[4], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[5])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[1] + 8, stride, mb_data.QFS[5], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[6])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[1] + 8 * stride, stride, mb_data.QFS[6], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[7])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[1] + 8 * (stride + 1), stride, mb_data.QFS[7], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[8])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[2], stride, mb_data.QFS[8], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[9])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[2] + 8, stride, mb_data.QFS[9], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[10])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[2] + 8 * stride, stride, mb_data.QFS[10], W[2], W[3], quantizer_scale, intra_dc_prec);
            if (mb_data.pattern_code[11])
                decode_block_template<uint8_t, alt_scan, true>(yuv_planes[2] + 8 * (stride + 1), stride, mb_data.QFS[11], W[2], W[3], quantizer_scale, intra_dc_prec);
        }
    }
}

void decode_420_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_420, false, false>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_420_alts_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_420, false, true>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_420_qst_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_420, true, false>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_420_qst_alts_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_420, true, true>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_422_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_422, false, false>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_422_alts_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_422, false, true>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_422_qst_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_422, true, false>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_422_qst_alts_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_422, true, true>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_444_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_444, false, false>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_444_alts_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_444, false, true>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_444_qst_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_444, true, false>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}
void decode_444_qst_alts_macroblock(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quantiser_scale_code) {
    decode_macroblock_template<chroma_format_444, true, true>(yuv_planes, stride, chroma_stride, mb_data, W, intra_dc_prec, quantiser_scale_code);
}

decode_macroblock_func_t select_decode_macroblock(int chroma_format, bool q_scale_type, bool alt_scan) {
    switch (chroma_format) {
    case chroma_format_420:
        if (!q_scale_type) {
            if (!alt_scan) return decode_420_macroblock;
            else           return decode_420_alts_macroblock;
        }
        else {
            if (!alt_scan) return decode_420_qst_macroblock;
            else           return decode_420_qst_alts_macroblock;
        }
    case chroma_format_422:
        if (!q_scale_type) {
            if (!alt_scan) return decode_422_macroblock;
            else           return decode_422_alts_macroblock;
        }
        else {
            if (!alt_scan) return decode_422_qst_macroblock;
            else           return decode_422_qst_alts_macroblock;
        }
    case chroma_format_444:
        if (!q_scale_type) {
            if (!alt_scan) return decode_444_macroblock;
            else           return decode_444_alts_macroblock;
        }
        else {
            if (!alt_scan) return decode_444_qst_macroblock;
            else           return decode_444_qst_alts_macroblock;
        }
    }
}