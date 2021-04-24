// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <gtest/gtest.h>

// Tiny MPEG2 headers
#include "bitstream.h"

// unit test common
#include "cavlc_utils.hpp"

#define PRINTF(...)  \
    do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); \
    testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); } while(0)

#define ARRAY_SIZE(p) (sizeof(p)/sizeof(*p))

constexpr int TEST_NUM_ITERATIONS = 100;

typedef uint8_t(get_macroblock_type_func)(bitstream_reader_i*, int);

static bool test_vlc(random_vlc_code_bitstream_generator_c &gen_vlc, vlc_t vlc, int MBA) {
    bool test = true;
    for (int i = 0; i < TEST_NUM_ITERATIONS; i++) {
        gen_vlc.generate_vlc_code(vlc);
        int parsed_MBA = get_macroblock_address_increment(&gen_vlc);
        if (parsed_MBA != MBA) {
            PRINTF("VLC parse failed: parsed MBA value =  %d, expected %d\n", parsed_MBA, MBA);
            test = false;
        }
    }
    return test;
}

static bool test_macroblock_type_vlc(random_vlc_code_bitstream_generator_c& gen_vlc, macroblock_type_vlc_t vlc, get_macroblock_type_func func, int picture_coding_type) {
    bool test = true;
    for (int i = 0; i < TEST_NUM_ITERATIONS; i++) {
        gen_vlc.generate_vlc_code(vlc);
        auto parsed_value = func(&gen_vlc, picture_coding_type);
        if (parsed_value != vlc.value) {
            PRINTF("VLC parse failed: parsed macroblock_type value =  %d, expected %d\n", parsed_value, vlc.value);
            test = false;
        }
    }
    return test;
}

TEST(cavlc_test, macroblock_address_increment) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    int macroblock_address_increment = 0;
    for (int MBA = 1; MBA < ARRAY_SIZE(macroblock_address_increment_to_vlc)-1; MBA++) {
        EXPECT_TRUE(test_vlc(gen_vlc, macroblock_address_increment_to_vlc[MBA], MBA));
    }
}

TEST(cavlc_test, get_macroblock_type) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    for (auto vlc : i_macroblock_type)
        EXPECT_TRUE(test_macroblock_type_vlc(gen_vlc, vlc, get_macroblock_type, 1)) << "I frame error";
    for (auto vlc : p_macroblock_type)
        EXPECT_TRUE(test_macroblock_type_vlc(gen_vlc, vlc, get_macroblock_type, 2)) << "P frame error";
    for (auto vlc : b_macroblock_type)
        EXPECT_TRUE(test_macroblock_type_vlc(gen_vlc, vlc, get_macroblock_type, 3)) << "B frame error";
}

TEST(cavlc_test, get_spatial_scalability_macroblock_type) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    for (auto vlc: ss_i_macroblock_type)
        EXPECT_TRUE(test_macroblock_type_vlc(gen_vlc, vlc, get_spatial_scalability_macroblock_type, 1)) << "I frame error";
    for (auto vlc : ss_p_macroblock_type)
        EXPECT_TRUE(test_macroblock_type_vlc(gen_vlc, vlc, get_spatial_scalability_macroblock_type, 2)) << "P frame error";
    for (auto vlc : ss_b_macroblock_type)
        EXPECT_TRUE(test_macroblock_type_vlc(gen_vlc, vlc, get_spatial_scalability_macroblock_type, 3)) << "B frame error";
}