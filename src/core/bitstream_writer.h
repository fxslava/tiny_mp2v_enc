#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <vector>

class bitstream_writer_c {
    void update() {
        if (bit_idx > 32) {
            buffer.push_back(bswap_32((uint32_t)(bit_buffer & 0xffffffff)));
            bit_idx -= 32;
        }
    }
public:

    void write_bits(uint32_t val, int len) {
        bit_buffer |= ((uint64_t)val << bit_idx);
        bit_idx += len;
        update();
    }

    void one_bit() {
        bit_buffer |= (1ull << bit_idx);
        bit_idx++;
        update();
    }

    void zero_bit() {
        bit_idx++;
        update();
    }

    void output_bitsream(uint32_t*& buf, size_t& size) {
        buf = &buffer[0];
        size = buffer.size();
    }
protected:
    uint32_t  bit_idx = 0;
    uint64_t  bit_buffer = 0;
    std::vector<uint32_t> buffer;
};
