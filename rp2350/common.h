#pragma once

/*
https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
https://developer.arm.com/documentation/100235/0100/The-Cortex-M33-Processor/Exception-model/Vector-table
*/

namespace rp2 {
using u8    = unsigned char;
using u16   = unsigned short;
using u32   = unsigned long;
using uv32  = u32 volatile;
using uint  = u32;
using uvint = uint volatile;
using uptr  = u32*;
typedef void (*vfunc)();
} // namespace rp2

extern "C" {

// Application code to jump to post-init
void _start();

// Defined in the linker script
extern rp2::uptr __vec_table;
extern rp2::uptr __reset_sp;
extern rp2::uptr __init_array_base;
extern rp2::uptr __init_array_end;
extern rp2::uptr __bss_base;
extern rp2::uptr __bss_end;
extern rp2::uptr __heap_base;
extern rp2::uptr __heap_end;
extern rp2::uptr __buf_x_base;
extern rp2::uptr __buf_x_end;
extern rp2::uptr __buf_y_base;
extern rp2::uptr __buf_y_end;

// C/C++ ABI-specified functions

inline void __aeabi_memcpy(rp2::u8* dest, rp2::u8 const* src, rp2::u32 n) {
    for (rp2::u32 i = 0; i < n; i++) {
        // Read from source and write into dest; the do-nothing `asm volatile`
        // is only to separate the read and write, to prevent fusing them and optimizing
        // into a "memcpy" operation involving a call to `__aeabi_memcpy`, the very thing
        // we're trying to define.
        rp2::u8 x = src[i];
        // asm volatile("");  // XXX actually needed??
        dest[i]   = x;
    }
}

inline void __aeabi_memcpy4(rp2::u8* dest, rp2::u8 const* src, unsigned n) {
    __aeabi_memcpy(dest, src, n);
}

} // extern "C"

namespace rp2::sys {

// Image definition [IMAGE_DEF]: section 5.9, "Metadata Block Details".
struct [[gnu::packed]] ImageDef2350ARM {
    u32 start     = 0xffffded3; // Start magic
    u8  type      = 0x42;       //
    u8  size      = 0x01;       //
    u16 flags     = 0x1021;     // Item 0: CHIP=2350, CPU=ARM, EXE=1, S=2
    u8  sizeType  = 0xff;       // BLOCK_ITEM_LAST has a 2-byte size
    u16 totalSize = 1;          // Total of preceding items' sizes (in words)
    u8  _pad      = 0;          //
    u32 link      = 0;          // No link since this is only 1 block
    u32 end       = 0xab123579; // End magic
};

} // namespace rp2::sys
