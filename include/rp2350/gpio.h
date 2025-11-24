#pragma once

namespace rp2350 {

// GPIO and SIO

// Section 9, GPIO
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
//
// These are for GPIO bank 0 (GPIOs 0 through 31) only.
struct GPIO {
    struct Status {
        unsigned           : 9; // 8..0
        unsigned outToPad  : 1; // 9: output signal to pad after register override is applied
        unsigned           : 3; // 12..10
        unsigned oeToPad   : 1; // 13: output enable to pad after register override is applied
        unsigned           : 3; // 16..14
        unsigned inFromPad : 1; // 17: input signal from pad, before override is applied
        unsigned           : 8; // 25..18
        unsigned irqToProc : 1; // 26: interrupt to processors, after override is applied
        unsigned           : 5; // 31..27
    };

    enum class Override {
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
    template <> struct FuncSel< 0> { constexpr static unsigned SPI0RX=1, UART0TX=2, I2C0SDA=3, PWM0A=4, SIO=5, PIO0=6, PIO1=7, PIO2=8, QMICS1n=9, USBOVRCURDET=10;};
    template <> struct FuncSel< 1> { constexpr static unsigned SPI0CSn=1, UART0RX=2, I2C0SCL=3, PWM0B=4, SIO=5, PIO0=6, PIO1=7, PIO2=8, TRACECLK=9, USBVBUSDET=10;};
    template <> struct FuncSel<25> { constexpr static unsigned SPI0CSn=1, UART0RX=2, I2C0SCL=3, PWM4B=4, SIO=5, PIO0=6, PIO1=7, PIO2=8, CLKGPOUT3=9, USBVBUSDET=10;};
    // clang-format on

    struct Control {
        unsigned funcSel : 5;  // 4..0
        unsigned         : 7;  // 11..5
        unsigned outOver : 2;  // 13..12
        unsigned oeOver  : 2;  // 15..14
        unsigned inOver  : 2;  // 17..16
        unsigned         : 10; // 27..18
        unsigned irqOver : 2;  // 29..28
        unsigned         : 2;  // 31..30
    };

    struct GPIORegPair {
        Status  status;
        Control control;
    };

    GPIORegPair  array[32];
    GPIORegPair& operator[](unsigned i) { return array[i]; }
};
inline auto& gpio = *(GPIO*)(0x40028000);

// Section 9.8, Processor GPIO Controls (SIO)
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
//
// Note that for simplicity this only supports Bank0.
// We'll assume Bank1 is not dealt with; the pins in Bank1
// will then continue to be reserved only for QSPI.
struct SIO {
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

} // namespace rp2350
