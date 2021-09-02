// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include "mb_decoder.h"
#include "decoder.h"
#include "scan.h"
#include "quant.h"
#include "idct.h"
#include "mc.h"
#include <cstddef>

enum mc_template_e {
    mc_templ_field,
    mc_templ_frame
};

template<typename pixel_t, bool alt_scan, bool intra, bool add>
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
    if (add)
        add_inverse_dct(plane, F, stride); 
    else
        inverse_dct(plane, F, stride);
}

template<typename pixel_t, int chroma_format, bool alt_scan, bool intra, bool add>
void decode_transform_template(pixel_t* yuv_planes[3], int stride, int chroma_stride, uint16_t W[4][64], int16_t QFS[12][64], bool pattern_code[12], uint8_t quantizer_scale, uint8_t intra_dc_prec) {
    // Luma
    if (pattern_code[0])
        decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[0], stride, QFS[0], W[0], W[1], quantizer_scale, intra_dc_prec);
    if (pattern_code[1])
        decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[0] + 8, stride, QFS[1], W[0], W[1], quantizer_scale, intra_dc_prec);
    if (pattern_code[2])
        decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[0] + 8 * stride, stride, QFS[2], W[0], W[1], quantizer_scale, intra_dc_prec);
    if (pattern_code[3])
        decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[0] + 8 * (stride + 1), stride, QFS[3], W[0], W[1], quantizer_scale, intra_dc_prec);

    // Chroma format 4:2:0
    if (chroma_format >= 1) {
        if (pattern_code[4])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[1], chroma_stride, QFS[4], W[0], W[1], quantizer_scale, intra_dc_prec);
        if (pattern_code[5])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[2], chroma_stride, QFS[5], W[0], W[1], quantizer_scale, intra_dc_prec);
    }
    // Chroma format 4:2:2
    if (chroma_format >= 2) {
        if (pattern_code[6])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[1] + 8 * chroma_stride, chroma_stride, QFS[6], W[2], W[3], quantizer_scale, intra_dc_prec);
        if (pattern_code[7])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[2] + 8 * chroma_stride, chroma_stride, QFS[7], W[2], W[3], quantizer_scale, intra_dc_prec);
    }
    // Chroma format 4:4:4
    if (chroma_format == 3) {
        if (pattern_code[8])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[1] + 8, stride, QFS[8], W[2], W[3], quantizer_scale, intra_dc_prec);
        if (pattern_code[9])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[2] + 8, stride, QFS[9], W[2], W[3], quantizer_scale, intra_dc_prec);
        if (pattern_code[10])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[1] + 8 * (stride + 1), stride, QFS[10], W[2], W[3], quantizer_scale, intra_dc_prec);
        if (pattern_code[11])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[2] + 8 * (stride + 1), stride, QFS[11], W[2], W[3], quantizer_scale, intra_dc_prec);
    }
}

template<int chroma_format, int plane_idx>
void apply_chroma_scale(int16_t& mvx, int16_t& mvy) {
    if (plane_idx > 0) {
        if (chroma_format < 3)
            mvx >>= 1;
        if (chroma_format < 2)
            mvy >>= 1;
    }
}

int mc_bidir_idx(int16_t mvfx, int16_t mvfy, int16_t mvbx, int16_t mvby) {
    return mvfx & 0x01 + ((mvfy & 0x01) << 1) + ((mvbx & 0x01) << 2) + ((mvby & 0x01) << 3);
}

