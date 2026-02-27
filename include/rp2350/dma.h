#pragma once

#include <platform.h>
#include <rp2350/common.h>
#include <rp2350/resets.h>

namespace rp2350 {

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
        default: __unreachable(); // TODO: panic
        }
    }

    constexpr static unsigned kDMAIRQs[4] {10, 11, 12, 13}; // p.84
};
inline auto& dma = *(DMA*)(0x50000000);

inline void initDMA() { resets.unreset(Resets::Bit::DMA, true); }

} // namespace rp2350
