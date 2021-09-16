// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#include <vector>
#include <random>
#include <algorithm>
#include <iterator>
#include <chrono>

// unit test common
#include "test_common.h"

// Tiny MPEG2 headers
#include "core/mc.h"

constexpr int TEST_NUM_ITERATIONS = 10;
constexpr int TEST_NUM_ITERATIONS_PERFORMANCE = 10000;
constexpr int MC_PLANE_SIZE = 16;
constexpr int MC_PLANE_STRIDE = 32;
constexpr int MC_PIXEL_MAX_VALUE = 255;
constexpr int MC_RANDOM_SEED = 1729;

#define OJECT_NAME(obj) std::string(#obj)

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

    GTEST_NO_INLINE_ void call_mc_routine(uint8_t* dst, mc_pred_func_t func) {
        func(dst, &src_plane_L0[0], MC_PLANE_STRIDE, MC_PLANE_SIZE);
    }

    GTEST_NO_INLINE_ void call_mc_routine(uint8_t* dst, mc_bidir_func_t func) {
        func(dst, &src_plane_L0[0], &src_plane_L1[0], MC_PLANE_STRIDE, MC_PLANE_SIZE);
    }

    void generate_sources(bool generate_L1) {
        std::uniform_int_distribution<uint32_t> uniform_gen(0, MC_PIXEL_MAX_VALUE);
        auto rand_gen = [&uniform_gen, this]() {
            return uniform_gen(gen);
        };
        std::generate(src_plane_L0.begin(), src_plane_L0.end(), rand_gen);
        if (generate_L1)
            std::generate(src_plane_L1.begin(), src_plane_L1.end(), rand_gen);
    }

    template<typename func_t>
    bool test_mc_pred(func_t func_c, func_t func_simd, char* name_func_c, char* name_func_simd) {
        for (int step = 0; step < TEST_NUM_ITERATIONS; step++) {
            generate_sources(std::is_same<func_t, mc_bidir_func_t>::value);
            call_mc_routine(&dst_plane_ref[0], func_c);
            call_mc_routine(&dst_plane[0], func_simd);
            if (dst_plane != dst_plane_ref)
                return false;
        }
        return true;
    }

    template<typename func_t>
    bool test_mc_pred_performance(func_t func_c, func_t func_simd, char* name_func_c, char* name_func_simd) {
        const auto start = std::chrono::system_clock::now();
        for (int step = 0; step < TEST_NUM_ITERATIONS_PERFORMANCE; step++)
            call_mc_routine(&dst_plane_ref[0], func_c);
        const auto middle = std::chrono::system_clock::now();
        for (int step = 0; step < TEST_NUM_ITERATIONS_PERFORMANCE; step++)
            call_mc_routine(&dst_plane[0], func_simd);
        const auto end = std::chrono::system_clock::now();
        auto elapsed_func_c_ms = std::chrono::duration_cast<std::chrono::microseconds>(middle - start).count();
        auto elapsed_func_simd_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - middle).count();
        float perf_inc = (float)(elapsed_func_c_ms - elapsed_func_simd_ms) * 100.0f / (float)(elapsed_func_simd_ms);

        auto color = perf_inc > 50.0f ? testing::internal::COLOR_GREEN : (perf_inc > 25.0f ? testing::internal::COLOR_YELLOW : testing::internal::COLOR_RED);
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xda\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xbf\n");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xb3 %24s \xb3 %24s \xb3 %23s vs %23s \xb3\n", name_func_c, name_func_simd, name_func_c, name_func_simd);
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xb3\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc5\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc5\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xb3\n");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xb3"); 
        testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, " %24d ", elapsed_func_c_ms);
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xb3");
        testing::internal::ColoredPrintf(testing::internal::COLOR_YELLOW, " %24d ", elapsed_func_simd_ms);
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xb3");
        testing::internal::ColoredPrintf(color, " %50.2f ", perf_inc);
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xb3\n");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc0\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4");
        testing::internal::ColoredPrintf(testing::internal::COLOR_DEFAULT, "\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xc4\xd9\n");
        
        return perf_inc > 25.0f;
    }