template<int chroma_format, int plane_idx, int vect_idx, mc_template_e mc_templ>
void mc_bidir_template(uint8_t* dst, uint8_t* ref0, uint8_t* ref1, mb_data_t mb_data, uint32_t stride, uint32_t chroma_stride) {
    auto  _stride = (mc_templ == mc_templ_field) ? stride << 1 : stride;
    auto  _chroma_stride = (mc_templ == mc_templ_field) ? chroma_stride << 1 : chroma_stride;
    uint8_t* fref = ref0;
    uint8_t* bref = ref1;
    auto  mvfx = mb_data.MVs[vect_idx][0][0];
    auto  mvfy = mb_data.MVs[vect_idx][0][1];
    auto  mvbx = mb_data.MVs[vect_idx][1][0];
    auto  mvby = mb_data.MVs[vect_idx][1][1];
    apply_chroma_scale<chroma_format, plane_idx>(mvfx, mvfy);
    apply_chroma_scale<chroma_format, plane_idx>(mvbx, mvby);
    int mvs_ridx = mc_bidir_idx(mvfx, mvfy, mvbx, mvby);
    fref += static_cast<ptrdiff_t>(mvfx >> 1) + static_cast<ptrdiff_t>(mvfy >> 1) * (plane_idx ? _chroma_stride : _stride);
    bref += static_cast<ptrdiff_t>(mvbx >> 1) + static_cast<ptrdiff_t>(mvby >> 1) * (plane_idx ? _chroma_stride : _stride);

    if (plane_idx == 0) {
        if (mb_data.mb.motion_vertical_field_select[vect_idx][0])
            fref += stride;
        if (mb_data.mb.motion_vertical_field_select[vect_idx][1])
            bref += stride;
        if (vect_idx)
            dst += stride;
    }
    else {
        switch (chroma_format) {
        case chroma_format_420:
            if (mb_data.mb.motion_vertical_field_select[vect_idx][0])
                fref += chroma_stride;
            if (mb_data.mb.motion_vertical_field_select[vect_idx][1])
                bref += chroma_stride;
            if (vect_idx)
                dst += chroma_stride;
            break;
        case chroma_format_422:
        case chroma_format_444:
            if (mb_data.mb.motion_vertical_field_select[vect_idx][0])
                fref += chroma_stride;
            if (mb_data.mb.motion_vertical_field_select[vect_idx][1])
                bref += chroma_stride;
            if (vect_idx)
                dst += chroma_stride;
            break;
        }
    }

    if (plane_idx == 0) {
        switch (mc_templ) {
        case mc_templ_field: mc_bidir_16xh[mvs_ridx](dst, bref, fref, _stride,  8); break;
        case mc_templ_frame: mc_bidir_16xh[mvs_ridx](dst, bref, fref, _stride, 16); break;
        }
    }
    else {
        switch (chroma_format) {
        case chroma_format_420: mc_bidir_8xh [mvs_ridx](dst, bref, fref, _chroma_stride, (mc_templ == mc_templ_field) ? 4 :  8); break;
        case chroma_format_422: mc_bidir_8xh [mvs_ridx](dst, bref, fref, _chroma_stride, (mc_templ == mc_templ_field) ? 8 : 16); break;
        case chroma_format_444: mc_bidir_16xh[mvs_ridx](dst, bref, fref, _chroma_stride, (mc_templ == mc_templ_field) ? 8 : 16); break;
        }
    }
}

int mc_unidir_idx(int16_t mvx, int16_t mvy) {
    return (mvx & 0x01) | ((mvy & 0x01) << 1);
}

