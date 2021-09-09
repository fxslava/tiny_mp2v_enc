// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include "mp2v_luts.hpp"

macroblock_type_vlc_t snr_macroblock_type[3] = {
    { { 0b1,   1 }, 0b000100 }, //0x80
    { { 0b01,  2 }, 0b100100 },
    { { 0b001, 3 }, 0b000000 }
};

vlc_t dmvector_to_vlc[3] = {
    { 0b11, 2}, // -1
    { 0b0,  1}, //  0
    { 0b10, 2}, //  1
};