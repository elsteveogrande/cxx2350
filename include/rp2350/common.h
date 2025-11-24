#pragma once

#include <cxx20/cxxabi.h>

extern "C" {

// Application code to jump to post-init
void _start();

// Defined in the linker script
extern void* __vec_table;
extern void* __reset_sp;
extern void* __init_array_base;
extern void* __init_array_end;
extern void* __bss_base;
extern void* __bss_end;
extern void* __heap_base;
extern void* __heap_end;
extern void* __buf_x_base;
extern void* __buf_x_end;
extern void* __buf_y_base;
extern void* __buf_y_end;

} // extern "C"

namespace rp2350 {
namespace sys {

// Image definition [IMAGE_DEF]: section 5.9, "Metadata Block Details".
struct [[gnu::packed]] ImageDef2350ARM {
    uint32_t start     = 0xffffded3; // Start magic
    uint8_t  type      = 0x42;       //
    uint8_t  size      = 0x01;       //
    uint16_t flags     = 0x1021;     // Item 0: CHIP=2350, CPU=ARM, EXE=1, S=2
    uint8_t  sizeType  = 0xff;       // BLOCK_ITEM_LAST has a 2-byte size
    uint16_t totalSize = 1;          // Total of preceding items' sizes (in words)
    uint8_t  _pad      = 0;          //
    uint32_t link      = 0;          // No link since this is only 1 block
    uint32_t end       = 0xab123579; // End magic
};

} // namespace sys

template <class R, class NVR = __remove_volatile(R)> struct Update {
    R*       reg_;
    uint32_t val_;
    Update(R* reg) : reg_(reg), val_(*(uint32_t*)reg) {}
    NVR* operator->() { return (NVR*)&val_; }

    ~Update() { write(); }
    void write() {
        if (reg_) { *(uint32_t volatile*)reg_ = val_; }
        reg_ = nullptr;
    }
};

template <class R> void update(R* reg, auto cb) {
    Update u(reg);
    cb(u);
};

} // namespace rp2350
