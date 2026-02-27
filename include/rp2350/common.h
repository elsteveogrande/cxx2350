#pragma once

#include <platform.h>

extern "C" {

// Application code to jump to post-init
extern void __start();

// Defined in the linker script
extern void* __sram_begin;
extern void* __sram_end;
extern void* __data_flash_begin;
extern void* __data_sram_begin;
extern void* __data_sram_end;
extern void* __init_array_begin;
extern void* __init_array_end;

} // extern "C"

namespace rp2350 {

// Clock config
constexpr static uint64_t kXOSC = 12'000'000;
constexpr static uint64_t kSysHz = 126'000'000; // target is 125'875'000
constexpr static uint64_t kFBDiv = 126;
constexpr static uint64_t kDiv1 = 4;
constexpr static uint64_t kDiv2 = 3;
static_assert(16 <= kFBDiv && kFBDiv <= 320);
static_assert(1 <= kDiv1 && kDiv1 <= 7);
static_assert(1 <= kDiv2 && kDiv2 <= kDiv1);
static_assert(kDiv1 >= kDiv2);
static_assert(kXOSC * kFBDiv >= 750'000'000);
static_assert(kXOSC * kFBDiv <= 1600'000'000);
static_assert(kXOSC * kFBDiv / (kDiv1 * kDiv2) == kSysHz);

// Image definition [IMAGE_DEF]: section 5.9, "Metadata Block Details".
struct [[gnu::aligned(64)]] ImageDef {
    struct [[gnu::packed]] Start {
        uint32_t const magic {0xffffded3};
    };

    template <uint16_t kFlags> struct [[gnu::packed]] ImageType {
        uint8_t const type = 0x42; // PICOBIN_BLOCK_ITEM_1BS_IMAGE_TYPE
        uint8_t const size = 0x01; //
        uint16_t const flags = kFlags;
    };

    // CHIP=2350, CPU=ARM, EXE=1, S=2
    struct [[gnu::packed]] ImageTypeARM : ImageType<0x1021> {};

    struct [[gnu::packed]] ItemListEnd {
        uint8_t sizeType = 0xff; // BLOCK_ITEM_LAST has a 2-byte size
        uint16_t totalSize = 1;  // Total of preceding items' sizes (in words)
        uint8_t _pad = 0;        //
    };

    struct [[gnu::packed]] End {
        uint32_t end = 0xab123579; // End magic
    };
};

struct [[gnu::packed]] ImageDef2350ARM : ImageDef {
    Start __start {};
    ImageTypeARM _img0 {};
    ItemListEnd _listEnd {};
    uint32_t link = 0; // No link since this is only 1 block
    End _end;
};

struct R32 {
    uint32_t& u32() { return *reinterpret_cast<uint32_t*>(this); }
};

struct U16 {
    uint16_t& u16() { return *reinterpret_cast<uint16_t*>(this); }
};

template <class R, class NVR = __remove_volatile(R)> struct Update {
    R* reg_;
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
