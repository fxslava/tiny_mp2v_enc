// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include "api/bitstream.h"

struct vlc_t {
    uint16_t value;
    uint16_t len;
};
struct macroblock_type_vlc_t {
    uint16_t code;
    uint16_t len;
    uint8_t value;
};

constexpr vlc_t    vlc_start_code = { 0x000001, 24 };
constexpr uint32_t macroblock_escape_code = 34;
constexpr vlc_t    vlc_macroblock_escape_code = { 0b00000001000, 11 };
constexpr uint32_t macroblock_quant_bit = 0b100000;
constexpr uint32_t macroblock_motion_forward_bit = 0b10000;
constexpr uint32_t macroblock_motion_backward_bit = 0b1000;
constexpr uint32_t macroblock_pattern_bit = 0b100;
constexpr uint32_t macroblock_intra_bit = 0b10;
constexpr uint32_t spatial_temporal_weight_code_flag_bit = 0b1;

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock addressing
extern vlc_t macroblock_address_increment_to_vlc[35];
int32_t get_macroblock_address_increment_lut(bitstream_reader_i* bs); // TODO: find best method
int32_t get_macroblock_address_increment(bitstream_reader_i* bs);

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock type (Tables B.2 to B.8.)
extern macroblock_type_vlc_t i_macroblock_type[2];
extern macroblock_type_vlc_t p_macroblock_type[7];
extern macroblock_type_vlc_t b_macroblock_type[11];
uint8_t get_macroblock_type(bitstream_reader_i* bs, int picture_coding_type);
extern macroblock_type_vlc_t ss_i_macroblock_type[5];
extern macroblock_type_vlc_t ss_p_macroblock_type[16];
extern macroblock_type_vlc_t ss_b_macroblock_type[20];
uint8_t get_spatial_scalability_macroblock_type(bitstream_reader_i* bs, int picture_coding_type);
extern macroblock_type_vlc_t snr_macroblock_type[3];
uint8_t get_snr_scalability_macroblock_type(bitstream_reader_i* bs, int picture_coding_type);

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.9 Macroblock pattern
extern vlc_t coded_block_pattern_to_vlc[64];
int32_t get_coded_block_pattern(bitstream_reader_i* bs);

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.10 Table
extern vlc_t motion_code_to_vlc[33];
int32_t get_motion_code(bitstream_reader_i* bs);

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.11 Table.
extern vlc_t dmvector_to_vlc[3];
int32_t get_dmvector(bitstream_reader_i* bs);

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.12 Table.
extern vlc_t dct_size_luminance_to_vlc[12];
int32_t get_dct_size_luminance(bitstream_reader_i* bs);

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.13 Table.
extern vlc_t dct_size_chrominance_to_vlc[12];
int32_t get_dct_size_chrominance(bitstream_reader_i* bs);