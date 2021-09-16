#pragma once
#include "common/cpu.hpp"
#include "mp2v_vlc.h"
#include <algorithm>

// Internal usage arrays
extern vlc_value_t vlc_mba[8][64];
extern uint8_t p_macroblock_type_0x08[5];
extern uint8_t b_macroblock_type_0x80[4];
extern uint8_t b_macroblock_type_0x40[4];
extern uint8_t b_macroblock_type_0x20[4];
extern uint8_t b_macroblock_type_0x10[4];
extern uint8_t b_macroblock_type_else[5];
extern uint8_t ss_i_macroblock_type_else[4];
extern uint8_t ss_p_macroblock_type_0x80[4];
extern uint8_t ss_p_macroblock_type_0x40[4];
extern uint8_t ss_p_macroblock_type_0x20[4];
extern uint8_t ss_p_macroblock_type_0x10[8];
extern uint8_t ss_p_macroblock_type_else[8];
extern uint8_t ss_b_macroblock_type_0x80[4];
extern uint8_t ss_b_macroblock_type_0x40[4];
extern uint8_t ss_b_macroblock_type_0x20[4];
extern uint8_t ss_b_macroblock_type_0x10[4];
extern uint8_t ss_b_macroblock_type_0x08[4];
extern uint8_t ss_b_macroblock_type_0x06[4];
extern uint8_t ss_b_macroblock_type_else[4];
extern vlc_value_t vlc_motion_code[7][64];
extern vlc_value_t vlc_cbp[9][32];
extern coeff_t vlc_coeff_zero_ex[8];
extern vlc_lut_coeff_t vlc_coeff_zero[12][32];
extern vlc_lut_coeff_t vlc_coeff_one0[11][32];
extern vlc_lut_coeff_t vlc_coeff_one1[8][8];
extern coeff_t vlc_coeff_one_ex[8];

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.1 Macroblock addressing
template<class bitstream_reader_t>
MP2V_INLINE int32_t get_macroblock_address_increment_lut_template(bitstream_reader_t* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    int nlz = bit_scan_reverse(buffer);
    int idx = buffer >> (32 - nlz - 6);
    int val = vlc_mba[nlz][idx].value;
    bs->skip_bits(vlc_mba[nlz][idx].vlc_len);
    return val;
}

template<class bitstream_reader_t>
MP2V_INLINE int32_t get_macroblock_address_increment_template(bitstream_reader_t* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    int nlz = bit_scan_reverse(buffer);
    switch (nlz) {
    case 0: return bs->read_next_bits(1);
    case 1: return 5  - bs->read_next_bits(3);
    case 2: return 7  - bs->read_next_bits(4);
    case 3: return 9  - bs->read_next_bits(5);
    case 4:
        if (buffer >= 0x0C000000)
            return 15 - bs->read_next_bits(7);
        else
            return 21 - bs->read_next_bits(8);
    case 5:
        if (buffer >= 0x06000000)
            return 21 - bs->read_next_bits(8);
        else if (buffer >= 0x04800000)
            return 39 - bs->read_next_bits(10);
        else
            return 57 - bs->read_next_bits(11);
    case 6: return 57 - bs->read_next_bits(11);
    }
    return 0; // invalid address increment
}

template<class bitstream_reader_t>
MP2V_INLINE uint8_t get_macroblock_type_template(bitstream_reader_t* bs, int picture_coding_type) {
    uint8_t buffer = (uint8_t)bs->get_next_bits(8);
    if (picture_coding_type == 1) {// I
        if (buffer >= 0x80) { bs->skip_bits(1);  return 0b000010; }
        else                { bs->skip_bits(2);  return 0b100010; }
    }
    if (picture_coding_type == 2) {// P
        if (buffer >= 0x80) { bs->skip_bits(1);  return 0b010100; } else
        if (buffer >= 0x40) { bs->skip_bits(2);  return 0b000100; } else
        if (buffer >= 0x20) { bs->skip_bits(3);  return 0b010000; } else
        if (buffer >= 0x08) return p_macroblock_type_0x08[bs->read_next_bits(5)]; else { 
         bs->skip_bits(6);  return 0b100010; }
    }
    if (picture_coding_type == 3) {// B
        if (buffer >= 0x80) return b_macroblock_type_0x80[bs->read_next_bits(2)]; else
        if (buffer >= 0x40) return b_macroblock_type_0x40[bs->read_next_bits(3)]; else
        if (buffer >= 0x20) return b_macroblock_type_0x20[bs->read_next_bits(4)]; else
        if (buffer >= 0x10) return b_macroblock_type_0x10[bs->read_next_bits(5)];
        else                return b_macroblock_type_else[bs->read_next_bits(6)];
    }
    return true;
}

