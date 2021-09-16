#include "common/cpu.hpp"

template<mc_type_e mc_type>
MP2V_INLINE uint8_t mc_func_template(uint8_t* src, uint32_t i, uint32_t stride) {
    switch (mc_type)
    {
    case MC_00:
        return src[i];
    case MC_01:
        return (src[i] + src[i + 1] + 1) >> 1;
    case MC_10:
        return (src[i] + src[i + stride] + 1) >> 1;
    case MC_11:
        return (((src[i] + src[i + 1] + 1) >> 1) + ((src[i + stride] + src[i + stride + 1] + 1) >> 1) + 1) >> 1;
    }
}

template<mc_type_e mc_type, int width>
MP2V_INLINE void pred_mc_template(uint8_t* dst, uint8_t* src, uint32_t stride, int height)
{
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            dst[i] = mc_func_template<mc_type>(src, i, stride);
        }
        src += stride;
        dst += stride;
    }
}

void mc_pred00_16xh_c  (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { pred_mc_template<MC_00, 16>(dst, src, stride, height); }
void mc_pred01_16xh_c  (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { pred_mc_template<MC_01, 16>(dst, src, stride, height); }
void mc_pred10_16xh_c  (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { pred_mc_template<MC_10, 16>(dst, src, stride, height); }
void mc_pred11_16xh_c  (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { pred_mc_template<MC_11, 16>(dst, src, stride, height); }
void mc_pred00_8xh_c   (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { pred_mc_template<MC_00, 8 >(dst, src, stride, height); }
void mc_pred01_8xh_c   (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { pred_mc_template<MC_01, 8 >(dst, src, stride, height); }
void mc_pred10_8xh_c   (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { pred_mc_template<MC_10, 8 >(dst, src, stride, height); }
void mc_pred11_8xh_c   (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height) { pred_mc_template<MC_11, 8 >(dst, src, stride, height); }

template<mc_type_e mc_type_src0, mc_type_e mc_type_src1, int width>
MP2V_INLINE void bidir_mc_template(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, int height)
{
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            uint8_t tmp0 = mc_func_template<mc_type_src0>(src0, i, stride);
            uint8_t tmp1 = mc_func_template<mc_type_src1>(src1, i, stride);
            uint8_t res  = (tmp0 + tmp1 + 1) >> 1;
            dst[i] = res;
        }
        src0 += stride;
        src1 += stride;
        dst += stride;
    }
}

void mc_bidir0000_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_00, MC_00, 16>(dst, src0, src1, stride, height); }
void mc_bidir0001_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_00, MC_01, 16>(dst, src0, src1, stride, height); }
void mc_bidir0010_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_00, MC_10, 16>(dst, src0, src1, stride, height); }
void mc_bidir0011_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_00, MC_11, 16>(dst, src0, src1, stride, height); }
void mc_bidir0100_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_01, MC_00, 16>(dst, src0, src1, stride, height); }
void mc_bidir0101_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_01, MC_01, 16>(dst, src0, src1, stride, height); }
void mc_bidir0110_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_01, MC_10, 16>(dst, src0, src1, stride, height); }
void mc_bidir0111_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_01, MC_11, 16>(dst, src0, src1, stride, height); }
void mc_bidir1000_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_10, MC_00, 16>(dst, src0, src1, stride, height); }
void mc_bidir1001_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_10, MC_01, 16>(dst, src0, src1, stride, height); }
void mc_bidir1010_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_10, MC_10, 16>(dst, src0, src1, stride, height); }
void mc_bidir1011_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_10, MC_11, 16>(dst, src0, src1, stride, height); }
void mc_bidir1100_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_11, MC_00, 16>(dst, src0, src1, stride, height); }
void mc_bidir1101_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_11, MC_01, 16>(dst, src0, src1, stride, height); }
void mc_bidir1110_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_11, MC_10, 16>(dst, src0, src1, stride, height); }
void mc_bidir1111_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_11, MC_11, 16>(dst, src0, src1, stride, height); }
void mc_bidir0000_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_00, MC_00, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0001_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_00, MC_01, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0010_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_00, MC_10, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0011_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_00, MC_11, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0100_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_01, MC_00, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0101_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_01, MC_01, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0110_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_01, MC_10, 8 >(dst, src0, src1, stride, height); }
void mc_bidir0111_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_01, MC_11, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1000_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_10, MC_00, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1001_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_10, MC_01, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1010_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_10, MC_10, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1011_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_10, MC_11, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1100_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_11, MC_00, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1101_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_11, MC_01, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1110_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_11, MC_10, 8 >(dst, src0, src1, stride, height); }
void mc_bidir1111_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height) { bidir_mc_template<MC_11, MC_11, 8 >(dst, src0, src1, stride, height); }