protected:
    std::vector<uint8_t> src_plane_L0; // unaligned
    std::vector<uint8_t> src_plane_L1; // unaligned
    std::vector<uint8_t, AlignmentAllocator<uint8_t, 32>> dst_plane;
    std::vector<uint8_t, AlignmentAllocator<uint8_t, 32>> dst_plane_ref;
    std::mt19937 gen{};
};

#define TEST_MC_ROUTINES(test_case, test_func, plane_c, simd) \
TEST_F(simd_mc_test_c, test_case##_pred00_16xh_##simd)    { EXPECT_TRUE(test_func(mc_pred_16xh_##plane_c [ 0], mc_pred_16xh_##simd [ 0], "mc_pred00_16xh_" #plane_c   , "mc_pred00_16xh_" #simd   )); } \
TEST_F(simd_mc_test_c, test_case##_pred01_16xh_##simd)    { EXPECT_TRUE(test_func(mc_pred_16xh_##plane_c [ 1], mc_pred_16xh_##simd [ 1], "mc_pred01_16xh_" #plane_c   , "mc_pred01_16xh_##simd"   )); } \
TEST_F(simd_mc_test_c, test_case##_pred10_16xh_##simd)    { EXPECT_TRUE(test_func(mc_pred_16xh_##plane_c [ 2], mc_pred_16xh_##simd [ 2], "mc_pred10_16xh_" #plane_c   , "mc_pred10_16xh_##simd"   )); } \
TEST_F(simd_mc_test_c, test_case##_pred11_16xh_##simd)    { EXPECT_TRUE(test_func(mc_pred_16xh_##plane_c [ 3], mc_pred_16xh_##simd [ 3], "mc_pred11_16xh_" #plane_c   , "mc_pred11_16xh_##simd"   )); } \
TEST_F(simd_mc_test_c, test_case##_pred00_8xh_##simd)     { EXPECT_TRUE(test_func(mc_pred_8xh_##plane_c  [ 0], mc_pred_8xh_##simd  [ 0], "mc_pred00_8xh_" #plane_c    , "mc_pred00_8xh_##simd"    )); } \
TEST_F(simd_mc_test_c, test_case##_pred01_8xh_##simd)     { EXPECT_TRUE(test_func(mc_pred_8xh_##plane_c  [ 1], mc_pred_8xh_##simd  [ 1], "mc_pred01_8xh_" #plane_c    , "mc_pred01_8xh_##simd"    )); } \
TEST_F(simd_mc_test_c, test_case##_pred10_8xh_##simd)     { EXPECT_TRUE(test_func(mc_pred_8xh_##plane_c  [ 2], mc_pred_8xh_##simd  [ 2], "mc_pred10_8xh_" #plane_c    , "mc_pred10_8xh_##simd"    )); } \
TEST_F(simd_mc_test_c, test_case##_pred11_8xh_##simd)     { EXPECT_TRUE(test_func(mc_pred_8xh_##plane_c  [ 3], mc_pred_8xh_##simd  [ 3], "mc_pred11_8xh_" #plane_c    , "mc_pred11_8xh_##simd"    )); } \
TEST_F(simd_mc_test_c, test_case##_bidir0000_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 0], mc_bidir_16xh_##simd[ 0], "mc_bidir0000_16xh_" #plane_c, "mc_bidir0000_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir0001_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 1], mc_bidir_16xh_##simd[ 1], "mc_bidir0001_16xh_" #plane_c, "mc_bidir0001_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir0010_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 2], mc_bidir_16xh_##simd[ 2], "mc_bidir0010_16xh_" #plane_c, "mc_bidir0010_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir0011_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 3], mc_bidir_16xh_##simd[ 3], "mc_bidir0011_16xh_" #plane_c, "mc_bidir0011_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir0100_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 4], mc_bidir_16xh_##simd[ 4], "mc_bidir0100_16xh_" #plane_c, "mc_bidir0100_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir0101_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 5], mc_bidir_16xh_##simd[ 5], "mc_bidir0101_16xh_" #plane_c, "mc_bidir0101_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir0110_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 6], mc_bidir_16xh_##simd[ 6], "mc_bidir0110_16xh_" #plane_c, "mc_bidir0110_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir0111_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 7], mc_bidir_16xh_##simd[ 7], "mc_bidir0111_16xh_" #plane_c, "mc_bidir0111_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir1000_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 8], mc_bidir_16xh_##simd[ 8], "mc_bidir1000_16xh_" #plane_c, "mc_bidir1000_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir1001_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[ 9], mc_bidir_16xh_##simd[ 9], "mc_bidir1001_16xh_" #plane_c, "mc_bidir1001_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir1010_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[10], mc_bidir_16xh_##simd[10], "mc_bidir1010_16xh_" #plane_c, "mc_bidir1010_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir1011_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[11], mc_bidir_16xh_##simd[11], "mc_bidir1011_16xh_" #plane_c, "mc_bidir1011_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir1100_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[12], mc_bidir_16xh_##simd[12], "mc_bidir1100_16xh_" #plane_c, "mc_bidir1100_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir1101_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[13], mc_bidir_16xh_##simd[13], "mc_bidir1101_16xh_" #plane_c, "mc_bidir1101_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir1110_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[14], mc_bidir_16xh_##simd[14], "mc_bidir1110_16xh_" #plane_c, "mc_bidir1110_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir1111_16xh_##simd) { EXPECT_TRUE(test_func(mc_bidir_16xh_##plane_c[15], mc_bidir_16xh_##simd[15], "mc_bidir1111_16xh_" #plane_c, "mc_bidir1111_16xh_" #simd)); } \
TEST_F(simd_mc_test_c, test_case##_bidir0000_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 0], mc_bidir_8xh_##simd [ 0], "mc_bidir0000_8xh_" #plane_c , "mc_bidir0000_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir0001_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 1], mc_bidir_8xh_##simd [ 1], "mc_bidir0001_8xh_" #plane_c , "mc_bidir0001_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir0010_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 2], mc_bidir_8xh_##simd [ 2], "mc_bidir0010_8xh_" #plane_c , "mc_bidir0010_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir0011_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 3], mc_bidir_8xh_##simd [ 3], "mc_bidir0011_8xh_" #plane_c , "mc_bidir0011_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir0100_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 4], mc_bidir_8xh_##simd [ 4], "mc_bidir0100_8xh_" #plane_c , "mc_bidir0100_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir0101_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 5], mc_bidir_8xh_##simd [ 5], "mc_bidir0101_8xh_" #plane_c , "mc_bidir0101_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir0110_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 6], mc_bidir_8xh_##simd [ 6], "mc_bidir0110_8xh_" #plane_c , "mc_bidir0110_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir0111_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 7], mc_bidir_8xh_##simd [ 7], "mc_bidir0111_8xh_" #plane_c , "mc_bidir0111_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir1000_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 8], mc_bidir_8xh_##simd [ 8], "mc_bidir1000_8xh_" #plane_c , "mc_bidir1000_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir1001_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [ 9], mc_bidir_8xh_##simd [ 9], "mc_bidir1001_8xh_" #plane_c , "mc_bidir1001_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir1010_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [10], mc_bidir_8xh_##simd [10], "mc_bidir1010_8xh_" #plane_c , "mc_bidir1010_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir1011_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [11], mc_bidir_8xh_##simd [11], "mc_bidir1011_8xh_" #plane_c , "mc_bidir1011_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir1100_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [12], mc_bidir_8xh_##simd [12], "mc_bidir1100_8xh_" #plane_c , "mc_bidir1100_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir1101_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [13], mc_bidir_8xh_##simd [13], "mc_bidir1101_8xh_" #plane_c , "mc_bidir1101_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir1110_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [14], mc_bidir_8xh_##simd [14], "mc_bidir1110_8xh_" #plane_c , "mc_bidir1110_8xh_" #simd )); } \
TEST_F(simd_mc_test_c, test_case##_bidir1111_8xh_##simd)  { EXPECT_TRUE(test_func(mc_bidir_8xh_##plane_c [15], mc_bidir_8xh_##simd [15], "mc_bidir1111_8xh_" #plane_c , "mc_bidir1111_8xh_" #simd )); }

#if (defined(__GNUC__) && defined(__x86_64)) || (defined(_MSC_VER) && defined(_M_X64))
TEST_MC_ROUTINES(validation, test_mc_pred, c, sse2)
TEST_MC_ROUTINES(performance, test_mc_pred_performance, nsse2, sse2)
#endif