template<class bitstream_reader_t>
MP2V_INLINE uint8_t get_spatial_scalability_macroblock_type_template(bitstream_reader_t* bs, int picture_coding_type) {
    if (picture_coding_type == 1) {// Intra
        uint8_t buffer = (uint8_t)bs->get_next_bits(8);
        if (buffer >= 0x80) { bs->skip_bits(1); return 0b000100; } else
        if (buffer >= 0x40) { bs->skip_bits(2); return 0b100100; } else
            return ss_i_macroblock_type_else[bs->read_next_bits(4)];
    }
    if (picture_coding_type == 2) {// Pred
        uint8_t buffer = (uint8_t)bs->get_next_bits(8);
        if (buffer >= 0x80) return ss_p_macroblock_type_0x80[bs->read_next_bits(2)]; else
        if (buffer >= 0x40) return ss_p_macroblock_type_0x40[bs->read_next_bits(3)]; else
        if (buffer >= 0x20) return ss_p_macroblock_type_0x20[bs->read_next_bits(4)]; else
        if (buffer >= 0x10) return ss_p_macroblock_type_0x10[bs->read_next_bits(6) - 4]; else
            return ss_p_macroblock_type_else[bs->read_next_bits(7)];
    }
    if (picture_coding_type == 3) {// Bidir
        uint8_t buffer = (uint8_t)bs->get_next_bits(8);
        if (buffer >= 0x80) return ss_b_macroblock_type_0x80[bs->read_next_bits(2)]; else
        if (buffer >= 0x40) return ss_b_macroblock_type_0x40[bs->read_next_bits(3)]; else
        if (buffer >= 0x20) return ss_b_macroblock_type_0x20[bs->read_next_bits(4)]; else
        if (buffer >= 0x10) return ss_b_macroblock_type_0x10[bs->read_next_bits(6) - 4]; else
        if (buffer >= 0x08) return ss_b_macroblock_type_0x08[bs->read_next_bits(7) - 4]; else
        if (buffer >= 0x06) return ss_b_macroblock_type_0x06[bs->read_next_bits(9) -12]; else
            return ss_b_macroblock_type_else[bs->read_next_bits(8) - 4];
    }
    return true;
}

