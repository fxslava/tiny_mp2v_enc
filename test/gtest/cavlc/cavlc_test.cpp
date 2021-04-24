// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <gtest/gtest.h>
#include <iostream>

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

// C++ stream interface
class TestCout : public std::stringstream
{
public:
    ~TestCout()
    {
        PRINTF("%s", str().c_str());
    }
};

#define TEST_COUT  TestCout()

#define ARRAY_SIZE(p) (sizeof(p)/sizeof(*p))

constexpr int TEST_NUM_ITERATIONS = 10;
constexpr int PERF_TEST_NUM_ITERATIONS = 1000000;

bool operator!=(coeff_t val0, coeff_t val1) {
    return (val0.level != val1.level) || (val0.run != val1.run);
}

std::ostream& operator<<(std::ostream& os, const coeff_t val)
{
    os << "{" << (int)val.run << ", " << (int)val.level << "} ";
    return os;
}

typedef int32_t(get_vlc_val_func)(bitstream_reader_i*);
static bool perf_test_vlc(random_vlc_code_bitstream_generator_c & gen_vlc, vlc_t vlc, int val, get_vlc_val_func func) {
    bool test = true;
    for (int i = 0; i < PERF_TEST_NUM_ITERATIONS; i++) {
        gen_vlc.generate_vlc_code(vlc);
        int parsed_vlc = func(&gen_vlc);
        if (parsed_vlc != val)
            test = false;
    }
    return test;
}

template<typename T>
static bool template_test_vlc(random_vlc_code_bitstream_generator_c& gen_vlc, vlc_t vlc, T val, T(func)(bitstream_reader_i*)) {
    bool test = true;
    for (int i = 0; i < TEST_NUM_ITERATIONS; i++) {
        gen_vlc.generate_vlc_code(vlc);
        T parsed_vlc = func(&gen_vlc);
        if (parsed_vlc != val) {
            TEST_COUT << "VLC parse failed: parsed VLC value =  " << parsed_vlc << "expected " << val << std::endl;
            test = false;
        }
    }
    return test;
}

typedef int32_t(get_vlc_val_func)(bitstream_reader_i*);
static bool test_vlc(random_vlc_code_bitstream_generator_c &gen_vlc, vlc_t vlc, int val, get_vlc_val_func func) {
    bool test = true;
    for (int i = 0; i < TEST_NUM_ITERATIONS; i++) {
        gen_vlc.generate_vlc_code(vlc);
        int parsed_vlc = func(&gen_vlc);
        if (parsed_vlc != val) {
            ERROR_PRINTF("VLC parse failed: parsed VLC value =  %d, expected %d\n", parsed_vlc, val);
            test = false;
        }
    }
    return test;
}

typedef uint8_t(get_macroblock_type_func)(bitstream_reader_i*, int);
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

#define TEST_GET_VLC_FUNC(arr, offset, fn) \
TEST(cavlc_test, fn) { \
    random_vlc_code_bitstream_generator_c gen_vlc; \
    for (int val = 0; val < ARRAY_SIZE(arr); val++) { \
        EXPECT_TRUE(test_vlc(gen_vlc, arr[val], val + offset, fn)); } \
}

TEST(cavlc_test, get_macroblock_address_increment) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    for (int MBA = 1; MBA < ARRAY_SIZE(macroblock_address_increment_to_vlc) - 1; MBA++) {
        EXPECT_TRUE(test_vlc(gen_vlc, macroblock_address_increment_to_vlc[MBA], MBA, get_macroblock_address_increment));
    }
}

TEST(cavlc_test, get_macroblock_address_increment_lut) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    for (int MBA = 1; MBA < ARRAY_SIZE(macroblock_address_increment_to_vlc) - 1; MBA++) {
        EXPECT_TRUE(test_vlc(gen_vlc, macroblock_address_increment_to_vlc[MBA], MBA, get_macroblock_address_increment_lut));
    }
}

/* Conformance tests */
TEST_MACROBLOCK_TYPE(i_macroblock_type, get_macroblock_type, picture_coding_type_intra);
TEST_MACROBLOCK_TYPE(p_macroblock_type, get_macroblock_type, picture_coding_type_pred );
TEST_MACROBLOCK_TYPE(b_macroblock_type, get_macroblock_type, picture_coding_type_bidir);
TEST_MACROBLOCK_TYPE(ss_i_macroblock_type, get_spatial_scalability_macroblock_type, picture_coding_type_intra);
TEST_MACROBLOCK_TYPE(ss_p_macroblock_type, get_spatial_scalability_macroblock_type, picture_coding_type_pred);
TEST_MACROBLOCK_TYPE(ss_b_macroblock_type, get_spatial_scalability_macroblock_type, picture_coding_type_bidir);
TEST_MACROBLOCK_TYPE(snr_macroblock_type, get_snr_scalability_macroblock_type, picture_coding_type_intra);
TEST_GET_VLC_FUNC(coded_block_pattern_to_vlc, 0, get_coded_block_pattern);
TEST_GET_VLC_FUNC(motion_code_to_vlc, -16, get_motion_code);
TEST_GET_VLC_FUNC(dmvector_to_vlc, -1, get_dmvector);
TEST_GET_VLC_FUNC(dct_size_luminance_to_vlc, 0, get_dct_size_luminance);
TEST_GET_VLC_FUNC(dct_size_chrominance_to_vlc, 0, get_dct_size_chrominance);

TEST(cavlc_test, get_coeff_zero) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    for (auto coeff : coeff_zero_vlc) {
        EXPECT_TRUE(template_test_vlc(gen_vlc, coeff.vlc, coeff.coeff, get_coeff_zero));
    }
}

/* Perfomance tests */
TEST(cavlc_performance_test, get_macroblock_address_increment) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    perf_test_vlc(gen_vlc, macroblock_address_increment_to_vlc[0], 0, get_macroblock_address_increment);
}

TEST(cavlc_performance_test, get_macroblock_address_increment_lut) {
    random_vlc_code_bitstream_generator_c gen_vlc;
    perf_test_vlc(gen_vlc, macroblock_address_increment_to_vlc[0], 0, get_macroblock_address_increment_lut);
}