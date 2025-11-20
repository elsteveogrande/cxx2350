#pragma once

#include <cxx20/cxxabi.h>
#include <rp2350/insns.h>

namespace rp2350 {

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

    enum class Bit : uint32_t {
        ADC       = uint32_t(1) << int(Index::ADC),
        BUSCTRL   = uint32_t(1) << int(Index::BUSCTRL),
        DMA       = uint32_t(1) << int(Index::DMA),
        HSTX      = uint32_t(1) << int(Index::HSTX),
        I2C0      = uint32_t(1) << int(Index::I2C0),
        I2C1      = uint32_t(1) << int(Index::I2C1),
        IOBANK0   = uint32_t(1) << int(Index::IOBANK0),
        IOQSPI    = uint32_t(1) << int(Index::IOQSPI),
        JTAG      = uint32_t(1) << int(Index::JTAG),
        PADSBANK0 = uint32_t(1) << int(Index::PADSBANK0),
        PADSQSPI  = uint32_t(1) << int(Index::PADSQSPI),
        PIO0      = uint32_t(1) << int(Index::PIO0),
        PIO1      = uint32_t(1) << int(Index::PIO1),
        PIO2      = uint32_t(1) << int(Index::PIO2),
        PLLSYS    = uint32_t(1) << int(Index::PLLSYS),
        PLLUSB    = uint32_t(1) << int(Index::PLLUSB),
        PWM       = uint32_t(1) << int(Index::PWM),
        SHA256    = uint32_t(1) << int(Index::SHA256),
        SPI0      = uint32_t(1) << int(Index::SPI0),
        SPI1      = uint32_t(1) << int(Index::SPI1),
        SYSCFG    = uint32_t(1) << int(Index::SYSCFG),
        SYSINFO   = uint32_t(1) << int(Index::SYSINFO),
        TBMAN     = uint32_t(1) << int(Index::TBMAN),
        TIMER0    = uint32_t(1) << int(Index::TIMER0),
        TIMER1    = uint32_t(1) << int(Index::TIMER1),
        TRNG      = uint32_t(1) << int(Index::TRNG),
        UART0     = uint32_t(1) << int(Index::UART0),
        UART1     = uint32_t(1) << int(Index::UART1),
        USBCTRL   = uint32_t(1) << int(Index::USBCTRL),
    };

    uint32_t resets;    // 0x40020000
    uint32_t wdSel;     // 0x40020004
    uint32_t resetDone; // 0x40020008

    void reset(Bit _bit) {
        uint32_t bits = uint32_t(_bit);
        resets |= bits;
    }

    void unreset(Bit _bit, bool wait = true) {
        uint32_t bits = uint32_t(_bit);
        if (resets & bits || (resetDone & bits) != bits) {
            resets &= ~bits;
            while (wait && ((resetDone & bits) != bits)) { rp2350::sys::nop(); }
        }
    }
};
inline auto& resets = *(Resets*)(0x40020000);

} // namespace rp2350
