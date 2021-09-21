#include "mc.h"

#if defined(__aarch64__) || defined(__arm__)
#include "mc_c.hpp"

mc_pred_func_t mc_pred_16xh[4] = {
    mc_pred00_16xh_c,
    mc_pred01_16xh_c,
    mc_pred10_16xh_c,
    mc_pred11_16xh_c
};

mc_pred_func_t mc_pred_8xh[4] = {
    mc_pred00_8xh_c,
    mc_pred01_8xh_c,
    mc_pred10_8xh_c,
    mc_pred11_8xh_c
};

mc_bidir_func_t mc_bidir_16xh[16] = {
    mc_bidir0000_16xh_c,
    mc_bidir0001_16xh_c,
    mc_bidir0010_16xh_c,
    mc_bidir0011_16xh_c,
    mc_bidir0100_16xh_c,
    mc_bidir0101_16xh_c,
    mc_bidir0110_16xh_c,
    mc_bidir0111_16xh_c,
    mc_bidir1000_16xh_c,
    mc_bidir1001_16xh_c,
    mc_bidir1010_16xh_c,
    mc_bidir1011_16xh_c,
    mc_bidir1100_16xh_c,
    mc_bidir1101_16xh_c,
    mc_bidir1110_16xh_c,
    mc_bidir1111_16xh_c
};

mc_bidir_func_t mc_bidir_8xh[16] = {
    mc_bidir0000_8xh_c,
    mc_bidir0001_8xh_c,
    mc_bidir0010_8xh_c,
    mc_bidir0011_8xh_c,
    mc_bidir0100_8xh_c,
    mc_bidir0101_8xh_c,
    mc_bidir0110_8xh_c,
    mc_bidir0111_8xh_c,
    mc_bidir1000_8xh_c,
    mc_bidir1001_8xh_c,
    mc_bidir1010_8xh_c,
    mc_bidir1011_8xh_c,
    mc_bidir1100_8xh_c,
    mc_bidir1101_8xh_c,
    mc_bidir1110_8xh_c,
    mc_bidir1111_8xh_c
};

#else
#include "mc_naive_sse2.hpp"
//#include "mc_sse2.hpp"

mc_pred_func_t mc_pred_16xh[4] = {
    mc_pred00_16xh_nsse2,
    mc_pred01_16xh_nsse2,
    mc_pred10_16xh_nsse2,
    mc_pred11_16xh_nsse2
};

mc_pred_func_t mc_pred_8xh[4] = {
    mc_pred00_8xh_nsse2,
    mc_pred01_8xh_nsse2,
    mc_pred10_8xh_nsse2,
    mc_pred11_8xh_nsse2
};

mc_bidir_func_t mc_bidir_16xh[16] = {
    mc_bidir0000_16xh_nsse2,
    mc_bidir0001_16xh_nsse2,
    mc_bidir0010_16xh_nsse2,
    mc_bidir0011_16xh_nsse2,
    mc_bidir0100_16xh_nsse2,
    mc_bidir0101_16xh_nsse2,
    mc_bidir0110_16xh_nsse2,
    mc_bidir0111_16xh_nsse2,
    mc_bidir1000_16xh_nsse2,
    mc_bidir1001_16xh_nsse2,
    mc_bidir1010_16xh_nsse2,
    mc_bidir1011_16xh_nsse2,
    mc_bidir1100_16xh_nsse2,
    mc_bidir1101_16xh_nsse2,
    mc_bidir1110_16xh_nsse2,
    mc_bidir1111_16xh_nsse2
};

mc_bidir_func_t mc_bidir_8xh[16] = {
    mc_bidir0000_8xh_nsse2,
    mc_bidir0001_8xh_nsse2,
    mc_bidir0010_8xh_nsse2,
    mc_bidir0011_8xh_nsse2,
    mc_bidir0100_8xh_nsse2,
    mc_bidir0101_8xh_nsse2,
    mc_bidir0110_8xh_nsse2,
    mc_bidir0111_8xh_nsse2,
    mc_bidir1000_8xh_nsse2,
    mc_bidir1001_8xh_nsse2,
    mc_bidir1010_8xh_nsse2,
    mc_bidir1011_8xh_nsse2,
    mc_bidir1100_8xh_nsse2,
    mc_bidir1101_8xh_nsse2,
    mc_bidir1110_8xh_nsse2,
    mc_bidir1111_8xh_nsse2
};

#endif