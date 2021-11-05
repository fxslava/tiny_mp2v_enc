#pragma once
#include "common/cpu.hpp"
#include "bitstream_writer.h"

void encode_mb(bitstream_writer_c& bs, uint8_t* yuv_ptrs[3], int16_t(&dc_pred)[3], int strides[3], int qp, int dc_prec);
