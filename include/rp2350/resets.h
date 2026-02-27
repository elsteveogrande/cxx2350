#pragma once

#include <platform.h>
#include <rp2350/insns.h>

namespace rp2350 {

// Section 7.5, Subsystem Resets
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
struct Resets {
    enum Index {
        ADC = 0,
        BUSCTRL = 1,
        DMA = 2,
        HSTX = 3,
        I2C0 = 4,
        I2C1 = 5,
        IOBANK0 = 6,
        IOQSPI = 7,
        JTAG = 8,
        PADSBANK0 = 9,
        PADSQSPI = 10,
        PIO0 = 11,
        PIO1 = 12,
        PIO2 = 13,
        PLLSYS = 14,
        PLLUSB = 15,
        PWM = 16,
        SHA256 = 17,
        SPI0 = 18,
        SPI1 = 19,
        SYSCFG = 20,
        SYSINFO = 21,
        TBMAN = 22,
        TIMER0 = 23,
        TIMER1 = 24,
        TRNG = 25,
        UART0 = 26,
        UART1 = 27,
        USBCTRL = 28,
    };

    enum class Bit : uint32_t {
        ADC = uint32_t(1) << int(Index::ADC),
        BUSCTRL = uint32_t(1) << int(Index::BUSCTRL),
        DMA = uint32_t(1) << int(Index::DMA),
        HSTX = uint32_t(1) << int(Index::HSTX),
        I2C0 = uint32_t(1) << int(Index::I2C0),
        I2C1 = uint32_t(1) << int(Index::I2C1),
        IOBANK0 = uint32_t(1) << int(Index::IOBANK0),
        IOQSPI = uint32_t(1) << int(Index::IOQSPI),
        JTAG = uint32_t(1) << int(Index::JTAG),
        PADSBANK0 = uint32_t(1) << int(Index::PADSBANK0),
        PADSQSPI = uint32_t(1) << int(Index::PADSQSPI),
        PIO0 = uint32_t(1) << int(Index::PIO0),
        PIO1 = uint32_t(1) << int(Index::PIO1),
        PIO2 = uint32_t(1) << int(Index::PIO2),
        PLLSYS = uint32_t(1) << int(Index::PLLSYS),
        PLLUSB = uint32_t(1) << int(Index::PLLUSB),
        PWM = uint32_t(1) << int(Index::PWM),
        SHA256 = uint32_t(1) << int(Index::SHA256),
        SPI0 = uint32_t(1) << int(Index::SPI0),
        SPI1 = uint32_t(1) << int(Index::SPI1),
        SYSCFG = uint32_t(1) << int(Index::SYSCFG),
        SYSINFO = uint32_t(1) << int(Index::SYSINFO),
        TBMAN = uint32_t(1) << int(Index::TBMAN),
        TIMER0 = uint32_t(1) << int(Index::TIMER0),
        TIMER1 = uint32_t(1) << int(Index::TIMER1),
        TRNG = uint32_t(1) << int(Index::TRNG),
        UART0 = uint32_t(1) << int(Index::UART0),
        UART1 = uint32_t(1) << int(Index::UART1),
        USBCTRL = uint32_t(1) << int(Index::USBCTRL),
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
        if ((resets & bits) || (resetDone & bits) != bits) {
            resets &= ~bits;
            while (wait && ((resetDone & bits) != bits)) { __nop(); }
        }
    }
};
inline auto& resets = *(Resets*)(0x40020000);

// Put everything except those parts needed for basic operation into reset.
[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void initResets() {
    // Turn reset on for everything except QSPI (since we're running on flash).
    // clang-format off
    constexpr static uint32_t kMask = 0
        | unsigned(Resets::Bit::ADC      )
        | unsigned(Resets::Bit::BUSCTRL  )
        | unsigned(Resets::Bit::DMA      )
        | unsigned(Resets::Bit::HSTX     )
        | unsigned(Resets::Bit::I2C0     )
        | unsigned(Resets::Bit::I2C1     )
        | unsigned(Resets::Bit::IOBANK0  )
        | unsigned(Resets::Bit::PADSBANK0)
        | unsigned(Resets::Bit::PIO0      )
        | unsigned(Resets::Bit::PIO1      )
        | unsigned(Resets::Bit::PIO2      )
        | unsigned(Resets::Bit::PWM       )
        | unsigned(Resets::Bit::SHA256    )
        | unsigned(Resets::Bit::SPI0      )
        | unsigned(Resets::Bit::SPI1      )
        | unsigned(Resets::Bit::TIMER0    )
        | unsigned(Resets::Bit::TIMER1    )
        | unsigned(Resets::Bit::TRNG      )
        | unsigned(Resets::Bit::UART0     )
        | unsigned(Resets::Bit::UART1     )
        | unsigned(Resets::Bit::USBCTRL   )
        // We won't reset these, they are needed for even minimal operation.
        // If the application wants to mess with these it still can, but we
        // won't automatically reset these.
        // | unsigned(Resets::Bit::IOQSPI    )
        // | unsigned(Resets::Bit::PADSQSPI  )
        // | unsigned(Resets::Bit::PLLSYS    )
        // | unsigned(Resets::Bit::PLLUSB    )
        // | unsigned(Resets::Bit::JTAG      )
        // | unsigned(Resets::Bit::SYSCFG    )
        // | unsigned(Resets::Bit::SYSINFO   )
        // | unsigned(Resets::Bit::TBMAN     )
    ;
    // clang-format on

    resets.resets |= kMask;
    // Some components seem to need a little bit of time before un-reset.
    for (unsigned i = 0; i < 1000000; i++) { __nop(); }
}

} // namespace rp2350
