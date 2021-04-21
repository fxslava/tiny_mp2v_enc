// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>

struct vlc_t {
    uint16_t value;
    uint16_t len;
};

constexpr vlc_t    vlc_start_code = { 0x000001, 24 };
constexpr uint32_t macroblock_escape_code = 34;
constexpr vlc_t    vlc_macroblock_escape_code = { 0b00000001000, 11 };

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock addressing
extern vlc_t   macroblock_address_increment_to_vlc[35];
int32_t get_macroblock_address_increment(vlc_t vlc);

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.9 Macroblock pattern
extern int32_t vlc_to_coded_block_pattern[64];
int32_t get_coded_block_pattern(vlc_t vlc);

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.10 Table
extern int32_t vlc_to_motion_code[33];
int32_t get_motion_code(vlc_t vlc);

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.11 Table.
extern vlc_t dmvector_to_vlc[3];
int32_t get_dmvector(vlc_t vlc);

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.12 Table.
extern vlc_t dct_size_luminance_to_vlc[12];

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.13 Table.
extern vlc_t dct_size_chrominance_to_vlc[12];