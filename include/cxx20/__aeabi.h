#pragma once

#include <cxx20/__base.h>

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
