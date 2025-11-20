#pragma once
#include "base.h"
#include "insns.h"

namespace rp2 {

// Section 7.5, Subsystem Resets
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
struct Resets {
    enum Index {
        ADC       = 0,
        BUSCTRL   = 1,
        DMA       = 2,
        HSTX      = 3,
        I2C0      = 4,
        I2C1      = 5,
        IOBANK0   = 6,
        IOQSPI    = 7,
        JTAG      = 8,
        PADSBANK0 = 9,
        PADSQSPI  = 10,
        PIO0      = 11,
        PIO1      = 12,
        PIO2      = 13,
        PLLSYS    = 14,
        PLLUSB    = 15,
        PWM       = 16,
        SHA256    = 17,
        SPI0      = 18,
        SPI1      = 19,
        SYSCFG    = 20,
        SYSINFO   = 21,
        TBMAN     = 22,
        TIMER0    = 23,
        TIMER1    = 24,
        TRNG      = 25,
        UART0     = 26,
        UART1     = 27,
        USBCTRL   = 28,
    };

    enum class Bit : u32 {
        ADC       = u32(1) << int(Index::ADC),
        BUSCTRL   = u32(1) << int(Index::BUSCTRL),
        DMA       = u32(1) << int(Index::DMA),
        HSTX      = u32(1) << int(Index::HSTX),
        I2C0      = u32(1) << int(Index::I2C0),
        I2C1      = u32(1) << int(Index::I2C1),
        IOBANK0   = u32(1) << int(Index::IOBANK0),
        IOQSPI    = u32(1) << int(Index::IOQSPI),
        JTAG      = u32(1) << int(Index::JTAG),
        PADSBANK0 = u32(1) << int(Index::PADSBANK0),
        PADSQSPI  = u32(1) << int(Index::PADSQSPI),
        PIO0      = u32(1) << int(Index::PIO0),
        PIO1      = u32(1) << int(Index::PIO1),
        PIO2      = u32(1) << int(Index::PIO2),
        PLLSYS    = u32(1) << int(Index::PLLSYS),
        PLLUSB    = u32(1) << int(Index::PLLUSB),
        PWM       = u32(1) << int(Index::PWM),
        SHA256    = u32(1) << int(Index::SHA256),
        SPI0      = u32(1) << int(Index::SPI0),
        SPI1      = u32(1) << int(Index::SPI1),
        SYSCFG    = u32(1) << int(Index::SYSCFG),
        SYSINFO   = u32(1) << int(Index::SYSINFO),
        TBMAN     = u32(1) << int(Index::TBMAN),
        TIMER0    = u32(1) << int(Index::TIMER0),
        TIMER1    = u32(1) << int(Index::TIMER1),
        TRNG      = u32(1) << int(Index::TRNG),
        UART0     = u32(1) << int(Index::UART0),
        UART1     = u32(1) << int(Index::UART1),
        USBCTRL   = u32(1) << int(Index::USBCTRL),
    };

    uv32 resets;    // 0x40020000
    uv32 wdSel;     // 0x40020004
    uv32 resetDone; // 0x40020008

    void reset(Bit _bit) {
        u32 bits = u32(_bit);
        resets |= bits;
    }

    void unreset(Bit _bit, bool wait = true) {
        u32 bits = u32(_bit);
        if (resets & bits || (resetDone & bits) != bits) {
            resets &= ~bits;
            while (wait && ((resetDone & bits) != bits)) { rp2::sys::nop(); }
        }
    }
};
inline auto& resets = *(Resets*)(0x40020000);

} // namespace rp2
