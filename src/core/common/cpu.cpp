// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include "cpu.h"
#include <intrin.h>

#if defined(_MSC_VER)
uint32_t bit_scan_reverse(uint32_t x)
{
    unsigned long index;
    _BitScanReverse(&index, x);
    return x ? 31 - index : 32;
}

uint32_t bit_scan_forward(uint32_t x)
{
    unsigned long index;
    _BitScanForward(&index, x);
    return x ? index : 32;
}

uint64_t bit_scan_forward64(uint64_t x)
{
    unsigned long index;
    _BitScanForward64(&index, x);
    return x ? index : 64;
}
#elif defined(__GNUC__) || defined(__clang__)
uint32_t bit_scan_reverse(uint32_t x)
{
    return x ? __builtin_clz(x) : 32;
}

uint32_t bit_scan_forward(uint32_t x)
{
    return x ? __builtin_ctz(x) : 32;
}

uint64_t bit_scan_forward64(uint64_t x)
{
    return x ? __builtin_ctzll(x) : 64;
}
#endif