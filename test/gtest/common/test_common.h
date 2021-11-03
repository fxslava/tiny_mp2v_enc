// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#include <gtest/gtest.h>
#include <iostream>
#include <future>

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
