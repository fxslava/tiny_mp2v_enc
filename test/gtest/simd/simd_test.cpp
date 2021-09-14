// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>

// unit test common
#include "test_common.h"

// Tiny MPEG2 headers
#include "core/mc.h"

constexpr int TEST_NUM_ITERATIONS = 100;
constexpr int MC_PLANE_SIZE = 16;
constexpr int MC_PLANE_STRIDE = 32;
constexpr int MC_PIXEL_MAX_VALUE = 255;
constexpr int MC_RANDOM_SEED = 1729;

class simd_mc_test_c : public ::testing::Test {
public:
    simd_mc_test_c() : gen(MC_RANDOM_SEED) {
        src_plane_L0.resize((MC_PLANE_SIZE + 1) * MC_PLANE_STRIDE);
        src_plane_L1.resize((MC_PLANE_SIZE + 1) * MC_PLANE_STRIDE);
        dst_plane.resize(MC_PLANE_SIZE * MC_PLANE_STRIDE);
        dst_plane_ref.resize(MC_PLANE_SIZE * MC_PLANE_STRIDE);
    }
    ~simd_mc_test_c() {}
    void SetUp() {
        std::fill(dst_plane.begin(), dst_plane.end(), 0);
        std::fill(dst_plane_ref.begin(), dst_plane_ref.end(), 0);
    }
    void TearDown() {}

    bool test_mc_routine(mc_pred_func_t func_c, mc_pred_func_t func_simd) {
        func_c(&dst_plane[0], &src_plane_L0[0], MC_PLANE_STRIDE, MC_PLANE_SIZE);
        func_simd(&dst_plane_ref[0], &src_plane_L0[0], MC_PLANE_STRIDE, MC_PLANE_SIZE);
        return dst_plane == dst_plane_ref;
    }

    bool test_mc_routine(mc_bidir_func_t func_c, mc_bidir_func_t func_simd) {
        func_c(&dst_plane[0], &src_plane_L0[0], &src_plane_L1[0], MC_PLANE_STRIDE, MC_PLANE_SIZE);
        func_simd(&dst_plane_ref[0], &src_plane_L0[0], &src_plane_L1[0], MC_PLANE_STRIDE, MC_PLANE_SIZE);
        return dst_plane == dst_plane_ref;
    }

    template<typename func_t>
    bool test_mc_pred(func_t func_c, func_t func_simd) {
        std::uniform_int_distribution<uint32_t> uniform_gen(0, MC_PIXEL_MAX_VALUE);
        auto rand_gen = [&uniform_gen, this]() {
            return uniform_gen(gen);
        };
        for (int step = 0; step < TEST_NUM_ITERATIONS; step++) {
            std::generate(src_plane_L0.begin(), src_plane_L0.end(), rand_gen);
            if (std::is_same<func_t, mc_bidir_func_t>::value)
                std::generate(src_plane_L1.begin(), src_plane_L1.end(), rand_gen);
            if (!test_mc_routine(func_c, func_simd))
                return false;
        }
        return true;
    }

protected:
    std::vector<uint8_t> src_plane_L0; // unaligned
    std::vector<uint8_t> src_plane_L1; // unaligned
    std::vector<uint8_t, AlignmentAllocator<uint8_t, 32>> dst_plane;
    std::vector<uint8_t, AlignmentAllocator<uint8_t, 32>> dst_plane_ref;
    std::mt19937 gen{};
};

