#pragma once

// Declare these basic types into the global namespace.
using int8_t   = char;
using int16_t  = short;
using int32_t  = long;
using int64_t  = long long;
using uint8_t  = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned long;
using uint64_t = unsigned long long;
static_assert(sizeof(int8_t) == 1);
static_assert(sizeof(int16_t) == 2);
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(int64_t) == 8);
static_assert(sizeof(uint8_t) == 1);
static_assert(sizeof(uint16_t) == 2);
static_assert(sizeof(uint32_t) == 4);
static_assert(sizeof(uint64_t) == 8);

extern "C" {

// C/C++ ABI-specified functions

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memcpy(uint8_t* dest, uint8_t const* src, uint32_t n) {
    for (uint32_t i = 0; i < n; i++) {
        // Read from source and write into dest; the do-nothing `asm volatile`
        // is only to separate the read and write, to prevent fusing them and optimizing
        // into a "memcpy" operation involving a call to `__aeabi_memcpy`, the very thing
        // we're trying to define.
        uint8_t x = src[i];
        // asm volatile("");  // XXX actually needed??
        dest[i]   = x;
    }
}

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memcpy4(uint8_t* dest, uint8_t const* src, unsigned n) {
    __aeabi_memcpy(dest, src, n);
}
}

typedef void (*vfunc)();
