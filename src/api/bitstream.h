// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <fstream>
#include <vector>
#include "core/common/cpu.hpp"

class bitstream_reader_c {
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

    // Low level
    MP2V_INLINE uint64_t& get_buffer() {
        return buffer;
    }

    template<int len>
    MP2V_INLINE void flush_bits() {
        buffer <<= len;
        buffer_idx += len;
    }

    MP2V_INLINE void flush_bits(int len) {
        buffer <<= len;
        buffer_idx += len;
    }

    MP2V_INLINE uint64_t& update() {
        if (buffer_idx >= 64) {
            buffer |= bswap_64(*(uint64_t*)(buffer_ptr)) << (buffer_idx - 64);
            buffer_ptr += 2;
            cache = bswap_64(*(uint64_t*)(buffer_ptr)); // precache next value
            buffer_idx -= 64;
            return buffer;
        } 
        else if (buffer_idx >= 32) {
            buffer |= bswap_64(*(uint64_t*)(buffer_ptr++)) >> (64 - buffer_idx);
            cache = bswap_64(*(uint64_t*)(buffer_ptr)); // precache next value
            buffer_idx -= 32;
            return buffer;
        }
        else if (buffer_idx != 0) {
            buffer |= cache >> (64 - buffer_idx);
            return buffer;
        }
    }
    // End of low level

    template<bool update_buffer = true>
    MP2V_INLINE uint32_t get_next_bits(int len) {
        if (update_buffer)
            update();
        return buffer >> (64 - len);
    }

    template<bool update_buffer = true>
    MP2V_INLINE uint32_t read_next_bits(int len) {
        uint32_t tmp = get_next_bits<update_buffer>(len);
        flush_bits(len);
        return tmp;
    }

    MP2V_INLINE bool next_start_code() {
        int current_byte_pos = buffer_idx / 8; // align bit index
        buffer_ptr -= 2;
        auto bytestream = (uint8_t*)buffer_ptr;
        int zcnt = 0;

        do {
            if (bytestream[current_byte_pos] == 0) {
                if (zcnt) zcnt++;
                else zcnt = 1;
            }
            else if (zcnt >= 2 && bytestream[current_byte_pos] == 1) break;
            current_byte_pos++;
        } while ((uint32_t*)&bytestream[current_byte_pos] < buffer_end);
        current_byte_pos -= 2;

        auto current_qword_pos = current_byte_pos / 8;
        buffer_ptr = (uint32_t*)&bytestream[current_qword_pos * 8];
        buffer_idx = (current_byte_pos - current_qword_pos * 8) * 8;

        buffer = bswap_64(*(uint64_t*)(buffer_ptr)) << buffer_idx;
        buffer_ptr += 2;
        cache  = bswap_64(*(uint64_t*)(buffer_ptr)); // precache next value
        if (buffer_idx)
            buffer |= cache >> (64 - buffer_idx);

        return false;
    }

    MP2V_INLINE void skip_bits(int len) {
        flush_bits(len);
    }

private:
    //FILE* bitstream = nullptr;
    std::vector<uint32_t, AlignmentAllocator<uint8_t, 32>> buffer_pool;
    ALIGN(16) uint32_t* buffer_ptr = nullptr;
    ALIGN(16) uint32_t* buffer_end = nullptr;
    ALIGN(16) uint64_t  buffer = 0;
    ALIGN(16) uint64_t  cache = 0;
    ALIGN(16) uint32_t  buffer_idx = 64;
};