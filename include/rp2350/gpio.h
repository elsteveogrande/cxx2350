#pragma once

#include <cxx20/cxxabi.h>
#include <rp2350/common.h>
#include <rp2350/pads.h>
#include <rp2350/resets.h>

namespace rp2350 {

// GPIO and SIO

// Section 9, GPIO
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
//
// These are for GPIO bank 0 (GPIOs 0 through 31) only.
struct GPIO {
    struct Status {
        unsigned : 9; // 8..0
        unsigned outToPad
            : 1;      // 9: output signal to pad after register override is applied
        unsigned : 3; // 12..10
        unsigned oeToPad
            : 1; // 13: output enable to pad after register override is applied
        unsigned           : 3; // 16..14
        unsigned inFromPad : 1; // 17: input signal from pad, before override is applied
        unsigned           : 8; // 25..18
        unsigned irqToProc
            : 1;      // 26: interrupt to processors, after override is applied
        unsigned : 5; // 31..27
    };

    enum class Override : unsigned {
        kNormal  = 0,
        kInvert  = 1,
        kLow     = 2,
        kDisable = 2,
        kHigh    = 3,
        kEnable  = 3,
    };

    // 9.4. Function Select; note that the correct funcsel depends on the GPIO number
    template <unsigned GPIO> struct FuncSel;

    // clang-format off
    // Table on p.591
    // TODO!  Fill in; double-check; maybe automate this

    template <> struct FuncSel< 0> { constexpr static unsigned         SPI0RX=1,    UART0TX=2,  I2C0SDA=3,  PWM0A=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8, QMICS1n=9,      USBOVRCURDET=10; };
    template <> struct FuncSel< 1> { constexpr static unsigned         SPI0CSn=1,   UART0RX=2,  I2C0SCL=3,  PWM0B=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8, TRACECLK=9,     USBVBUSDET=10; };
    template <> struct FuncSel< 2> {};
    template <> struct FuncSel< 3> {};
    template <> struct FuncSel< 4> {};
    template <> struct FuncSel< 5> {};
    template <> struct FuncSel< 6> {};
    template <> struct FuncSel< 7> {};
    template <> struct FuncSel< 8> {};
    template <> struct FuncSel< 9> {};
    template <> struct FuncSel<10> {};
    template <> struct FuncSel<11> {};
    template <> struct FuncSel<12> { constexpr static unsigned HSTX=0, SPI1RX=1,    UART0TX=2,  I2C0SDA=3,  PWM6A=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8, CLK_GPIN0=9,    USBOVRCURDET=10; };
    template <> struct FuncSel<13> { constexpr static unsigned HSTX=0, SPI1CSn=1,   UART0RX=2,  I2C0SCL=3,  PWM6B=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8, CLK_GPOUT0=9,   USBVBUSDET=10; };
    template <> struct FuncSel<14> { constexpr static unsigned HSTX=0, SPI1SCK=1,   UART0CTS=2, I2C1SDA=3,  PWM7A=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8, CLK_GPIN1=9,    USBVBUSEN=10; };
    template <> struct FuncSel<15> { constexpr static unsigned HSTX=0, SPI1TX=1,    UART0RTS=2, I2C1SCL=3,  PWM7B=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8, CLK_GPOUT1=9,   USBOVRCURDET=10; };
    template <> struct FuncSel<16> { constexpr static unsigned HSTX=0, SPI0RX=1,    UART0TX=2,  I2C0SDA=3,  PWM0A=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8,                 USBVBUSDET=10; };
    template <> struct FuncSel<17> { constexpr static unsigned HSTX=0, SPI0RX=1,    UART0RX=2,  I2C0SCL=3,  PWM0B=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8,                 USBVBUSEN=10; };
    template <> struct FuncSel<18> { constexpr static unsigned HSTX=0, SPI0RX=1,    UART0CTS=2, I2C1SDA=3,  PWM1A=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8,                 USBOVRCURDET=10; };
    template <> struct FuncSel<19> { constexpr static unsigned HSTX=0, SPI0RX=1,    UART0RTS=2, I2C1SCL=3,  PWM1B=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8, QMICS1n=9,      USBVBUSDET=10; };
    template <> struct FuncSel<20> {};
    template <> struct FuncSel<21> {};
    template <> struct FuncSel<22> {};
    template <> struct FuncSel<23> {};
    template <> struct FuncSel<24> {};
    template <> struct FuncSel<25> { constexpr static unsigned         SPI0CSn=1,   UART0RX=2,  I2C0SCL=3,  PWM4B=4,    SIO=5, PIO0=6, PIO1=7, PIO2=8, CLKGPOUT3=9,    USBVBUSDET=10;};
    template <> struct FuncSel<26> {};
    template <> struct FuncSel<27> {};
    template <> struct FuncSel<28> {};
    template <> struct FuncSel<29> {};
    // clang-format on