template<int chroma_format, int plane_idx, int vect_idx, mc_template_e mc_templ, bool forward>
void mc_unidir_template(uint8_t* dst, uint8_t* ref, mb_data_t mb_data, uint32_t stride, uint32_t chroma_stride) {
    auto  _stride = (mc_templ == mc_templ_field) ? stride << 1 : stride;
    auto  _chroma_stride = (mc_templ == mc_templ_field) ? chroma_stride << 1 : chroma_stride;
    auto  mvx = mb_data.MVs[vect_idx][forward ? 0 : 1][0];
    auto  mvy = mb_data.MVs[vect_idx][forward ? 0 : 1][1];
    apply_chroma_scale<chroma_format, plane_idx>(mvx, mvy);
    int mvs_ridx = mc_unidir_idx(mvx, mvy);
    int offset = (mvx >> 1) + (mvy >> 1) * (plane_idx ? _chroma_stride : _stride);
    ref += static_cast<ptrdiff_t>(offset);

    if (mc_templ == mc_templ_field) {
        if (plane_idx == 0) {
            if (mb_data.mb.motion_vertical_field_select[vect_idx][forward ? 0 : 1])
                ref += stride;
            if (vect_idx)
                dst += stride;
        }
        else {
            switch (chroma_format) {
            case chroma_format_420:
                if (mb_data.mb.motion_vertical_field_select[vect_idx][forward ? 0 : 1])
                    ref += chroma_stride;
                if (vect_idx)
                    dst += chroma_stride;
                break;
            case chroma_format_422:
            case chroma_format_444:
                if (mb_data.mb.motion_vertical_field_select[vect_idx][forward ? 0 : 1])
                    ref += chroma_stride;
                if (vect_idx)
                    dst += chroma_stride;
                break;
            }
        }
    }

    if (plane_idx == 0) {
        switch (mc_templ) {
        case mc_templ_field: mc_pred_16xh[mvs_ridx](dst, ref, _stride,  8); break;
        case mc_templ_frame: mc_pred_16xh[mvs_ridx](dst, ref, _stride, 16); break;
        }
    }
    else {
        switch (chroma_format) {
        case chroma_format_420: mc_pred_8xh [mvs_ridx](dst, ref, _chroma_stride, (mc_templ == mc_templ_field) ? 4 :  8); break;
        case chroma_format_422: mc_pred_8xh [mvs_ridx](dst, ref, _chroma_stride, (mc_templ == mc_templ_field) ? 8 : 16); break;
        case chroma_format_444: mc_pred_16xh[mvs_ridx](dst, ref, _chroma_stride, (mc_templ == mc_templ_field) ? 8 : 16); break;
        }
    }
}

