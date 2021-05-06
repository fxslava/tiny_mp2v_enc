#pragma once
#include <stdint.h>

enum mc_type_e { MC_00, MC_10, MC_01, MC_11 };

void mc_pred00_16xh_c(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_pred01_16xh_c(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_pred10_16xh_c(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_pred11_16xh_c(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_pred00_8xh_c (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_pred01_8xh_c (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_pred10_8xh_c (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_pred11_8xh_c (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);

void mc_cache_pred00_16xh_c(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_cache_pred01_16xh_c(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_cache_pred10_16xh_c(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_cache_pred11_16xh_c(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_cache_pred00_8xh_c (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_cache_pred01_8xh_c (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_cache_pred10_8xh_c (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
void mc_cache_pred11_8xh_c (uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);

void mc_bidir0000_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0001_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0010_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0011_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0100_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0101_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0110_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0111_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1000_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1001_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1010_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1011_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1100_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1101_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1110_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1111_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0000_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0001_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0010_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0011_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0100_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0101_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0110_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir0111_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1000_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1001_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1010_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1011_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1100_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1101_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1110_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_bidir1111_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);

void mc_cache_bidir0000_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0001_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0010_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0011_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0100_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0101_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0110_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0111_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1000_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1001_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1010_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1011_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1100_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1101_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1110_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1111_16xh_c(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0000_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0001_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0010_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0011_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0100_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0101_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0110_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir0111_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1000_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1001_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1010_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1011_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1100_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1101_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1110_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);
void mc_cache_bidir1111_8xh_c (uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);














