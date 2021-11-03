// Copyright � 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <malloc.h>

#if defined(__GNUC__) 
#if defined(__x86_64)
#define CPU_PLATFORM_X64
#elif defined(__aarch64__) || defined(__arm__)
#define CPU_PLATFORM_AARCH64
#endif
#endif

#if defined(_MSC_VER) 
#if defined(_M_X64)
#define CPU_PLATFORM_X64
#elif defined(_M_ARM) || defined(_M_ARM64)
#define CPU_PLATFORM_AARCH64
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#define MP2V_INLINE                   inline __attribute__((always_inline))
#define ALIGN(n)                      __attribute__ ((aligned(n)))
#else
#define MP2V_INLINE                   __forceinline
#define ALIGN(n)                      __declspec(align(n))
#endif

#if defined(_MSC_VER)
#include <intrin.h>
#define bswap_16(x) _byteswap_ushort(x)
#define bswap_32(x) _byteswap_ulong(x)

MP2V_INLINE uint32_t bit_scan_reverse(uint32_t x)
{
    unsigned long index;
    _BitScanReverse(&index, x);
    return x ? 31 - index : 32;
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

template <typename T, size_t N = 16>
class AlignmentAllocator {
public:
    typedef T value_type;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type;

    typedef T* pointer;
    typedef const T* const_pointer;

    typedef T& reference;
    typedef const T& const_reference;

public:
    inline AlignmentAllocator() throw () { }

    template <typename T2>
    inline AlignmentAllocator(const AlignmentAllocator<T2, N>&) throw () { }

    inline ~AlignmentAllocator() throw () { }

    inline pointer adress(reference r) {
        return &r;
    }

    inline const_pointer adress(const_reference r) const {
        return &r;
    }

#if defined(_MSC_VER)
    inline pointer allocate(size_type n) {
        return (pointer)_aligned_malloc(n * sizeof(value_type), N);
    }

    inline void deallocate(pointer p, size_type) {
        _aligned_free(p);
    }
#else
    inline pointer allocate(size_type n) {
        return (pointer)aligned_alloc(N, n * sizeof(value_type));
    }

    inline void deallocate(pointer p, size_type) {
        free(p);
    }
#endif

    inline void construct(pointer p, const value_type& wert) {
        new (p) value_type(wert);
    }

    inline void destroy(pointer p) {
        p->~value_type();
    }

    inline size_type max_size() const throw () {
        return size_type(-1) / sizeof(value_type);
    }

    template <typename T2>
    struct rebind {
        typedef AlignmentAllocator<T2, N> other;
    };

    bool operator!=(const AlignmentAllocator<T, N>& other) const {
        return !(*this == other);
    }

    // Returns true if and only if storage allocated from *this
    // can be deallocated from other, and vice versa.
    // Always returns true for stateless allocators.
    bool operator==(const AlignmentAllocator<T, N>& other) const {
        return true;
    }
};