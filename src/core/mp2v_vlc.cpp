// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#include "mp2v_vlc.h"
#include "common/cpu.h"

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock addressing
vlc_t macroblock_address_increment_to_vlc[35] = {
    { 0b0, 0 },           // 0 , val: none
    // nlz 0
    { 0b1, 1 },           // 1 , val: 1
    // nlz 1
    { 0b011, 3 },         // 2 , val: 3
    { 0b010, 3 },         // 3 , val: 2
    // nlz 2
    { 0b0011, 4 },        // 4 , val: 3
    { 0b0010, 4 },        // 5 , val: 2
    // nlz 3
    { 0b00011, 5 },       // 6 , val: 3
    { 0b00010, 5 },       // 7 , val: 2
    // nlz 4
    { 0b0000111, 7 },     // 8 , val: 7
    { 0b0000110, 7 },     // 9 , val: 6
    // nlz 4
    { 0b00001011, 8 },    // 10, val: 11
    { 0b00001010, 8 },    // 11, val: 10
    { 0b00001001, 8 },    // 12, val: 9
    { 0b00001000, 8 },    // 13, val: 8
    // nlz 5
    { 0b00000111, 8 },    // 14, val: 7
    { 0b00000110, 8 },    // 15, val: 6
    // nlz 5
    { 0b0000010111, 10 },  // 16, val: 23
    { 0b0000010110, 10 },  // 17, val: 22
    { 0b0000010101, 10 },  // 18, val: 21
    { 0b0000010100, 10 },  // 19, val: 20
    { 0b0000010011, 10 },  // 20, val: 19
    { 0b0000010010, 10 },  // 21, val: 18
    // nlz 5
    { 0b00000100011, 11 }, // 22, val: 35
    { 0b00000100010, 11 }, // 23, val: 34
    { 0b00000100001, 11 }, // 24, val: 33
    { 0b00000100000, 11 }, // 25, val: 32
    // nlz 6
    { 0b00000011111, 11 }, // 26, val: 31
    { 0b00000011110, 11 }, // 27, val: 30
    { 0b00000011101, 11 }, // 28, val: 29
    { 0b00000011100, 11 }, // 29, val: 28
    { 0b00000011011, 11 }, // 30, val: 27
    { 0b00000011010, 11 }, // 31, val: 26
    { 0b00000011001, 11 }, // 32, val: 25
    { 0b00000011000, 11 }, // 33, val: 24
    vlc_macroblock_escape_code
};

int32_t get_macroblock_address_increment(bitstream_reader_i* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    int nlz = bit_scan_reverse(buffer);
    switch (nlz) {
    case 0: return bs->read_next_bits(1);
    case 1: return 5  - bs->read_next_bits(3);
    case 2: return 7  - bs->read_next_bits(4);
    case 3: return 9  - bs->read_next_bits(5);
    case 4:
        if (buffer > 0x0B000000)
            return 15 - bs->read_next_bits(7);
        else
            return 22 - bs->read_next_bits(8);
    case 5:
        if (buffer > 0x05C00000)
            return 21 - bs->read_next_bits(10);
        else if (buffer > 0x04600000)
            return 39 - bs->read_next_bits(11);
    case 6: return 57 - bs->read_next_bits(11);
    }
    return 0; // invalid address increment
}

macroblock_type_vlc_t i_macroblock_type[2] = {
    { 0b1,      1, 0b000010},
    { 0b01,     2, 0b100010}
};

macroblock_type_vlc_t p_macroblock_type[7] = {
    { 0b1,      1, 0b010100 },
    { 0b01,     2, 0b000100 },//0x40
    { 0b001,    3, 0b010000 },//0x20
    { 0b00011,  5, 0b000010 },//0x18
    { 0b00010,  5, 0b110100 },
    { 0b00001,  5, 0b100100 },
    { 0b000001, 6, 0b100010 } //0x04
};

