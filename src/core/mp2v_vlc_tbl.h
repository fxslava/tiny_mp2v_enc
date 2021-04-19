// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <stdint.h>

struct vlc_t {
    uint16_t value;
    uint16_t len;
};

constexpr uint32_t vlc_macroblock_escape_code = 34;

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock addressing
vlc_t   macroblock_address_increment_to_vlc[35];
int32_t get_macroblock_address_increment(vlc_t vlc);

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.3 Macroblock pattern