template<class bitstream_reader_t>
MP2V_INLINE uint8_t get_snr_scalability_macroblock_type_template(bitstream_reader_t* bs) {
    uint8_t buffer = (uint8_t)bs->get_next_bits(2);
    if (buffer >= 2) { bs->skip_bits(1); return 0b000100; }
    if (buffer >= 1) { bs->skip_bits(2); return 0b100100; }
    bs->skip_bits(3);
    return 0b000000;
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - Variable length code tables. B.9 Macroblock pattern
template<class bitstream_reader_t>
MP2V_INLINE int32_t get_coded_block_pattern_template(bitstream_reader_t* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    int nlz = bit_scan_reverse(buffer);
    int idx = buffer >> (32 - nlz - 5);
    int val = vlc_cbp[nlz][idx].value;
    bs->skip_bits(vlc_cbp[nlz][idx].vlc_len);
    return val;
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.10 Table
template<class bitstream_reader_t>
MP2V_INLINE int32_t get_motion_code_template(bitstream_reader_t* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    int nlz = bit_scan_reverse(buffer);
    int idx = buffer >> (32 - nlz - 6);
    int val = vlc_motion_code[nlz][idx].value;
    bs->skip_bits(vlc_motion_code[nlz][idx].vlc_len);
    return val;
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.4 Motion vectors. B.11 Table.
template<class bitstream_reader_t>
MP2V_INLINE int32_t get_dmvector_template(bitstream_reader_t* bs) {
    int buffer = bs->get_next_bits(2);
    switch (buffer) {
    case 3:
        bs->skip_bits(2); return -1;
    case 0:
    case 1:
        bs->skip_bits(1); return 0;
    case 2:
        bs->skip_bits(2); return 1;
    }
    return 0;
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.12 Table.
template<class bitstream_reader_t>
MP2V_INLINE int32_t get_dct_size_luminance_template(bitstream_reader_t* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    int nlo = bit_scan_reverse(~buffer);
    if (nlo > 1) {
        int val = nlo + 2;
        if (val > 10) {
            bs->skip_bits(9);
            return 11;
        }
        else {
            bs->skip_bits(val - 1);
            return val;
        }
    }
    else {
        buffer >>= 29;
        switch (buffer) {
        case 0b100: bs->skip_bits(3); return 0;
        case 0b000: bs->skip_bits(2); return 1;
        case 0b001: bs->skip_bits(2); return 1;
        case 0b010: bs->skip_bits(2); return 2;
        case 0b011: bs->skip_bits(2); return 2;
        case 0b101: bs->skip_bits(3); return 3;
        }
    }
    return 0;
}

//ISO/IEC 13818-2 : 2000 (E) Annex B - B.5 DCT coefficients. B.13 Table.
template<class bitstream_reader_t>
MP2V_INLINE int32_t get_dct_size_chrominance_template(bitstream_reader_t* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    int nlo = bit_scan_reverse(~buffer);
    if (nlo > 1) {
        int val = nlo + 1;
        if (val > 10) {
            bs->skip_bits(10);
            return 11;
        }
        else {
            bs->skip_bits(val);
            return val;
        }
    }
    else {
        buffer >>= 30;
        bs->skip_bits(2);
        return buffer;
    }
}

template<class bitstream_reader_t>
MP2V_INLINE coeff_t get_coeff_zero_template(bitstream_reader_t* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    if ((buffer >> (32 - 5)) == 0b00100) {
        int idx = (buffer >> (32 - 8)) & 7;
        bs->skip_bits(8);
        return vlc_coeff_zero_ex[idx];
    }
    else {
        int nlz = bit_scan_reverse(buffer);
        int idx = buffer >> (32 - nlz - 5);
        coeff_t val = vlc_coeff_zero[nlz][idx].coeff;
        bs->skip_bits(vlc_coeff_zero[nlz][idx].len);
        return val;
    }
}

template<class bitstream_reader_t>
MP2V_INLINE coeff_t get_coeff_one_template(bitstream_reader_t* bs) {
    uint32_t buffer = bs->get_next_bits(32);
    if ((buffer >> (32 - 5)) == 0b00100) {
        int idx = (buffer >> (32 - 8)) & 7;
        bs->skip_bits(8);
        return vlc_coeff_one_ex[idx];
    }
    else {
        int nlz = bit_scan_reverse(buffer);
        if (nlz > 0) {
            int idx = buffer >> (32 - nlz - 5);
            coeff_t val = vlc_coeff_one0[nlz - 1][idx].coeff;
            bs->skip_bits(vlc_coeff_one0[nlz - 1][idx].len);
            return val;
        }
        else
        {
            nlz = std::min<uint32_t>(bit_scan_reverse(~buffer), 8);
            int idx = (buffer >> (32 - nlz - 3)) & 7;
            coeff_t val = vlc_coeff_one1[nlz - 1][idx].coeff;
            bs->skip_bits(vlc_coeff_one1[nlz - 1][idx].len);
            return val;
        }
    }
}

#define DEFINE_CAVLC_METHODS(STREAM_READER) \
MP2V_INLINE int32_t get_macroblock_address_increment_lut(STREAM_READER* bs) { return get_macroblock_address_increment_lut_template(bs);  } \
MP2V_INLINE int32_t get_macroblock_address_increment(STREAM_READER* bs) { return get_macroblock_address_increment_template(bs); } \
MP2V_INLINE uint8_t get_macroblock_type(STREAM_READER* bs, int picture_coding_type) { return get_macroblock_type_template(bs, picture_coding_type); } \
MP2V_INLINE uint8_t get_spatial_scalability_macroblock_type(STREAM_READER* bs, int picture_coding_type) { return get_spatial_scalability_macroblock_type_template(bs, picture_coding_type); } \
MP2V_INLINE uint8_t get_snr_scalability_macroblock_type(STREAM_READER* bs) { return get_snr_scalability_macroblock_type_template(bs); } \
MP2V_INLINE int32_t get_coded_block_pattern(STREAM_READER* bs) { return get_coded_block_pattern_template(bs); } \
MP2V_INLINE int32_t get_motion_code(STREAM_READER* bs) { return get_motion_code_template(bs); } \
MP2V_INLINE int32_t get_dmvector(STREAM_READER* bs) { return get_dmvector_template(bs); } \
MP2V_INLINE int32_t get_dct_size_luminance(STREAM_READER* bs) { return get_dct_size_luminance_template(bs); } \
MP2V_INLINE int32_t get_dct_size_chrominance(STREAM_READER* bs) { return get_dct_size_chrominance_template(bs); } \
MP2V_INLINE coeff_t get_coeff_zero(STREAM_READER* bs) { return get_coeff_zero_template(bs); } \
MP2V_INLINE coeff_t get_coeff_one(STREAM_READER* bs) { return get_coeff_one_template(bs); }
