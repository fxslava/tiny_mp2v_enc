// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#pragma once
#include "core/mp2v_vlc.h"
#include <random>

class random_vlc_code_bitstream_generator_c {
private:
    void generate_vlc_code(uint32_t value, uint32_t len) {
        fullness = len;
        uint32_t offset = 32 - len;
        uint64_t max_val = (1ll << offset) - 1;
        std::uniform_int_distribution<uint32_t> distr_low(0, (uint32_t)max_val);
        std::uniform_int_distribution<uint32_t> distr_high(0, 0xffffffff);
        buffer = ((uint64_t)(distr_low(gen) & max_val) << 32) | ((uint64_t)distr_high(gen) & 0xffffffff);
        buffer |= ((uint64_t)value << (offset + 32));
    }

public:
    random_vlc_code_bitstream_generator_c(uint32_t seed = 1729) : gen(seed) {}

    void generate_vlc_code(vlc_t vlc_code)                 { generate_vlc_code(vlc_code.value, vlc_code.len); }
    int  get_fullness() { return fullness; }

    uint32_t get_next_bits(int len) {
        uint64_t mask = (1ll << len) - 1;
        return (buffer >> (64 - len)) & mask;
    }

    uint32_t read_next_bits(int len) {
        fullness -= len;
        return get_next_bits(len);
    }

    bool seek_pattern(uint32_t pattern, int len) { return true; }
    void skip_bits(int len) { fullness -= len; }

private:
    uint64_t buffer = 0;
    uint32_t fullness = 0;
    std::mt19937 gen;
};