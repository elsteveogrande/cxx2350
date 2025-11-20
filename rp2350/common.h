#pragma once

#include "abi.h"
#include "base.h"

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
