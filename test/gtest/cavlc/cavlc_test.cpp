// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <gtest/gtest.h>
#include <iostream>

// unit test common
#include "cavlc_utils.hpp"

// Tiny MPEG2 headers
#include "core/mp2v_hdr.h"
#include "core/mp2v_vlc_dec.hpp"

DEFINE_CAVLC_METHODS(random_vlc_code_bitstream_generator_c)

#define ERROR_PRINTF(...)  \
    do { testing::internal::ColoredPrintf(testing::internal::COLOR_RED, "[  FAILED  ] "); \
    testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, __VA_ARGS__); } while(0)

// C++ stream interface
class TestCout : public std::stringstream
{
public:
    ~TestCout() { ERROR_PRINTF("%s", str().c_str()); }
};

#define TEST_COUT  TestCout()

#define ARRAY_SIZE(p) (sizeof(p)/sizeof(*p))

constexpr int TEST_NUM_ITERATIONS = 100;

bool operator!=(coeff_t val0, coeff_t val1) {
    return (val0.level != val1.level) || (val0.run != val1.run);
}

std::ostream& operator<<(std::ostream& os, const coeff_t val)
{
    os << "{" << (int)val.run << ", " << (int)val.level << "} ";
    return os;
}

template<typename T>
static bool test_vlc(random_vlc_code_bitstream_generator_c& gen_vlc, vlc_t vlc, T val, T(func)(random_vlc_code_bitstream_generator_c*)) {
    bool test = true;
    for (int i = 0; i < TEST_NUM_ITERATIONS; i++) {
        gen_vlc.generate_vlc_code(vlc);
        T parsed_vlc = func(&gen_vlc);
        if (parsed_vlc != val) {
            TEST_COUT << "VLC parse failed: parsed VLC value =  " << parsed_vlc << "expected " << val << std::endl;
            test = false;
        }
        if (gen_vlc.get_fullness() != 0) {
            TEST_COUT << "VLC length parse failed: parsed VLC value =  " << parsed_vlc << std::endl;
            test = false;
        }
    }
    return test;
}

#define TEST_MACROBLOCK_TYPE(arr, fn) \
TEST(cavlc_test, fn##_##picture_coding_type) { \
    random_vlc_code_bitstream_generator_c gen_vlc; \
    for (auto mb_type : arr) \
        EXPECT_TRUE(test_vlc(gen_vlc, mb_type.vlc, mb_type.value, fn)); \
}

#define TEST_COEFFICIENTS(arr, fn) \
TEST(cavlc_test, fn) { \
    random_vlc_code_bitstream_generator_c gen_vlc; \
    for (auto coeff : arr) { \
        EXPECT_TRUE(test_vlc(gen_vlc, coeff.vlc, coeff.coeff, fn)); } \
}

#define TEST_GET_VLC_FUNC(arr, offset, fn, array_size, start_idx) \
TEST(cavlc_test, fn) { \
    random_vlc_code_bitstream_generator_c gen_vlc; \
    for (int val = start_idx; val < array_size; val++) { \
        EXPECT_TRUE(test_vlc(gen_vlc, arr[val], val + offset, fn)); } \
}

// wrappers
uint8_t get_i_macroblock_type(random_vlc_code_bitstream_generator_c* bs) { return get_macroblock_type(bs, picture_coding_type_intra); }
uint8_t get_p_macroblock_type(random_vlc_code_bitstream_generator_c* bs) { return get_macroblock_type(bs, picture_coding_type_pred); }
uint8_t get_b_macroblock_type(random_vlc_code_bitstream_generator_c* bs) { return get_macroblock_type(bs, picture_coding_type_bidir); }
uint8_t get_i_spatial_scalability_macroblock_type(random_vlc_code_bitstream_generator_c* bs) { return get_spatial_scalability_macroblock_type(bs, picture_coding_type_intra); }
uint8_t get_p_spatial_scalability_macroblock_type(random_vlc_code_bitstream_generator_c* bs) { return get_spatial_scalability_macroblock_type(bs, picture_coding_type_pred); }
uint8_t get_b_spatial_scalability_macroblock_type(random_vlc_code_bitstream_generator_c* bs) { return get_spatial_scalability_macroblock_type(bs, picture_coding_type_bidir); }

/* Conformance tests */
TEST_GET_VLC_FUNC(macroblock_address_increment_to_vlc, 0, get_macroblock_address_increment, ARRAY_SIZE(macroblock_address_increment_to_vlc) - 1, 1);
TEST_GET_VLC_FUNC(macroblock_address_increment_to_vlc, 0, get_macroblock_address_increment_lut, ARRAY_SIZE(macroblock_address_increment_to_vlc) - 1, 1);
TEST_MACROBLOCK_TYPE(i_macroblock_type, get_i_macroblock_type);
TEST_MACROBLOCK_TYPE(p_macroblock_type, get_p_macroblock_type);
TEST_MACROBLOCK_TYPE(b_macroblock_type, get_b_macroblock_type);
TEST_MACROBLOCK_TYPE(ss_i_macroblock_type, get_i_spatial_scalability_macroblock_type);
TEST_MACROBLOCK_TYPE(ss_p_macroblock_type, get_p_spatial_scalability_macroblock_type);
TEST_MACROBLOCK_TYPE(ss_b_macroblock_type, get_b_spatial_scalability_macroblock_type);
TEST_MACROBLOCK_TYPE(snr_macroblock_type, get_snr_scalability_macroblock_type);
TEST_GET_VLC_FUNC(coded_block_pattern_to_vlc, 0, get_coded_block_pattern, ARRAY_SIZE(coded_block_pattern_to_vlc), 0);
TEST_GET_VLC_FUNC(motion_code_to_vlc, -16, get_motion_code, ARRAY_SIZE(motion_code_to_vlc), 0);
TEST_GET_VLC_FUNC(dmvector_to_vlc, -1, get_dmvector, ARRAY_SIZE(dmvector_to_vlc), 0);
TEST_GET_VLC_FUNC(dct_size_luminance_to_vlc, 0, get_dct_size_luminance, ARRAY_SIZE(dct_size_luminance_to_vlc), 0);
TEST_GET_VLC_FUNC(dct_size_chrominance_to_vlc, 0, get_dct_size_chrominance, ARRAY_SIZE(dct_size_chrominance_to_vlc), 0);
TEST_COEFFICIENTS(coeff_zero_vlc, get_coeff_zero)
TEST_COEFFICIENTS(coeff_one_vlc, get_coeff_one)