// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#include "mb_decoder.h"
#include "mp2v_vlc.h"
#include "mc.h"

enum mc_template_e {
    mc_templ_field,
    mc_templ_frame
};

#if defined(__aarch64__) || defined(__arm__)
#include "scan.h"
#include "idct.h"
#include "quant_c.hpp"

template<typename pixel_t, bool alt_scan, bool intra, bool add>
MP2V_INLINE void decode_block_template(pixel_t* plane, uint32_t stride, int16_t QFS[64], uint16_t W_i[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_prec) {
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
        add_inverse_dct_c(plane, F, stride); 
    else
        inverse_dct_c(plane, F, stride);
}
#else
#include "scan_dequant_idct_sse2.hpp"
template<typename pixel_t, bool alt_scan, bool intra, bool add>
MP2V_INLINE void decode_block_template(pixel_t* plane, uint32_t stride, int16_t QFS[64], uint16_t W_i[64], uint16_t W[64], uint8_t quantizer_scale, int intra_dc_prec) {
    scan_dequant_idct_template_sse2<pixel_t, intra, add>(plane, stride, QFS, intra ? W_i : W, quantizer_scale, intra_dc_prec);
}
#endif

template<typename pixel_t, int chroma_format, bool alt_scan, bool intra, bool add>
MP2V_INLINE void decode_transform_template(pixel_t* yuv_planes[3], int stride, int chroma_stride, uint16_t W[4][64], int16_t QFS[12][64], bool pattern_code[12], uint8_t quantizer_scale, uint8_t intra_dc_prec) {
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
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[1] + 8, chroma_stride, QFS[8], W[2], W[3], quantizer_scale, intra_dc_prec);
        if (pattern_code[9])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[2] + 8, chroma_stride, QFS[9], W[2], W[3], quantizer_scale, intra_dc_prec);
        if (pattern_code[10])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[1] + 8 * (stride + 1), chroma_stride, QFS[10], W[2], W[3], quantizer_scale, intra_dc_prec);
        if (pattern_code[11])
            decode_block_template<pixel_t, alt_scan, intra, add>(yuv_planes[2] + 8 * (stride + 1), chroma_stride, QFS[11], W[2], W[3], quantizer_scale, intra_dc_prec);
    }
}

template<int chroma_format, int plane_idx>
MP2V_INLINE void apply_chroma_scale(int16_t& mvx, int16_t& mvy) {
    if (plane_idx > 0) {
        if (chroma_format < 3)
            mvx >>= 1;
        if (chroma_format < 2)
            mvy >>= 1;
    }
}

MP2V_INLINE int mc_bidir_idx(int16_t mvfx, int16_t mvfy, int16_t mvbx, int16_t mvby) {
    return mvfx & 0x01 + ((mvfy & 0x01) << 1) + ((mvbx & 0x01) << 2) + ((mvby & 0x01) << 3);
}

template<int chroma_format, int plane_idx, int vect_idx, mc_template_e mc_templ>
MP2V_INLINE void mc_bidir_template(uint8_t* dst, uint8_t* ref0, uint8_t* ref1, mb_data_t& mb_data, uint32_t stride, uint32_t chroma_stride) {
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

    auto plane_stride = (plane_idx == 0) ? stride : chroma_stride;
    if (mc_templ == mc_templ_field) {
        if (mb_data.mb.motion_vertical_field_select[vect_idx][0])
            fref += plane_stride;
        if (mb_data.mb.motion_vertical_field_select[vect_idx][1])
            bref += plane_stride;
        if (vect_idx)
            dst += plane_stride;
    }

    if (plane_idx == 0) {
        switch (mc_templ) {
        case mc_templ_field: mc_bidir_16xh[mvs_ridx](dst, bref, fref, _stride, 8); break;
        case mc_templ_frame: mc_bidir_16xh[mvs_ridx](dst, bref, fref, _stride, 16); break;
        }
    }
    else {
        switch (chroma_format) {
        case chroma_format_420: mc_bidir_8xh[mvs_ridx](dst, bref, fref, _chroma_stride, (mc_templ == mc_templ_field) ? 4 : 8); break;
        case chroma_format_422: mc_bidir_8xh[mvs_ridx](dst, bref, fref, _chroma_stride, (mc_templ == mc_templ_field) ? 8 : 16); break;
        case chroma_format_444: mc_bidir_16xh[mvs_ridx](dst, bref, fref, _chroma_stride, (mc_templ == mc_templ_field) ? 8 : 16); break;
        }
    }
}

