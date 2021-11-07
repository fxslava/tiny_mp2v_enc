#pragma once
#include "common/cpu.hpp"
#include <stdint.h>
#include <stdlib.h>
#include <vector>

class bitstream_writer_c {
    MP2V_INLINE void update() {
        if (bit_idx <= 32) {
            buffer.push_back(bswap_32((uint32_t)(bit_buffer >> 32)));
            bit_buffer <<= 32;
            bit_idx += 32;
        }
    }
public:

    MP2V_INLINE void write_bits(uint32_t val, int len) {
        bit_idx -= len;
        bit_buffer |= ((uint64_t)val << bit_idx);
        update();
    }

    MP2V_INLINE void one_bit() {
        bit_idx--;
        bit_buffer |= (1ull << bit_idx);
        update();
    }

    MP2V_INLINE void zero_bit() {
        bit_idx--;
        update();
    }

    MP2V_INLINE void output_bitsream(uint32_t*& buf, size_t& size) {
        buf = &buffer[0];
        size = buffer.size();
    }

    MP2V_INLINE void align() {
        bit_idx = (bit_idx & ~7);
        update();
    }

    MP2V_INLINE void flush() {
        update();
        buffer.push_back(bswap_32((uint32_t)(bit_buffer >> 32)));
        bit_idx = 64;
        bit_buffer = 0;
    }
protected:
    uint32_t  bit_idx = 64;
    uint64_t  bit_buffer = 0;
    std::vector<uint32_t> buffer;
};
