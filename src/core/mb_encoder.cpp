#include "common/cpu.hpp"
#include "mb_encoder.h"
#include "mp2v_luts.hpp"

#if defined(CPU_PLATFORM_AARCH64)
#include "fdct_quant_scan_aarch64.hpp"
#elif defined(CPU_PLATFORM_X64)
#include "fdct_quant_scan_sse2.hpp"
#else
#include "fdct_quant_scan_c.hpp"
#endif

MP2V_INLINE void write_vlc(bitstream_writer_c& bs, vlc_t vlc) { bs.write_bits(vlc.value, vlc.len); }

MP2V_INLINE bool encode_coeff(bitstream_writer_c &bs, int16_t level, int run) {
    if (level < 2)       { if (run < 32) { write_vlc(bs, coeff_one_tab1[run]); return true; } } 
    else if (level < 3)  { if (run < 17) { write_vlc(bs, coeff_one_tab2[run]); return true; } }
    else if (level < 4)  { if (run < 7)  { write_vlc(bs, coeff_one_tab3[run]); return true; } }
    else if (level < 5)  { if (run < 4)  { write_vlc(bs, coeff_one_tab4[run]); return true; } }
    else if (level < 6)  { if (run < 3)  { write_vlc(bs, coeff_one_tab5[run]); return true; } }
    else if (level < 19) { if (run < 2)  { write_vlc(bs, coeff_one_tab6[run + (level - 6) * 2]); return true; } }
    else if (level < 41) { if (run < 1)  { write_vlc(bs, coeff_one_tab7[level - 19]); return true; } }
    return false;
}

MP2V_INLINE int16_t next_coeff(uint64_t& nnz, int16_t*& QF, int& run)
{
    nnz >>= 1;
    QF++;
    run = static_cast<uint32_t>(bit_scan_forward64(~nnz));
    nnz >>= run;
    QF += run;
    return QF[0];
}

template<bool luma>
MP2V_INLINE void encode_intra_block(bitstream_writer_c &bs, int16_t &dc_pred, uint8_t* plane, int stride, int qp, int dc_prec) {
    int16_t QF[65];
    int16_t* qf = QF;

    uint64_t nnz = forward_dct_scan_quant_template<true, true>(QF, plane, stride, qp, dc_prec);

    int16_t dc_diff = QF[0] - dc_pred;
    dc_pred = QF[0];
    int dc_size = 0;
    if (dc_diff != 0)
        dc_size = 32 - bit_scan_reverse(std::abs(dc_diff));

    auto dct_size_vlc_tbl = luma ? dct_size_luminance_to_vlc : dct_size_chrominance_to_vlc;
    write_vlc(bs, dct_size_vlc_tbl[dc_size]);
    if (dc_size != 0) {
        if (dc_diff < 0) dc_diff += (1 << dc_size) - 1;
        bs.write_bits(dc_diff, dc_size);
    }

    int run = 0;
    int16_t level = next_coeff(nnz, qf, run);
    while (level) {
        if (encode_coeff(bs, std::abs(level), run)) {
            if (level < 0) bs.one_bit();
            else bs.zero_bit();
        }
        else
            bs.write_bits(((run << 12) | (1L << (6 + 12)) | (level & 0x0FFF)), (6 + 6 + 12));

        /*if (level >= -40 && level < 41 && run < 32)
            write_vlc(bs, coeff_big_one_tab[run][level + 40]);
        else
            bs.write_bits(((run << 12) | (1L << (6 + 12)) | (level & 0x0FFF)), (6 + 6 + 12));*/

        level = next_coeff(nnz, qf, run);
    }

    bs.write_bits(0b0110, 4); // end of block
}

void encode_mb(bitstream_writer_c &bs, uint8_t* yuv_ptrs[3], int16_t (&dc_pred)[3], int strides[3], int qp, int dc_prec) {
    //macroblock address increment
    bs.one_bit(); // macroblock address increment = 1
    //modes
    bs.one_bit(); // macroblock type = 0b000010 (just intra bit)
    //luma
    encode_intra_block<true>(bs, dc_pred[0], yuv_ptrs[0], strides[0], qp, dc_prec);
    encode_intra_block<true>(bs, dc_pred[0], yuv_ptrs[0] + 8, strides[0], qp, dc_prec);
    encode_intra_block<true>(bs, dc_pred[0], yuv_ptrs[0] + 8 * strides[0], strides[0], qp, dc_prec);
    encode_intra_block<true>(bs, dc_pred[0], yuv_ptrs[0] + 8 * strides[0] + 8, strides[0], qp, dc_prec);
    //chroma
    encode_intra_block<false>(bs, dc_pred[1], yuv_ptrs[1], strides[1], qp, dc_prec);
    encode_intra_block<false>(bs, dc_pred[2], yuv_ptrs[2], strides[2], qp, dc_prec);
    encode_intra_block<false>(bs, dc_pred[1], yuv_ptrs[1] + 8 * strides[1], strides[1], qp, dc_prec);
    encode_intra_block<false>(bs, dc_pred[2], yuv_ptrs[2] + 8 * strides[2], strides[2], qp, dc_prec);
}