MP2V_INLINE int mc_unidir_idx(int16_t mvx, int16_t mvy) {
    return (mvx & 0x01) | ((mvy & 0x01) << 1);
}

template<int chroma_format, int plane_idx, int vect_idx, mc_template_e mc_templ, bool forward>
MP2V_INLINE void mc_unidir_template(uint8_t* dst, uint8_t* ref, mb_data_t &mb_data, uint32_t stride, uint32_t chroma_stride) {
    auto  _stride = (mc_templ == mc_templ_field) ? stride << 1 : stride;
    auto  _chroma_stride = (mc_templ == mc_templ_field) ? chroma_stride << 1 : chroma_stride;
    auto  mvx = mb_data.MVs[vect_idx][forward ? 0 : 1][0];
    auto  mvy = mb_data.MVs[vect_idx][forward ? 0 : 1][1];
    apply_chroma_scale<chroma_format, plane_idx>(mvx, mvy);
    int mvs_ridx = mc_unidir_idx(mvx, mvy);
    int offset = (mvx >> 1) + (mvy >> 1) * (plane_idx ? _chroma_stride : _stride);
    ref += static_cast<ptrdiff_t>(offset);

    auto plane_stride = (plane_idx == 0) ? stride : chroma_stride;
    if (mc_templ == mc_templ_field) {
        if (mb_data.mb.motion_vertical_field_select[vect_idx][forward ? 0 : 1])
            ref += plane_stride;
        if (vect_idx)
            dst += plane_stride;
    }

    if (plane_idx == 0) {
        switch (mc_templ) {
        case mc_templ_field: mc_pred_16xh[mvs_ridx](dst, ref, _stride,  8); break;
        case mc_templ_frame: mc_pred_16xh[mvs_ridx](dst, ref, _stride, 16); break;
        }
    }
    else {
        switch (chroma_format) {
        case chroma_format_420: mc_pred_8xh[mvs_ridx](dst, ref, _chroma_stride, (mc_templ == mc_templ_field) ? 4 :  8); break;
        case chroma_format_422: mc_pred_8xh[mvs_ridx](dst, ref, _chroma_stride, (mc_templ == mc_templ_field) ? 8 : 16); break;
        case chroma_format_444: mc_pred_16xh[mvs_ridx](dst, ref, _chroma_stride, (mc_templ == mc_templ_field) ? 8 : 16); break;
        }
    }
}

