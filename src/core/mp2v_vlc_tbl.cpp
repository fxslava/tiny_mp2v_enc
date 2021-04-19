// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#include "mp2v_vlc_tbl.h"

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock addressing
vlc_t macroblock_address_increment_to_vlc[35] = {
    { 0b0, 0 },           // 0 , val: none
    { 0b1, 1 },           // 1 , val: 1
    { 0b011, 3 },         // 2 , val: 3
    { 0b010, 3 },         // 3 , val: 2
    { 0b0011, 4 },        // 4 , val: 3
    { 0b0010, 4 },        // 5 , val: 2
    { 0b00011, 5 },       // 6 , val: 3
    { 0b00010, 5 },       // 7 , val: 2
    { 0b0000111, 6 },     // 8 , val: 7
    { 0b0000110, 6 },     // 9 , val: 6
    { 0b00001011, 7 },    // 10, val: 11
    { 0b00001010, 7 },    // 11, val: 10
    { 0b00001001, 7 },    // 12, val: 9
    { 0b00001000, 7 },    // 13, val: 8
    { 0b00000111, 7 },    // 14, val: 7
    { 0b00000110, 7 },    // 15, val: 6
    { 0b0000010111, 8 },  // 16, val: 23
    { 0b0000010110, 8 },  // 17, val: 22
    { 0b0000010101, 8 },  // 18, val: 21
    { 0b0000010100, 8 },  // 19, val: 20
    { 0b0000010011, 8 },  // 20, val: 19
    { 0b0000010010, 8 },  // 21, val: 18
    { 0b00000100011, 9 }, // 22, val: 35
    { 0b00000100010, 9 }, // 23, val: 34
    { 0b00000100001, 9 }, // 24, val: 33
    { 0b00000100000, 9 }, // 25, val: 32
    { 0b00000011111, 9 }, // 26, val: 31
    { 0b00000011110, 9 }, // 27, val: 30
    { 0b00000011101, 9 }, // 28, val: 29
    { 0b00000011100, 9 }, // 29, val: 28
    { 0b00000011011, 9 }, // 30, val: 27
    { 0b00000011010, 9 }, // 31, val: 25
    { 0b00000011001, 9 }, // 32, val: 24
    { 0b00000011000, 9 }, // 33, val: 23
    { 0b00000001000, 9 }  // macroblock_escape
};

constexpr uint64_t vlclen_1_to_macroblock_address_increment_tbl_offset = 0x864626160E0A0602;

int32_t vlc_to_macroblock_address_increment[] = {
    // 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19  20  21  22  23  24  25  26  27  28  29  30  31
      -1,  1,//1
      -1, -1,  3,  2,//3
      -1, -1,  5,  4,//4
      -1, -1,  7,  6,//5
      -1, -1, -1, -1, -1, -1,  9,  8,//6
      -1, -1, -1, -1, -1, -1, 15, 14, 13, 12, 11, 10, -1, -1, -1, -1, -1,//7
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 21, 20, 19, 18, 17, 16, -1, -1, -1, -1, -1, -1, -1, -1, //8
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 34, 33, 32, 31, 30, 29, 28, 27, 26, //9
   // 32  33  34  35  36  37  38  39  40  41  42  43  44  45  46  47  48  49  50  51  52  53  54  55  56  57  58  59  60  61  62  63
      25, 24, 23, 22, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, //9
};

int32_t get_macroblock_address_increment(vlc_t vlc) {
    uint64_t mask = 0xff;
    mask <<= (vlc.len - 1);
    uint32_t idx = static_cast<uint32_t>(mask & vlclen_1_to_macroblock_address_increment_tbl_offset) + vlc.value;
    return vlc_to_macroblock_address_increment[idx];
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.3 Macroblock pattern
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

/*
0b111	    3	60
0b1101	    4	4
0b1100	    4	8
0b1011	    4	16
0b1010	    4	32
0b10011	    5	12
0b10010	    5	48
0b10001	    5	20
0b10000	    5	40
0b01111	    5	28
0b01110	    5	44
0b01101	    5	52
0b01100	    5	56
0b01011	    5	1
0b01010	    5	61
0b01001	    5	2
0b01000	    5	62
0b001111	6	24
0b001110	6	36
0b001101	6	3
0b001100	6	63
0b0010111	7	5
0b0010110	7	9
0b0010101	7	17
0b0010100	7	33
0b0010011	7	6
0b0010010	7	10
0b0010001	7	18
0b0010000	7	34
0b00011111	8	7
0b00011110	8	11
0b00011101	8	19
0b00011100	8	35
0b00011011	8	13
0b00011010	8	49
0b00011001	8	21
0b00011000	8	41
0b00010111	8	14
0b00010110	8	50
0b00010101	8	22
0b00010100	8	42
0b00010011	8	15
0b00010010	8	51
0b00010001	8	23
0b00010000	8	43
0b00001111	8	25
0b00001110	8	37
0b00001101	8	26
0b00001100	8	38
0b00001011	8	29
0b00001010	8	45
0b00001001	8	53
0b00001000	8	57
0b00000111	8	30
0b00000110	8	46
0b00000101	8	54
0b00000100	8	58
0b000000111	9	31
0b000000110	9	47
0b000000101	9	55
0b000000100	9	59
0b000000011	9	27
0b000000010	9	39
0b000000001	9	0
*/