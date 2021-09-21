// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>

#if defined(__GNUC__) || defined(__clang__)
#define MP2V_INLINE                   inline __attribute__((always_inline))
#else
#define MP2V_INLINE                   __forceinline
#endif

#if defined(_MSC_VER)
#include <intrin.h>
#define bswap_16(x) _byteswap_ushort(x)
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)

MP2V_INLINE uint32_t bit_scan_reverse(uint32_t x)
{
    unsigned long index;
    _BitScanReverse(&index, x);
    return x ? 31 - index : 32;
}

MP2V_INLINE uint8_t bit_scan_reverse64(uint64_t x)
{
    unsigned long index;
    _BitScanReverse64(&index, x);
    return x ? 63 - index : 64;
}

MP2V_INLINE uint32_t bit_scan_forward(uint32_t x)
{
    unsigned long index;
    _BitScanForward(&index, x);
    return x ? index : 32;
}

MP2V_INLINE uint64_t bit_scan_forward64(uint64_t x)
{
    unsigned long index;
    _BitScanForward64(&index, x);
    return x ? index : 64;
}
#elif defined(__GNUC__) || defined(__clang__)
#define bswap_16(x) __builtin_bswap16(x);
#define bswap_32(x) __builtin_bswap32(x);
#define bswap_64(x) __builtin_bswap64(x);

MP2V_INLINE uint32_t bit_scan_reverse(uint32_t x)
{
    return x ? __builtin_clz(x) : 32;
}

MP2V_INLINE uint32_t bit_scan_forward(uint32_t x)
{
    return x ? __builtin_ctz(x) : 32;
}

MP2V_INLINE uint64_t bit_scan_forward64(uint64_t x)
{
    return x ? __builtin_ctzll(x) : 64;
}
#endif