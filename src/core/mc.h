#pragma once
#include <stdint.h>

enum mc_type_e { MC_00, MC_10, MC_01, MC_11 };

typedef void(*mc_pred_func_t)(uint8_t* dst, uint8_t* src, uint32_t stride, uint32_t height);
typedef void(*mc_bidir_func_t)(uint8_t* dst, uint8_t* src0, uint8_t* src1, uint32_t stride, uint32_t height);

extern mc_pred_func_t  mc_pred_16xh[4];
extern mc_pred_func_t  mc_pred_8xh[4];
extern mc_bidir_func_t mc_bidir_16xh[16];
extern mc_bidir_func_t mc_bidir_8xh[16];