macroblock_type_vlc_t b_macroblock_type[11] = {
    { 0b10,     2, 0b011000 },
    { 0b11,     2, 0b011100 },
    { 0b010,    3, 0b001000 }, //0x40
    { 0b011,    3, 0b001100 },
    { 0b0010,   4, 0b010000 }, //0x20
    { 0b0011,   4, 0b010100 },
    { 0b00011,  5, 0b000010 }, //0x18
    { 0b00010,  5, 0b111100 },
    { 0b000011, 6, 0b110100 }, //0x0C
    { 0b000010, 6, 0b101100 },
    { 0b000001, 6, 0b100010 }
};

uint8_t get_macroblock_type(bitstream_reader_i* bs, int picture_coding_type) {
    uint8_t buffer = (uint8_t)bs->get_next_bits(8);
    if (picture_coding_type == 1) {// Intra
        if (buffer > 0x04) return 0b000010;
        else               return 0b100010;
    }
    if (picture_coding_type == 2) {// Pred
        if (buffer > 0x40) { bs->skip_bits(1);  return 0b010100; } else
        if (buffer > 0x20) { bs->skip_bits(2);  return 0b000100; } else
        if (buffer > 0x18) { bs->skip_bits(3);  return 0b010000; } else
        if (buffer > 0x04) {
            int val = bs->read_next_bits(5);
            if (val == 3) return 0b000010;
            if (val == 2) return 0b110100;
            if (val == 1) return 0b100100;
        } else {
            bs->skip_bits(6);
            return 0b100010;
        }
    }
    if (picture_coding_type == 3) {// Bidir
        if (buffer > 0x40) {
            int val = bs->read_next_bits(2);
            if (val == 2) return 0b011000;
            if (val == 3) return 0b011100;
        } else
        if (buffer > 0x20) {
            int val = bs->read_next_bits(3);
            if (val == 2) return 0b001000;
            if (val == 3) return 0b001100;
        } else
        if (buffer > 0x18) { 
            int val = bs->read_next_bits(4);
            if (val == 2) return 0b010000;
            if (val == 3) return 0b010100;
        } else
        if (buffer > 0x0C) {
            int val = bs->read_next_bits(5);
            if (val == 3) return 0b000010;
            if (val == 2) return 0b111100;
        } else {
            int val = bs->read_next_bits(6);
            if (val == 3) return 0b110100;
            if (val == 2) return 0b101100;
            if (val == 1) return 0b100010;
        }
    }
}

macroblock_type_vlc_t ss_i_macroblock_type[5] = {
    { 0b1,         1, 0b000100 },
    { 0b01,        2, 0b100100 }, //0x04
    { 0b0011,      4, 0b000010 }, //0x03
    { 0b0010,      4, 0b100010 },
    { 0b0001,      4, 0b000000 }
};

macroblock_type_vlc_t ss_p_macroblock_type[16] = {
    { 0b11,        2, 0b110101 },
    { 0b10,        2, 0b010100 },
    { 0b011,       3, 0b010101 }, //0x06
    { 0b010,       3, 0b110100 },
    { 0b0011,      4, 0b010001 }, //0x03
    { 0b0010,      4, 0b010000 },
    { 0b000111,    6, 0b000101 }, //0x1C
    { 0b000110,    6, 0b000001 },
    { 0b000101,    6, 0b100101 },
    { 0b000100,    6, 0b100100 },
    { 0b0000111,   7, 0b000010 }, //0x0E
    { 0b0000110,   7, 0b100010 },
    { 0b0000101,   7, 0b000100 },
    { 0b0000100,   7, 0b000100 },
    { 0b0000011,   7, 0b000000 },
    { 0b0000010,   7, 0b100100 }
};

macroblock_type_vlc_t ss_b_macroblock_type[20] = {
    { 0b11,        2, 0b011100 },
    { 0b10,        2, 0b011000 },
    { 0b011,       3, 0b001100 }, //0x060
    { 0b010,       3, 0b001000 },
    { 0b0011,      4, 0b010100 }, //0x030
    { 0b0010,      4, 0b010000 },
    { 0b000111,    6, 0b001101 }, //0x1C0
    { 0b000110,    6, 0b001001 },
    { 0b000101,    6, 0b010101 },
    { 0b000100,    6, 0b010001 },
    { 0b0000111,   7, 0b111100 }, //0x0E0
    { 0b0000110,   7, 0b000010 },
    { 0b0000101,   7, 0b101100 },
    { 0b0000100,   7, 0b110100 },
    { 0b00000101,  8, 0b110101 }, //0x050
    { 0b00000100,  8, 0b100010 },
    { 0b000001111, 9, 0b000100 }, //0x078
    { 0b000001110, 9, 0b000000 },
    { 0b000001101, 9, 0b100100 },
    { 0b000001100, 9, 0b101101 }
};