TEST_F(simd_mc_test_c, mc_pred00_16xh_nsse2)    { EXPECT_TRUE(test_mc_pred(mc_pred_16xh [ 0], mc_pred_16xh_nsse2 [ 0])); }
TEST_F(simd_mc_test_c, mc_pred01_16xh_nsse2)    { EXPECT_TRUE(test_mc_pred(mc_pred_16xh [ 1], mc_pred_16xh_nsse2 [ 1])); }
TEST_F(simd_mc_test_c, mc_pred10_16xh_nsse2)    { EXPECT_TRUE(test_mc_pred(mc_pred_16xh [ 2], mc_pred_16xh_nsse2 [ 2])); }
TEST_F(simd_mc_test_c, mc_pred11_16xh_nsse2)    { EXPECT_TRUE(test_mc_pred(mc_pred_16xh [ 3], mc_pred_16xh_nsse2 [ 3])); }
TEST_F(simd_mc_test_c, mc_pred00_8xh_nsse2)     { EXPECT_TRUE(test_mc_pred(mc_pred_8xh  [ 0], mc_pred_8xh_nsse2  [ 0])); }
TEST_F(simd_mc_test_c, mc_pred01_8xh_nsse2)     { EXPECT_TRUE(test_mc_pred(mc_pred_8xh  [ 1], mc_pred_8xh_nsse2  [ 1])); }
TEST_F(simd_mc_test_c, mc_pred10_8xh_nsse2)     { EXPECT_TRUE(test_mc_pred(mc_pred_8xh  [ 2], mc_pred_8xh_nsse2  [ 2])); }
TEST_F(simd_mc_test_c, mc_pred11_8xh_nsse2)     { EXPECT_TRUE(test_mc_pred(mc_pred_8xh  [ 3], mc_pred_8xh_nsse2  [ 3])); }
TEST_F(simd_mc_test_c, mc_bidir0000_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 0], mc_bidir_16xh_nsse2[ 0])); }
TEST_F(simd_mc_test_c, mc_bidir0001_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 1], mc_bidir_16xh_nsse2[ 1])); }
TEST_F(simd_mc_test_c, mc_bidir0010_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 2], mc_bidir_16xh_nsse2[ 2])); }
TEST_F(simd_mc_test_c, mc_bidir0011_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 3], mc_bidir_16xh_nsse2[ 3])); }
TEST_F(simd_mc_test_c, mc_bidir0100_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 4], mc_bidir_16xh_nsse2[ 4])); }
TEST_F(simd_mc_test_c, mc_bidir0101_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 5], mc_bidir_16xh_nsse2[ 5])); }
TEST_F(simd_mc_test_c, mc_bidir0110_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 6], mc_bidir_16xh_nsse2[ 6])); }
TEST_F(simd_mc_test_c, mc_bidir0111_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 7], mc_bidir_16xh_nsse2[ 7])); }
TEST_F(simd_mc_test_c, mc_bidir1000_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 8], mc_bidir_16xh_nsse2[ 8])); }
TEST_F(simd_mc_test_c, mc_bidir1001_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[ 9], mc_bidir_16xh_nsse2[ 9])); }
TEST_F(simd_mc_test_c, mc_bidir1010_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[10], mc_bidir_16xh_nsse2[10])); }
TEST_F(simd_mc_test_c, mc_bidir1011_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[11], mc_bidir_16xh_nsse2[11])); }
TEST_F(simd_mc_test_c, mc_bidir1100_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[12], mc_bidir_16xh_nsse2[12])); }
TEST_F(simd_mc_test_c, mc_bidir1101_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[13], mc_bidir_16xh_nsse2[13])); }
TEST_F(simd_mc_test_c, mc_bidir1110_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[14], mc_bidir_16xh_nsse2[14])); }
TEST_F(simd_mc_test_c, mc_bidir1111_16xh_nsse2) { EXPECT_TRUE(test_mc_pred(mc_bidir_16xh[15], mc_bidir_16xh_nsse2[15])); }
TEST_F(simd_mc_test_c, mc_bidir0000_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 0], mc_bidir_8xh_nsse2 [ 0])); }
TEST_F(simd_mc_test_c, mc_bidir0001_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 1], mc_bidir_8xh_nsse2 [ 1])); }
TEST_F(simd_mc_test_c, mc_bidir0010_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 2], mc_bidir_8xh_nsse2 [ 2])); }
TEST_F(simd_mc_test_c, mc_bidir0011_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 3], mc_bidir_8xh_nsse2 [ 3])); }
TEST_F(simd_mc_test_c, mc_bidir0100_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 4], mc_bidir_8xh_nsse2 [ 4])); }
TEST_F(simd_mc_test_c, mc_bidir0101_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 5], mc_bidir_8xh_nsse2 [ 5])); }
TEST_F(simd_mc_test_c, mc_bidir0110_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 6], mc_bidir_8xh_nsse2 [ 6])); }
TEST_F(simd_mc_test_c, mc_bidir0111_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 7], mc_bidir_8xh_nsse2 [ 7])); }
TEST_F(simd_mc_test_c, mc_bidir1000_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 8], mc_bidir_8xh_nsse2 [ 8])); }
TEST_F(simd_mc_test_c, mc_bidir1001_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [ 9], mc_bidir_8xh_nsse2 [ 9])); }
TEST_F(simd_mc_test_c, mc_bidir1010_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [10], mc_bidir_8xh_nsse2 [10])); }
TEST_F(simd_mc_test_c, mc_bidir1011_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [11], mc_bidir_8xh_nsse2 [11])); }
TEST_F(simd_mc_test_c, mc_bidir1100_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [12], mc_bidir_8xh_nsse2 [12])); }
TEST_F(simd_mc_test_c, mc_bidir1101_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [13], mc_bidir_8xh_nsse2 [13])); }
TEST_F(simd_mc_test_c, mc_bidir1110_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [14], mc_bidir_8xh_nsse2 [14])); }
TEST_F(simd_mc_test_c, mc_bidir1111_8xh_nsse2)  { EXPECT_TRUE(test_mc_pred(mc_bidir_8xh [15], mc_bidir_8xh_nsse2 [15])); }
