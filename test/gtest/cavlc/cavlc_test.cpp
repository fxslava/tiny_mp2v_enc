// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <gtest/gtest.h>

// Tiny MPEG2 headers
#include "bitstream.h"
#include "core/mp2v_hdr.h"

// unit test common
#include "cavlc_utils.hpp"

#define PRINTF(...)  \
    do { testing::internal::ColoredPrintf(testing::internal::COLOR_GREEN, "[          ] "); \
    testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); } while(0)

#define ERROR_PRINTF(...)  \
    do { testing::internal::ColoredPrintf(testing::internal::COLOR_RED, "[  FAILED  ] "); \
    testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); } while(0)

#define ARRAY_SIZE(p) (sizeof(p)/sizeof(*p))

constexpr int TEST_NUM_ITERATIONS = 10;

typedef uint8_t(get_macroblock_type_func)(bitstream_reader_i*, int);

static bool test_vlc(random_vlc_code_bitstream_generator_c &gen_vlc, vlc_t vlc, int MBA) {
    bool test = true;
    for (int i = 0; i < TEST_NUM_ITERATIONS; i++) {
        gen_vlc.generate_vlc_code(vlc);
        int parsed_MBA = get_macroblock_address_increment(&gen_vlc);
        if (parsed_MBA != MBA) {
            ERROR_PRINTF("VLC parse failed: parsed MBA value =  %d, expected %d\n", parsed_MBA, MBA);
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
            ERROR_PRINTF("VLC parse failed: parsed macroblock_type value =  %d, expected %d\n", parsed_value, vlc.value);
            test = false;
        }
    }
    return test;
}

#define TEST_MACROBLOCK_TYPE(arr, fn, picture_coding_type) \
TEST(cavlc_test, fn##_##picture_coding_type) { \
    random_vlc_code_bitstream_generator_c gen_vlc; \
    for (auto vlc : arr) \
        EXPECT_TRUE(test_macroblock_type_vlc(gen_vlc, vlc, fn, picture_coding_type)); \
}

TEST(cavlc_test, macroblock_address_increment) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    int macroblock_address_increment = 0;
    for (int MBA = 1; MBA < ARRAY_SIZE(macroblock_address_increment_to_vlc) - 1; MBA++) {
        EXPECT_TRUE(test_vlc(gen_vlc, macroblock_address_increment_to_vlc[MBA], MBA));
    }
}

TEST_MACROBLOCK_TYPE(i_macroblock_type, get_macroblock_type, picture_coding_type_intra);
TEST_MACROBLOCK_TYPE(p_macroblock_type, get_macroblock_type, picture_coding_type_pred );
TEST_MACROBLOCK_TYPE(b_macroblock_type, get_macroblock_type, picture_coding_type_bidir);
TEST_MACROBLOCK_TYPE(ss_i_macroblock_type, get_spatial_scalability_macroblock_type, picture_coding_type_intra);
TEST_MACROBLOCK_TYPE(ss_p_macroblock_type, get_spatial_scalability_macroblock_type, picture_coding_type_pred);
TEST_MACROBLOCK_TYPE(ss_b_macroblock_type, get_spatial_scalability_macroblock_type, picture_coding_type_bidir);
TEST_MACROBLOCK_TYPE(snr_macroblock_type, get_snr_scalability_macroblock_type, picture_coding_type_intra);