    struct Control {
        unsigned funcSel : 5;  // 4..0
        unsigned         : 7;  // 11..5
        Override outOver : 2;  // 13..12
        Override oeOver  : 2;  // 15..14
        Override inOver  : 2;  // 17..16
        unsigned         : 10; // 27..18
        Override irqOver : 2;  // 29..28
        unsigned         : 2;  // 31..30
    };

    struct GPIORegPair {
        Status status;
        Control control;
    };

    GPIORegPair array[32];
    GPIORegPair& operator[](unsigned i) { return array[i]; }
};
inline auto& gpio = *(GPIO*)(0x40028000);

// Section 9.8, Processor GPIO Controls (SIO)
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
//
// Note that for simplicity this only supports Bank0.
// We'll assume Bank1 is not dealt with; the pins in Bank1
// will then continue to be reserved only for QSPI.
struct SIO : R32 {
    unsigned cpuID;         // 0xd0000000
    unsigned gpioIn;        // 0xd0000004
    unsigned z_008;         // 0xd0000008
    unsigned z_00c;         // 0xd000000c
    unsigned gpioOut;       // 0xd0000010
    unsigned z_014;         // 0xd0000014
    unsigned gpioOutSet;    // 0xd0000018
    unsigned z_01c;         // 0xd000001c
    unsigned gpioOutClr;    // 0xd0000020
    unsigned z_0;           // 0xd0000024
    unsigned gpioOutXor;    // 0xd0000028
    unsigned z_02c;         // 0xd000002c
    unsigned gpioOutEnb;    // 0xd0000030
    unsigned z_034;         // 0xd0000034
    unsigned gpioOutEnbSet; // 0xd0000038
    unsigned z_03c;         // 0xd000003c
    unsigned gpioOutEnbClr; // 0xd0000040
    unsigned z_044;         // 0xd0000044
    unsigned gpioOutEnbXor; // 0xd0000048

    // TODO:
    // fifo(Status,Write)
    // spinlockState
    // 0x080 - 0x0bc , INTERP0
    // 0x0c0 - 0x0fc , INTERP1
    // 0x100 - 0x17c , SPINLOCKn
    // lots more
};
inline auto& sio = *(SIO*)(0xd0000000);

template <uint8_t I> void initOutput(unsigned funcSel = GPIO::FuncSel<I>::SIO) {
    gpio[I].control.funcSel = funcSel;
    sio.gpioOutEnbSet       = (1 << I);
    sio.gpioOutClr          = (1 << I);

    {
        Update u {&gpio[I].control};
        u->funcSel = funcSel;
        u->inOver  = GPIO::Override::kNormal;
        u->irqOver = GPIO::Override::kNormal;
        u->outOver = GPIO::Override::kNormal;
        u->oeOver  = GPIO::Override::kNormal;
    }

    {
        Update u {&padsBank0.gpio[I]};
        u->drive         = PadsBank0::Drive::k12mA;
        u->inputEnable   = false;
        u->outputDisable = false;
        u->isolation     = false;
    }
}

template <uint8_t I> void initInput(unsigned funcSel = GPIO::FuncSel<I>::SIO) {
    sio.gpioOutEnbClr = (1 << I);

    {
        Update u {&gpio[I].control};
        u->funcSel = funcSel;
        u->inOver  = GPIO::Override::kNormal;
        u->irqOver = GPIO::Override::kNormal;
        u->outOver = GPIO::Override::kNormal;
        u->oeOver  = GPIO::Override::kNormal;
    }

    {
        Update u {&padsBank0.gpio[I]};
        u->slewFast      = true;
        u->schmitt       = false;
        u->inputEnable   = true;
        u->outputDisable = true;
        u->pullDown      = false;
        u->pullUp        = false;
        u->isolation     = false;
    }
}

namespace sys {
inline void initGPIO() {
    resets.unreset(Resets::Bit::PADSBANK0, true);
    resets.unreset(Resets::Bit::IOBANK0, true);
    initOutput<25>(); // config LED
}
} // namespace sys

} // namespace rp2350
