// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include "common/cpu.hpp"
#include "api/bitstream.h"

struct vlc_t {
    uint16_t value;
    uint16_t len;
};
struct macroblock_type_vlc_t {
    vlc_t vlc;
    uint8_t value;
};

struct coeff_t {
    uint8_t run;
    uint8_t level;
};

struct vlc_coeff_t {
    vlc_t vlc;
    coeff_t coeff;
};

// Internal types
struct vlc_value_t {
    int8_t vlc_len;
    int8_t value;
};

struct vlc_lut_coeff_t {
    int16_t len;
    coeff_t coeff;
};

constexpr vlc_t    vlc_start_code = { 0x000001, 24 };
constexpr uint32_t macroblock_escape_code = 34;
constexpr vlc_t    vlc_macroblock_escape_code = { 0b00000001000, 11 };;

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock addressing
extern vlc_t macroblock_address_increment_to_vlc[35];

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock type (Tables B.2 to B.8.)
extern macroblock_type_vlc_t i_macroblock_type[2];
extern macroblock_type_vlc_t p_macroblock_type[7];
extern macroblock_type_vlc_t b_macroblock_type[11];
extern macroblock_type_vlc_t ss_i_macroblock_type[5];
extern macroblock_type_vlc_t ss_p_macroblock_type[16];
extern macroblock_type_vlc_t ss_b_macroblock_type[20];
extern macroblock_type_vlc_t snr_macroblock_type[3];

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.9 Macroblock pattern
extern vlc_t coded_block_pattern_to_vlc[64];

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.10 Table
extern vlc_t motion_code_to_vlc[33];

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.11 Table.
extern vlc_t dmvector_to_vlc[3];

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.12 Table.
extern vlc_t dct_size_luminance_to_vlc[12];

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.13 Table.
extern vlc_t dct_size_chrominance_to_vlc[12];

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.14 Table.
extern vlc_coeff_t coeff_zero_vlc[111];

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.15 Table.
extern vlc_coeff_t coeff_one_vlc[111];

#include "mp2v_vlc_dec.hpp"
DEFINE_CAVLC_METHODS(bitstream_reader_c)