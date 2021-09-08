// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <fstream>
#include <vector>
#include "core/common/cpu.hpp"

class bitstream_reader_c {
private:
    MP2V_INLINE void read32() {
        buffer <<= 32;
        uint32_t tmp = *buffer_ptr;
        buffer_ptr++;
        buffer |= (uint64_t)_byteswap_ulong(tmp);
        buffer_idx -= 32;
    }

    MP2V_INLINE void update_buffer() {
        if (buffer_idx >= 32)
            read32();
    }
public:
    bitstream_reader_c(std::string filename) :
        buffer(0),
        buffer_idx(64)
    {
        std::ifstream fp(filename, std::ios::binary);

        // Calculate size of buffer
        fp.seekg(0, std::ios_base::end);
        std::size_t size = fp.tellg();
        fp.seekg(0, std::ios_base::beg);

        // Allocate buffer
        buffer_pool.resize(size / sizeof(uint32_t));
        buffer_ptr = &buffer_pool[0];
        buffer_end = &buffer_pool[buffer_pool.size() - 1];

        // read file
        fp.read((char*)buffer_ptr, size);
        fp.close();
    }

    ~bitstream_reader_c() {}

    MP2V_INLINE uint32_t get_next_bits(int len) {
        update_buffer();
        uint64_t mask = (1ll << len) - 1;
        uint64_t tmp = buffer << buffer_idx;
        return (tmp >> (64 - len)) & mask;
    }

    MP2V_INLINE uint32_t read_next_bits(int len) {
        uint32_t tmp = get_next_bits(len);
        buffer_idx += len;
        return tmp;
    }

    MP2V_INLINE bool seek_pattern(uint32_t pattern, int len) {
        do {
            update_buffer();
            int range = 64 - buffer_idx - len;
            uint64_t tmp = buffer << buffer_idx;
            uint64_t mask = (1ll << len) - 1;
            int offset = 64 - len;
            for (int pos = 0; pos < range; pos++) {
                if ((uint32_t)((tmp >> offset) & mask) == pattern) {
                    buffer_idx += pos;
                    return true;
                }
                tmp <<= 1;
            }
            buffer_idx += range;
        } while (buffer_ptr < buffer_end);
        return false;
    }

    MP2V_INLINE void skip_bits(int len) {
        buffer_idx += len;
    }

private:
    //FILE* bitstream = nullptr;
    std::vector<uint32_t> buffer_pool;
    uint32_t* buffer_ptr = nullptr;
    uint32_t* buffer_end = nullptr;
    uint64_t  buffer = 0;
    uint32_t  buffer_idx = 64;
};