template<int chroma_format, mc_template_e mc_templ, bool two_vect>
void base_motion_compensation(uint8_t* dst[3], uint8_t* ref0[3], uint8_t* ref1[3], mb_data_t mb_data, uint32_t stride, uint32_t chroma_stride) {
    auto mb = mb_data.mb;
    if ((mb.macroblock_type & macroblock_motion_forward_bit) && (mb.macroblock_type & macroblock_motion_backward_bit)) {
        mc_bidir_template<chroma_format, 0, 0, mc_templ>(dst[0], ref0[0], ref1[0], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_bidir_template<chroma_format, 0, 1, mc_templ>(dst[0], ref0[0], ref1[0], mb_data, stride, chroma_stride);
        mc_bidir_template<chroma_format, 1, 0, mc_templ>(dst[1], ref0[1], ref1[1], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_bidir_template<chroma_format, 1, 1, mc_templ>(dst[1], ref0[1], ref1[1], mb_data, stride, chroma_stride);
        mc_bidir_template<chroma_format, 2, 0, mc_templ>(dst[2], ref0[2], ref1[2], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_bidir_template<chroma_format, 2, 1, mc_templ>(dst[2], ref0[2], ref1[2], mb_data, stride, chroma_stride);
    } else
    if ((mb.macroblock_type & macroblock_motion_forward_bit) && !(mb.macroblock_type & macroblock_motion_backward_bit)) {
        mc_unidir_template<chroma_format, 0, 0, mc_templ, true>(dst[0], ref0[0], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 0, 1, mc_templ, true>(dst[0], ref0[0], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 1, 0, mc_templ, true>(dst[1], ref0[1], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 1, 1, mc_templ, true>(dst[1], ref0[1], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 2, 0, mc_templ, true>(dst[2], ref0[2], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 2, 1, mc_templ, true>(dst[2], ref0[2], mb_data, stride, chroma_stride);
    } else
    if (!(mb.macroblock_type & macroblock_motion_forward_bit) && (mb.macroblock_type & macroblock_motion_backward_bit)) {
        mc_unidir_template<chroma_format, 0, 0, mc_templ, false>(dst[0], ref1[0], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 0, 1, mc_templ, false>(dst[0], ref1[0], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 1, 0, mc_templ, false>(dst[1], ref1[1], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 1, 1, mc_templ, false>(dst[1], ref1[1], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 2, 0, mc_templ, false>(dst[2], ref1[2], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 2, 1, mc_templ, false>(dst[2], ref1[2], mb_data, stride, chroma_stride);
    } else {
        mc_unidir_template<chroma_format, 0, 0, mc_templ, true>(dst[0], ref0[0], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 0, 1, mc_templ, true>(dst[0], ref0[0], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 1, 0, mc_templ, true>(dst[1], ref0[1], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 1, 1, mc_templ, true>(dst[1], ref0[1], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 2, 0, mc_templ, true>(dst[2], ref0[2], mb_data, stride, chroma_stride);
        if (two_vect)
            mc_unidir_template<chroma_format, 2, 1, mc_templ, true>(dst[2], ref0[2], mb_data, stride, chroma_stride);
    }
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
    int &quant_scale_code,
    uint8_t* ref0[3], uint8_t* ref1[3]) {

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

    if (mb_intra)
        decode_transform_template<uint8_t, chroma_format, alt_scan, true, false>(yuv_planes, stride, chroma_stride, W, mb_data.QFS, mb_data.pattern_code, quantizer_scale, intra_dc_prec);
    else {
        // Motion compensation
        switch (mb_data.mb.prediction_type) {
        case Field_based:
            if (mb_data.mb.motion_vector_count == 2)
                base_motion_compensation<chroma_format, mc_templ_field, true>(yuv_planes, ref0, ref1, mb_data, stride, chroma_stride);
            else
                base_motion_compensation<chroma_format, mc_templ_field, false>(yuv_planes, ref0, ref1, mb_data, stride, chroma_stride);
            break;
        case Frame_based:
            if (mb_data.mb.motion_vector_count == 2)
                base_motion_compensation<chroma_format, mc_templ_frame, true>(yuv_planes, ref0, ref1, mb_data, stride, chroma_stride);
            else
                base_motion_compensation<chroma_format, mc_templ_frame, false>(yuv_planes, ref0, ref1, mb_data, stride, chroma_stride);
            break;
        case Dual_Prime:
        case MC16x8:
            // Not supported
            break;
        }
        decode_transform_template<uint8_t, chroma_format, alt_scan, false, true>(yuv_planes, stride, chroma_stride, W, mb_data.QFS, mb_data.pattern_code, quantizer_scale, intra_dc_prec);
    }
}

decode_macroblock_func_t select_decode_macroblock(int chroma_format, bool q_scale_type, bool alt_scan) {
    switch (chroma_format) {
    case chroma_format_420:
        if (!q_scale_type) {
            if (!alt_scan) return decode_macroblock_template<chroma_format_420, false, false>;
            else           return decode_macroblock_template<chroma_format_420, false, true>;
        }
        else {
            if (!alt_scan) return decode_macroblock_template<chroma_format_420, true, false>;
            else           return decode_macroblock_template<chroma_format_420, true, true>;
        }
    case chroma_format_422:
        if (!q_scale_type) {
            if (!alt_scan) return decode_macroblock_template<chroma_format_422, false, false>;
            else           return decode_macroblock_template<chroma_format_422, false, true>;
        }
        else {
            if (!alt_scan) return decode_macroblock_template<chroma_format_422, true, false>;
            else           return decode_macroblock_template<chroma_format_422, true, true>;
        }
    case chroma_format_444:
        if (!q_scale_type) {
            if (!alt_scan) return decode_macroblock_template<chroma_format_444, false, false>;
            else           return decode_macroblock_template<chroma_format_444, false, true>;
        }
        else {
            if (!alt_scan) return decode_macroblock_template<chroma_format_444, true, false>;
            else           return decode_macroblock_template<chroma_format_444, true, true>;
        }
    }
}