template<int chroma_format, mc_template_e mc_templ, bool two_vect>
MP2V_INLINE void base_motion_compensation(uint8_t* dst[3], uint8_t* ref0[3], uint8_t* ref1[3], mb_data_t &mb_data, uint32_t stride, uint32_t chroma_stride) {
    auto mb = mb_data.mb;
    if ((mb.macroblock_type & macroblock_motion_forward_bit) && (mb.macroblock_type & macroblock_motion_backward_bit)) {
        mc_bidir_template<chroma_format, 0, 0, mc_templ>(dst[0], ref0[0], ref1[0], mb_data, stride, chroma_stride);
        mc_bidir_template<chroma_format, 1, 0, mc_templ>(dst[1], ref0[1], ref1[1], mb_data, stride, chroma_stride);
        mc_bidir_template<chroma_format, 2, 0, mc_templ>(dst[2], ref0[2], ref1[2], mb_data, stride, chroma_stride);
        if (two_vect) {
            mc_bidir_template<chroma_format, 0, 1, mc_templ>(dst[0], ref0[0], ref1[0], mb_data, stride, chroma_stride);
            mc_bidir_template<chroma_format, 1, 1, mc_templ>(dst[1], ref0[1], ref1[1], mb_data, stride, chroma_stride);
            mc_bidir_template<chroma_format, 2, 1, mc_templ>(dst[2], ref0[2], ref1[2], mb_data, stride, chroma_stride);
        }
    } else
    if ((mb.macroblock_type & macroblock_motion_forward_bit) && !(mb.macroblock_type & macroblock_motion_backward_bit)) {
        mc_unidir_template<chroma_format, 0, 0, mc_templ, true>(dst[0], ref0[0], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 1, 0, mc_templ, true>(dst[1], ref0[1], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 2, 0, mc_templ, true>(dst[2], ref0[2], mb_data, stride, chroma_stride);
        if (two_vect) {
            mc_unidir_template<chroma_format, 0, 1, mc_templ, true>(dst[0], ref0[0], mb_data, stride, chroma_stride);
            mc_unidir_template<chroma_format, 1, 1, mc_templ, true>(dst[1], ref0[1], mb_data, stride, chroma_stride);
            mc_unidir_template<chroma_format, 2, 1, mc_templ, true>(dst[2], ref0[2], mb_data, stride, chroma_stride);
        }
    } else
    if (!(mb.macroblock_type & macroblock_motion_forward_bit) && (mb.macroblock_type & macroblock_motion_backward_bit)) {
        mc_unidir_template<chroma_format, 0, 0, mc_templ, false>(dst[0], ref1[0], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 1, 0, mc_templ, false>(dst[1], ref1[1], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 2, 0, mc_templ, false>(dst[2], ref1[2], mb_data, stride, chroma_stride);
        if (two_vect) {
            mc_unidir_template<chroma_format, 0, 1, mc_templ, false>(dst[0], ref1[0], mb_data, stride, chroma_stride);
            mc_unidir_template<chroma_format, 1, 1, mc_templ, false>(dst[1], ref1[1], mb_data, stride, chroma_stride);
            mc_unidir_template<chroma_format, 2, 1, mc_templ, false>(dst[2], ref1[2], mb_data, stride, chroma_stride);
        }
    } else {
        mc_unidir_template<chroma_format, 0, 0, mc_templ, true>(dst[0], ref0[0], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 1, 0, mc_templ, true>(dst[1], ref0[1], mb_data, stride, chroma_stride);
        mc_unidir_template<chroma_format, 2, 0, mc_templ, true>(dst[2], ref0[2], mb_data, stride, chroma_stride);
        if (two_vect) {
            mc_unidir_template<chroma_format, 0, 1, mc_templ, true>(dst[0], ref0[0], mb_data, stride, chroma_stride);
            mc_unidir_template<chroma_format, 1, 1, mc_templ, true>(dst[1], ref0[1], mb_data, stride, chroma_stride);
            mc_unidir_template<chroma_format, 2, 1, mc_templ, true>(dst[2], ref0[2], mb_data, stride, chroma_stride);
        }
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
    mb_data_t &mb_data, 
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

void decode_macroblock_func_nope(uint8_t* yuv_planes[3], int stride, int chroma_stride, mb_data_t &mb_data, uint16_t W[4][64], uint8_t intra_dc_prec, int &quant_scale_code, uint8_t* ref0[3], uint8_t* ref1[3]) {};

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
    return decode_macroblock_func_nope;
}

template<int picture_coding_type, int picture_structure, int frame_pred_frame_dct>
static bool parse_modes(bitstream_reader_c* m_bs, macroblock_t& mb, int spatial_temporal_weight_code_table_index, mv_format_e& mv_format) {
    mb.macroblock_type = get_macroblock_type(m_bs, picture_coding_type);
    if ((mb.macroblock_type & spatial_temporal_weight_code_flag_bit) && (spatial_temporal_weight_code_table_index != 0)) {
        /*uint32_t spatial_temporal_weight_code = */m_bs->read_next_bits(2);
    }
    if ((mb.macroblock_type & macroblock_motion_forward_bit) || (mb.macroblock_type & macroblock_motion_backward_bit)) {
        if (picture_structure == picture_structure_framepic) {
            mb.frame_motion_type = 2; // Frame-based
            if (frame_pred_frame_dct == 0)
                mb.frame_motion_type = m_bs->read_next_bits(2);
        }
        else {
            mb.field_motion_type = m_bs->read_next_bits(2);
        }
    }
    if ((picture_structure == picture_structure_framepic) && (frame_pred_frame_dct == 0) &&
        ((mb.macroblock_type & macroblock_intra_bit) || (mb.macroblock_type & macroblock_pattern_bit))) {
        mb.dct_type = m_bs->read_next_bits(1);
    }

    // decode modes
    if (mb.macroblock_type & macroblock_intra_bit)
    {
        mb.motion_vector_count = 0;
        mb.dmv = 0;
        if (picture_structure == picture_structure_framepic) {
            mv_format = Frame;
            mb.prediction_type = Frame_based;
        }
        else {
            mv_format = Field;
            mb.prediction_type = Field_based;
        }
    }
    else
    {
        mb.motion_vector_count = 1;
        mb.dmv = 0;
        if (picture_structure == picture_structure_framepic) {
            switch (mb.frame_motion_type) {
            case 1:
                mv_format = Field;
                mb.motion_vector_count = 2;
                mb.prediction_type = Field_based;
                break;
            case 2:
                mv_format = Frame;
                mb.prediction_type = Frame_based;
                break;
            case 3:
                mv_format = Field;
                mb.prediction_type = Dual_Prime;
                mb.dmv = 1;
                break;
            }
        }
        else {
            switch (mb.field_motion_type) {
            case 1:
                mv_format = Field;
                mb.prediction_type = Field_based;
                break;
            case 2:
                mv_format = Field;
                mb.motion_vector_count = 2;
                mb.prediction_type = MC16x8;
                break;
            case 3:
                mv_format = Field;
                mb.prediction_type = Dual_Prime;
                mb.dmv = 1;
                break;
            }
        }
    }
    return true;
}

template<int chroma_format>
bool parse_coded_block_pattern(bitstream_reader_c* m_bs, macroblock_t& mb) {
    mb.coded_block_pattern_420 = get_coded_block_pattern(m_bs);
    if (chroma_format == chroma_format_422)
        mb.coded_block_pattern_1 = m_bs->read_next_bits(2);
    if (chroma_format == chroma_format_444)
        mb.coded_block_pattern_2 = m_bs->read_next_bits(6);
    return false;
}

template<uint8_t picture_structure, int t, bool residual>
MP2V_INLINE void update_motion_predictor(uint32_t f_code, int32_t motion_code, uint32_t motion_residual, int16_t& PMV, int16_t& MVs, mv_format_e mv_format) {
    int r_size = f_code - 1;
    int f = 1 << r_size;
    int high = (16 * f) - 1;
    int low = ((-16) * f);
    int range = (32 * f);

    int delta;
    if (residual) {
        delta = ((labs(motion_code) - 1) * f) + motion_residual + 1;
        if (motion_code < 0)
            delta = -delta;
    }
    else
        delta = motion_code;

    int prediction = PMV;
    if ((mv_format == Field) && (t == 1) && (picture_structure == picture_structure_framepic))
        prediction = PMV >> 1;

    MVs = prediction + delta;

    if (MVs < low)  MVs += range;
    if (MVs > high) MVs -= range;

    if ((mv_format == Field) && (t == 1) && (picture_structure == picture_structure_framepic))
        PMV = MVs * 2;
    else
        PMV = MVs;
}

template<uint8_t picture_structure>
MP2V_INLINE bool parse_motion_vector(bitstream_reader_c* m_bs, int dmv, uint32_t f_code[2], int16_t PMV[2], int16_t MVs[2], mv_format_e mv_format) {
    int32_t motion_code = get_motion_code(m_bs);
    if ((f_code[0] != 1) && (motion_code != 0)) {
        uint32_t motion_residual = m_bs->read_next_bits(f_code[0] - 1);
        update_motion_predictor<picture_structure, 0, true>(f_code[0], motion_code, motion_residual, PMV[0], MVs[0], mv_format);
    }
    else
        update_motion_predictor<picture_structure, 0, false>(f_code[0], motion_code, 0, PMV[0], MVs[0], mv_format);

    if (dmv == 1)
        /*mb.dmvector[0] = */get_dmvector(m_bs);

    motion_code = get_motion_code(m_bs);
    if ((f_code[1] != 1) && (motion_code != 0)) {
        uint32_t motion_residual = m_bs->read_next_bits(f_code[1] - 1);
        update_motion_predictor<picture_structure, 1, true>(f_code[1], motion_code, motion_residual, PMV[1], MVs[1], mv_format);
    }
    else
        update_motion_predictor<picture_structure, 1, false>(f_code[1], motion_code, 0, PMV[1], MVs[1], mv_format);

    if (dmv == 1)
        /*mb.dmvector[1] = */get_dmvector(m_bs);
    return true;
}

template <uint8_t picture_structure, int s>
MP2V_INLINE bool parse_motion_vectors(bitstream_reader_c* m_bs, macroblock_t& mb, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2], mv_format_e mv_format) {
    if (mb.motion_vector_count == 1) {
        if ((mv_format == Field) && (mb.dmv != 1))
            mb.motion_vertical_field_select[0][s] = m_bs->read_next_bits(1);
        parse_motion_vector<picture_structure>(m_bs, mb.dmv, f_code[s], PMV[0][s], MVs[0][s], mv_format);
    }
    else {
        mb.motion_vertical_field_select[0][s] = m_bs->read_next_bits(1);
        parse_motion_vector<picture_structure>(m_bs, mb.dmv, f_code[s], PMV[0][s], MVs[0][s], mv_format);
        mb.motion_vertical_field_select[1][s] = m_bs->read_next_bits(1);
        parse_motion_vector<picture_structure>(m_bs, mb.dmv, f_code[s], PMV[1][s], MVs[1][s], mv_format);
    }
    return true;
}

template<uint8_t picture_coding_type,        //3 bit (I, P, B)
         uint8_t picture_structure,          //2 bit (top|bottom field, frame)
         uint8_t frame_pred_frame_dct,       //1 bit // only with picture_structure == frame
         uint8_t concealment_motion_vectors, //1 bit // only with picture_coding_type == I
         uint8_t chroma_format,              //2 bit (420, 422, 444)
         bool q_scale_type, bool alt_scan>
    bool parse_macroblock_template(bitstream_reader_c* m_bs, macroblock_t& mb, int spatial_temporal_weight_code_table_index, uint32_t f_code[2][2], int16_t PMV[2][2][2], int16_t MVs[2][2][2]) {
    mv_format_e mv_format;
    mb.macroblock_address_increment = 0;
    while (m_bs->get_next_bits(vlc_macroblock_escape_code.len) == vlc_macroblock_escape_code.value) {
        m_bs->skip_bits(vlc_macroblock_escape_code.len);
        mb.macroblock_address_increment += 33;
    }
    mb.macroblock_address_increment += get_macroblock_address_increment(m_bs);

    parse_modes<picture_coding_type, picture_structure, frame_pred_frame_dct>(m_bs, mb, spatial_temporal_weight_code_table_index, mv_format);
    if (mb.macroblock_type & macroblock_quant_bit)
        mb.quantiser_scale_code = m_bs->read_next_bits(5);

    if ((mb.macroblock_type & macroblock_motion_forward_bit) || ((mb.macroblock_type & macroblock_intra_bit) && concealment_motion_vectors))
        parse_motion_vectors<picture_structure, 0>(m_bs, mb, f_code, PMV, MVs, mv_format);

    if ((mb.macroblock_type & macroblock_motion_backward_bit) != 0)
        parse_motion_vectors<picture_structure, 1>(m_bs, mb, f_code, PMV, MVs, mv_format);

    if (((mb.macroblock_type & macroblock_intra_bit) != 0) && concealment_motion_vectors)
        m_bs->skip_bits(1);

    if (mb.macroblock_type & macroblock_pattern_bit)
        parse_coded_block_pattern<chroma_format>(m_bs, mb);
    return true;
}

#define SEL_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(pct, ps, fpfdct, cmv) { \
    switch (chroma_format) { \
    case chroma_format_420:  \
        if (!q_scale_type) { \
               if (!alt_scan) return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_420, false, false>;    \
               else           return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_420, false, true>; }   \
        else { if (!alt_scan) return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_420, true, false>;     \
               else           return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_420, true, true>; }    \
    case chroma_format_422:                                                                                               \
        if (!q_scale_type) {                                                                                              \
               if (!alt_scan) return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_422, false, false>;    \
               else           return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_422, false, true>; }   \
        else { if (!alt_scan) return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_422, true, false>;     \
               else           return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_422, true, true>; }    \
    case chroma_format_444:                                                                                               \
        if (!q_scale_type) {                                                                                              \
               if (!alt_scan) return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_444, false, false>;    \
               else           return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_444, false, true>; }   \
        else { if (!alt_scan) return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_444, true, false>;     \
               else           return parse_macroblock_template<pct, ps, fpfdct, cmv, chroma_format_444, true, true>; } } }

#define SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(pct, cmv) \
    if (picture_structure == picture_structure_framepic) { \
        if (frame_pred_frame_dct) \
            SEL_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(pct, picture_structure_framepic, 1, cmv) \
        else \
            SEL_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(pct, picture_structure_framepic, 0, cmv) \
    } else \
        SEL_CHROMA_FROMATS_PARSE_MACROBLOCKS_ROUTINES(pct, picture_structure_topfield, 0, cmv)

parse_macroblock_func_t select_parse_macroblock_func(uint8_t picture_coding_type, uint8_t picture_structure, uint8_t frame_pred_frame_dct, uint8_t concealment_motion_vectors, uint8_t chroma_format, bool q_scale_type, bool alt_scan)
{
    switch (picture_coding_type) {
    case picture_coding_type_intra:
        if (concealment_motion_vectors) {
            SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(picture_coding_type_intra, 1)
        }
        else {
            SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(picture_coding_type_intra, 0)
        }
    case picture_coding_type_pred:
        SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(picture_coding_type_pred, 0)
    case picture_coding_type_bidir:
        SEL_FRAME_FIELD_PARSE_MACROBLOCKS_ROUTINES(picture_coding_type_bidir, 0)
    default:
        return 0;
    };
};
