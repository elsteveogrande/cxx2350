#pragma once

#include <platform.h>
#include <rp2350/common.h>
#include <rp2350/pads.h>
#include <rp2350/resets.h>

namespace rp2350 {

// TODO
//  To clear the interrupt, write to the relevant bits of the Interrupt Clear Register,
//  UARTICR (bits 7 to 10 are the error clear bits).

// Section 12.1, "UART"
template <unsigned U> struct UART {
    struct Data : R32 {
        unsigned data         : 8; // 7..0
        unsigned framingError : 1; // 8 write-to-clear
        unsigned parityError  : 1; // 9 write-to-clear
        unsigned breakError   : 1; // 10 write-to-clear
        unsigned overrunError : 1; // 11 write-to-clear
        unsigned              : 20;
    };

    struct RXStatus : R32 {
        unsigned framingError : 1; // 0 write-to-clear
        unsigned parityError  : 1; // 1 write-to-clear
        unsigned breakError   : 1; // 2 write-to-clear
        unsigned overrunError : 1; // 3 write-to-clear
        unsigned              : 28;
    };

    struct Flags : R32 {
        unsigned cts     : 1; // 0
        unsigned dsr     : 1; // 1
        unsigned dcd     : 1; // 2
        unsigned busy    : 1; // 3
        unsigned rxEmpty : 1; // 4
        unsigned txFull  : 1; // 5
        unsigned rxFull  : 1; // 6
        unsigned txEmpty : 1; // 7
        unsigned ring    : 1; // 8
        unsigned         : 23;
    };

    // Integer part of `(CLK_PERI / target baud)`
    struct IntBaud {
        unsigned div : 16; // 15..0
        unsigned     : 16;
    };

    // Fractional (sixty-fourths) part of `(CLK_PERI / target baud)`; represents (div /
    // 64)
    struct FracBaud {
        unsigned div : 6; // 5..0
        unsigned     : 26;
    };

    enum class WordLength : unsigned {
        k5Bit = 0,
        k6Bit = 1,
        k7Bit = 2,
        k8Bit = 3,
    };

    struct LineControl : R32 {
        unsigned brk          : 1;  // 0
        unsigned parityEnable : 1;  // 1
        unsigned evenParity   : 1;  // 2
        unsigned stop2        : 1;  // 3
        unsigned fifoEnable   : 1;  // 4
        WordLength wordLength : 2;  // 6..5
        unsigned stickParity  : 1;  // 7
        unsigned              : 24; // 8
    };

    struct Control : R32 {
        unsigned enable      : 1; // 0
        unsigned sirEnable   : 1; // 1
        unsigned sirLowPower : 1; // 2
        unsigned             : 4;
        unsigned loopback    : 1; // 7
        unsigned txEnable    : 1; // 8
        unsigned rxEnable    : 1; // 9
        unsigned dtr         : 1; // 10
        unsigned rts         : 1; // 11
        unsigned out1        : 1; // 12
        unsigned out2        : 1; // 13
        unsigned rtsEnable   : 1; // 14
        unsigned ctsEnable   : 1; // 15
        unsigned             : 16;
    };

    struct IntFIFOLevel {
        unsigned txLevelSel : 3; // 2..0: 0:(1/8 full) 1:(1/4) 2(1/2) 3(3/4) 4(7/8)
        unsigned rxLevelSel : 3; // 5..3: 0:(1/8 full) 1:(1/4) 2(1/2) 3(3/4) 4(7/8)
        unsigned            : 26;
    };

    struct IntMask : R32 {
        unsigned ri  : 1; // 0
        unsigned cts : 1; // 1
        unsigned dcd : 1; // 2
        unsigned dsr : 1; // 3
        unsigned rx  : 1; // 4
        unsigned tx  : 1; // 5
        unsigned rt  : 1; // 6
        unsigned fe  : 1; // 7
        unsigned pe  : 1; // 8
        unsigned be  : 1; // 9
        unsigned oe  : 1; // 10
        unsigned     : 21;
    };

    struct DMAControl {
        unsigned rxDMAEnable : 1; // 0
        unsigned txDMAEnable : 1; // 1
        unsigned dmaOnError  : 1; // 2
        unsigned             : 29;
    };

    Data data;                 // 0x000
    RXStatus rxStatus;         // 0x004
    uint32_t z_008;            //
    uint32_t z_00c;            //
    uint32_t z_010;            //
    uint32_t z_014;            //
    Flags flags;               // 0x018
    uint32_t z_01c;            //
    uint32_t sirLowPower;      // 0x020
    IntBaud intBaud;           // 0x024
    FracBaud fracBaud;         // 0x028
    LineControl lineControl;   // 0x02c
    Control control;           // 0x030
    IntFIFOLevel intFIFOLevel; // 0x034
    IntMask intMask;           // 0x038
    IntMask rawIntStatus;      // 0x03c
    IntMask maskedIntStatus;   // 0x040
    IntMask intClear;          // 0x044
    DMAControl dmaControl;     // 0x048
    // uint32_t periph0;         // 0xfe0
    // uint32_t periph1;         // 0xfe4
    // uint32_t periph2;         // 0xfe8
    // uint32_t periph3;         // 0xfec
    // uint32_t pCell0;          // 0xff0
    // uint32_t pCell1;          // 0xff4
    // uint32_t pCell2;          // 0xff8
    // uint32_t pCell3;          // 0xffc

    void init(uint32_t sysHz, uint32_t baud) {
        // TODO take parameters.  For now hardcode 8/N/1
        // TODO understand what's going on here.  Mostly a copy of uart_set_baudrate on
        // PDF p973 or:
        // https://github.com/raspberrypi/pico-sdk/blob/master/src/rp2_common/hardware_uart/uart.c#L155-L180
        // or: https://developer.arm.com/documentation/ddi0183/latest/
        uint32_t div = ((8 * sysHz) / baud) + 1;
        uint32_t dint = div >> 7;
        uint32_t dfrac = (div & 0x7f) >> 1;
        intBaud.div = dint;
        fracBaud.div = dfrac;
        // These control register writes also latch the divisors set above
        update(&lineControl, [&](auto& _) {
            _.zero();
            _->fifoEnable = true;
            _->wordLength = WordLength::k8Bit;
        });
        update(&control, [](auto& _) {
            _.zero();
            _->enable = true;
            _->txEnable = true;
            _->rxEnable = true;
        });
        update(&dmaControl, [](auto& _) {
            _.zero();
            // SDK unconditionally enables these (harmless even if not using DMA)
            _->rxDMAEnable = true;
            _->txDMAEnable = true;
        });
        update(&intFIFOLevel, [](auto& _) {
            _->txLevelSel = 0;
            _->rxLevelSel = 0;
        });
        update(&intMask, [](auto& _) {
            _.zero();
            _->tx = true;
            _->rx = true;
            _->rt = true;
        });
    }

    constexpr static unsigned irqn() {
        switch (U) {
        case 0: return 33;
        case 1: return 34;
        default: __unreachable();
        }
    }
};
inline auto& uart0 = *(UART<0>*)(0x40070000);
inline auto& uart1 = *(UART<1>*)(0x40078000);

}; // namespace rp2350
