#pragma once
#include "base.h"

extern "C" {

// C/C++ ABI-specified functions

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memcpy(rp2350::u8* dest, rp2350::u8 const* src, rp2350::u32 n) {
    for (rp2350::u32 i = 0; i < n; i++) {
        // Read from source and write into dest; the do-nothing `asm volatile`
        // is only to separate the read and write, to prevent fusing them and optimizing
        // into a "memcpy" operation involving a call to `__aeabi_memcpy`, the very thing
        // we're trying to define.
        rp2350::u8 x = src[i];
        // asm volatile("");  // XXX actually needed??
        dest[i]      = x;
    }
}

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memcpy4(rp2350::u8* dest, rp2350::u8 const* src, unsigned n) {
    __aeabi_memcpy(dest, src, n);
}
}
