#pragma once

#include <platform.h>

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

// Global config
constexpr static uint64_t kXOSC = 12'000'000;
constexpr static uint64_t kSysHz = 126'000'000; // target is 125'875'000
constexpr static uint64_t kFBDiv = 126;
constexpr static uint64_t kDiv1 = 4;
constexpr static uint64_t kDiv2 = 3;
// Verify
static_assert(16 <= kFBDiv && kFBDiv <= 320);
static_assert(1 <= kDiv1 && kDiv1 <= 7);
static_assert(1 <= kDiv2 && kDiv2 <= kDiv1);
static_assert(kDiv1 >= kDiv2);
static_assert(kXOSC * kFBDiv >= 750'000'000);
static_assert(kXOSC * kFBDiv <= 1600'000'000);
static_assert(kXOSC * kFBDiv / (kDiv1 * kDiv2) == kSysHz);

// Image definition [IMAGE_DEF]: section 5.9, "Metadata Block Details".
struct ImageDef {
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
    Start _start {};
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

// 12.6. DMA
// 12.6.10. List of Registers
struct DMA {
    enum class DataSize : unsigned {
        _8BIT = 0,
        _16BIT = 1,
        _32BIT = 2,
    };

    // p.1128
    struct Control {
        unsigned enable       : 1; // 0
        unsigned highPri      : 1; // 1
        DataSize dataSize     : 2; // 3..2
        unsigned incrRead     : 1; // 4
        unsigned incrReadRev  : 1; // 5
        unsigned incrWrite    : 1; // 6
        unsigned incrWriteRev : 1; // 7
        unsigned ringSize     : 4; // 11..8
        unsigned ringSel      : 1; // 12
        unsigned chainTo      : 4; // 16..13
        unsigned treqSel      : 6; // 22..17
        unsigned irqQuiet     : 1; // 23
        unsigned bswap        : 1; // 24
        unsigned sniffEnable  : 1; // 25
        unsigned busy         : 1; // 26
        unsigned              : 2; //
        unsigned writeError   : 1; // 29
        unsigned readError    : 1; // 30
        unsigned ahbError     : 1; // 31
    };

    enum class Mode {
        NORMAL = 0,       // Decrement on each xfer until 0, then trigger CHAIN_TO
        TRIGGER_SELF = 1, // Like NORMAL but trigger self instead
        ENDLESS = 0x0f,   // No decrement, no chain, no IRQ; xfer endlessly until ABORT
    };

    struct TransCount {
        unsigned count : 28; // 27..0
        Mode mode      : 4;  // 21..28
    };

    // p.1126
    struct Channel {
        uintptr_t readAddr;  // updates automatically each time a read completes
        uintptr_t writeAddr; // updates automatically each time a write completes
        TransCount transCount;
        Control ctrlTrig;
        Control ctrl;
        unsigned z014[2];
        TransCount transCountTrig;
        unsigned z020[3];
        uintptr_t writeAddrTrig;
        unsigned z030[3];
        uintptr_t readAddrTrig;
    };
    static_assert(sizeof(Channel) == 64);

    // p.1132
    struct IRQ {
        uint32_t enable; // 15..0 only
        uint32_t force;  // 15..0 only
        uint32_t status; // 15..0 only
    };
    static_assert(sizeof(IRQ) == 12);

    struct Trigger {
        uint32_t channels; // 15..0 only
    };

    Channel channels[16];     // 0x50000000, 0x50000040, ...
    uint32_t rawStatus;       // 0x50000400
    IRQ irq0;                 // 0x50000404, 408, 40c, ...
    uint32_t _z50000410;      //
    IRQ irq1;                 // 0x50000414, 418, 41c, ...
    uint32_t _z50000420;      //
    IRQ irq2;                 // 0x50000424, 428, 42c, ...
    uint32_t _z50000430;      //
    IRQ irq3;                 // 0x50000434, 438, 43c, ...
    uint32_t timers[4];       // 0x50000440, 0x50000444, ...
    Trigger multiChanTrigger; // 0x50000450

    IRQ& irqRegs(unsigned i) {
        switch (i) {
        case 0: return irq0;
        case 1: return irq1;
        case 2: return irq2;
        case 3: return irq3;
        default: __builtin_unreachable(); // TODO: panic
        }
    }

    constexpr static unsigned kDMAIRQs[4] {10, 11, 12, 13}; // p.84
};
inline auto& dma = *(DMA*)(0x50000000);

// 12.11. HSTX
struct HSTX {
    // 12.11.7. List of control registers [p.1209]

    struct CSR : R32 {
        unsigned enable       : 1 {}; // 0
        unsigned expandEnable : 1 {}; // 1
        unsigned              : 2;    //
        unsigned coupled      : 1 {}; // 4
        unsigned coupleSel    : 2 {}; // 6..5
        unsigned              : 1;    //
        unsigned shift        : 5 {}; // 12..8
        unsigned              : 3;    //
        unsigned nShifts      : 5 {}; // 20..16
        unsigned              : 3;    //
        unsigned clkPhase     : 4 {}; // 27..24
        unsigned clkDiv       : 4 {}; // 31..28
    };

    struct Bit : R32 {
        unsigned selectP : 5 {}; // 4..0
        unsigned         : 3;    //
        unsigned selectN : 5 {}; // 12..8
        unsigned         : 3;    //
        unsigned invert  : 1 {}; // 16
        unsigned clock   : 1 {}; // 17
        unsigned         : 14;   //
    };

    // p.1212
    struct ExShift : R32 {
        unsigned rawShift   : 5; // 4..0
        unsigned            : 3; //
        unsigned rawNShifts : 5; // 12..8
        unsigned            : 3; //
        unsigned encShift   : 5; // 20..16
        unsigned            : 3; //
        unsigned encNShifts : 5; // 28..24
        unsigned            : 3; //
    };

    // p.1212
    struct ExTMDS : R32 {
        unsigned l0Rot   : 5; // 4..0
        unsigned l0NBits : 3; // 7..5
        unsigned l1Rot   : 5; // 4..0
        unsigned l1NBits : 3; // 7..5
        unsigned l2Rot   : 5; // 4..0
        unsigned l2NBits : 3; // 7..5
        unsigned         : 8; //
    };

    CSR csr;
    Bit bits[8];
    ExShift expandShift;
    ExTMDS expandTMDS;

    // 12.11.8. List of FIFO registers [p.1213]

    struct Status {
        unsigned level       : 8;
        unsigned full        : 1;
        unsigned empty       : 1;
        unsigned wroteOnFull : 1; // Write 1 to this to clear
        unsigned             : 21;
    };

    struct FIFO {
        Status stat;
        uint32_t fifoWrite;
    };
    FIFO& fifo() { return *(FIFO*)(0x50600000); }
};
inline auto& hstx = *(HSTX*)(0x400c0000);

[[gnu::used]] [[clang::always_inline]] [[noreturn]]
inline void __abort() {
    asm volatile(".short 0xde00");
    __builtin_unreachable();
}

} // namespace rp2350