uint8_t get_spatial_scalability_macroblock_type(bitstream_reader_i* bs, int picture_coding_type) {
    if (picture_coding_type == 1) {// Intra
        uint8_t buffer = (uint8_t)bs->get_next_bits(8);
        if (buffer > 0x04) { bs->skip_bits(1); return 0b000100; } else
        if (buffer > 0x03) { bs->skip_bits(2); return 0b100100; } else {
            int val = bs->read_next_bits(4);
            if (val == 3) return 0b000010;
            if (val == 2) return 0b100010;
            if (val == 1) return 0b000000;
        }
    }
    if (picture_coding_type == 2) {// Pred
        uint8_t buffer = (uint8_t)bs->get_next_bits(8);
        if (buffer > 0x06) { 
        } else
        if (buffer > 0x03) { 
        } else
        if (buffer > 0x1C) { 
        } else
        if (buffer > 0x0E) {
            int val = bs->read_next_bits(5);
            if (val == 3) return 0b000010;
            if (val == 2) return 0b110100;
            if (val == 1) return 0b100100;
        }
    }
    if (picture_coding_type == 3) {// Bidir
        uint8_t buffer = (uint8_t)bs->get_next_bits(12);
    }
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.9 Macroblock pattern
vlc_t coded_block_pattern_to_vlc[64] = {
    {0b000000001, 9 },    {0b01011,     5 },    {0b01001,     5 },    {0b001101,    6 },
    {0b1101,      4 },    {0b0010111,   7 },    {0b0010011,   7 },    {0b00011111,  8 },
    {0b1100,      4 },    {0b0010110,   7 },    {0b0010010,   7 },    {0b00011110,  8 },
    {0b10011,     5 },    {0b00011011,  8 },    {0b00010111,  8 },    {0b00010011,  8 },
    {0b1011,      4 },    {0b0010101,   7 },    {0b0010001,   7 },    {0b00011101,  8 },
    {0b10001,     5 },    {0b00011001,  8 },    {0b00010101,  8 },    {0b00010001,  8 },
    {0b001111,    6 },    {0b00001111,  8 },    {0b00001101,  8 },    {0b000000011, 9 },
    {0b01111,     5 },    {0b00001011,  8 },    {0b00000111,  8 },    {0b000000111, 9 },
    {0b1010,      4 },    {0b0010100,   7 },    {0b0010000,   7 },    {0b00011100,  8 },
    {0b001110,    6 },    {0b00001110,  8 },    {0b00001100,  8 },    {0b000000010, 9 },
    {0b10000,     5 },    {0b00011000,  8 },    {0b00010100,  8 },    {0b00010000,  8 },
    {0b01110,     5 },    {0b00001010,  8 },    {0b00000110,  8 },    {0b000000110, 9 },
    {0b10010,     5 },    {0b00011010,  8 },    {0b00010110,  8 },    {0b00010010,  8 },
    {0b01101,     5 },    {0b00001001,  8 },    {0b00000101,  8 },    {0b000000101, 9 },
    {0b01100,     5 },    {0b00001000,  8 },    {0b00000100,  8 },    {0b000000100, 9 },
    {0b111,       3 },    {0b01010,     5 },    {0b01000,     5 },    {0b001100,    6 },
};
                                                               // 9 8 7 6 5 4 3 2
constexpr uint64_t vlclen_2_to_coded_block_pattern_to_offset = 0x391D151105010000;

int32_t vlc_to_coded_block_pattern[64] = {
    60, 4, 8,16,32,12,48,20,40,28,44,52,56, 1,61, 2,62,24,36, 3,63, 5, 9,17,33, 6,10,18,34, 7,11,19,
    35,13,49,21,41,14,50,22,42,15,51,23,43,25,37,26,38,29,45,53,57,30,46,54,58,31,47,55,59,27,39, 0,
};

int32_t get_coded_block_pattern(vlc_t vlc) {
    uint64_t mask = 0xff;
    mask <<= (vlc.len - 2) * 8;
    uint32_t idx = static_cast<uint32_t>(mask & vlclen_2_to_coded_block_pattern_to_offset) + vlc.value;
    return vlc_to_coded_block_pattern[idx];
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.10 Table
vlc_t motion_code_to_vlc[] = {
    { 0b00000011001, 11 }, // -16
    { 0b00000011011, 11 }, // -15
    { 0b00000011101, 11 }, // -14
    { 0b00000011111, 11 }, // -13
    { 0b00000100001, 11 }, // -12
    { 0b00000100011, 11 }, // -11
    { 0b0000010011 , 10 }, // -10
    { 0b0000010101 , 10 }, //  -9
    { 0b0000010111 , 10 }, //  -8
    { 0b00000111   ,  8 }, //  -7
    { 0b00001001   ,  8 }, //  -6
    { 0b00001011   ,  8 }, //  -5
    { 0b0000111    ,  7 }, //  -4
    { 0b00011      ,  5 }, //  -3
    { 0b0011       ,  4 }, //  -2
    { 0b011        ,  3 }, //  -1
    { 0b1          ,  1 }, //   0
    { 0b010        ,  3 }, //   1
    { 0b0010       ,  4 }, //   2
    { 0b00010      ,  5 }, //   3
    { 0b0000110    ,  7 }, //   4
    { 0b00001010   ,  8 }, //   5
    { 0b00001000   ,  8 }, //   6
    { 0b00000110   ,  8 }, //   7
    { 0b0000010110 , 10 }, //   8
    { 0b0000010100 , 10 }, //   9
    { 0b0000010010 , 10 }, //  10
    { 0b00000100010, 11 }, //  11
    { 0b00000100000, 11 }, //  12
    { 0b00000011110, 11 }, //  13
    { 0b00000011100, 11 }, //  14
    { 0b00000011010, 11 }, //  15
    { 0b00000011000, 11 }  //  16
};

constexpr uint32_t vlclen_motion_code_to_offset = 0x157A4E1;
int32_t vlc_to_motion_code[33] = { 0,1,-1,2,-2,3,-3,4,-4,7,-7,6,-6,5,-5,10,-10,9,-9,8,-8,16,-16,15,-15,14,-14,13,-13,12,-12,11,-11 };

int32_t get_motion_code(vlc_t vlc) {
    uint64_t mask = 0x1f;
    mask <<= vlc.len * 5;
    uint32_t idx = static_cast<uint32_t>(mask & vlclen_motion_code_to_offset) + vlc.value;
    return vlc_to_coded_block_pattern[idx];
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.11 Table.
vlc_t dmvector_to_vlc[3] = {
    { 0b11, 2}, // -1
    { 0b0,  1}, //  0
    { 0b10, 2}, //  1
};

int32_t get_dmvector(vlc_t vlc) {
    return vlc.len == 1 ? 0 : (vlc.value == 2 ? 1 : -1);
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.12 Table.
vlc_t dct_size_luminance_to_vlc[12] = {
    { 0b100, 0 },
    { 0b00, 1 },
    { 0b01, 2 },
    { 0b101, 3 },
    { 0b110, 4 },
    { 0b1110, 5 },
    { 0b11110, 6 },
    { 0b111110, 7 },
    { 0b1111110, 8 },
    { 0b11111110, 9 },
    { 0b111111110, 10 },
    { 0b111111111, 11 }
};

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.13 Table.
vlc_t dct_size_chrominance_to_vlc[12] = {
    { 0b00, 0 },
    { 0b01, 1 },
    { 0b10, 2 },
    { 0b110, 3 },
    { 0b1110, 4 },
    { 0b11110, 5 },
    { 0b111110, 6 },
    { 0b1111110, 7 },
    { 0b11111110, 8 },
    { 0b111111110, 9 },
    { 0b1111111110, 10 },
    { 0b1111111111, 11 }
};