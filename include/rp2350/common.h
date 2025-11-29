#pragma once

#include <cxx20/cxxabi.h>

extern "C" {

// Application code to jump to post-init
extern void _start();

// Defined in the linker script
extern void* __reset_sp;
extern void* __bss_base;
extern void* __bss_end;
extern void* __init_array_base;
extern void* __init_array_end;

} // extern "C"

namespace rp2350 {
namespace sys {

// Global config
constexpr static uint64_t kXOSC  = 12'000'000;
constexpr static uint64_t kSysHz = 270'000'000;
constexpr static uint64_t kFBDiv = 90;
constexpr static uint64_t kDiv1  = 4;
constexpr static uint64_t kDiv2  = 1;
// Verify
static_assert(16 <= kFBDiv && kFBDiv <= 320);
static_assert(1 <= kDiv1 && kDiv1 <= 7);
static_assert(1 <= kDiv2 && kDiv2 <= 7);
static_assert(kDiv1 >= kDiv2);
static_assert(kXOSC * kFBDiv >= 750'000'000);
static_assert(kXOSC * kFBDiv <= 1600'000'000);
static_assert(kXOSC * kFBDiv / (kDiv1 * kDiv2) == kSysHz);

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

struct R32 {
    uint32_t& u32() { return *reinterpret_cast<uint32_t*>(this); }
};

template <class R, class NVR = __remove_volatile(R)> struct Update {
    R*       reg_;
    uint32_t val_;

    ~Update() { write(); }
    Update(R* reg) : reg_(reg), val_(*(uint32_t*)reg) {}

    NVR* operator->() { return (NVR*)&val_; }

    auto& zero() {
        val_ = 0;
        return *this;
    }

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
