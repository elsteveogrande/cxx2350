#pragma once
#include "base.h"

namespace rp2 {

// GPIO and SIO

// Section 9, GPIO
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
//
// These are for GPIO bank 0 (GPIOs 0 through 31) only.
struct GPIO {
    struct Status {
        uint            : 9; // 8..0
        uvint outToPad  : 1; // 9: output signal to pad after register override is applied
        uint            : 3; // 12..10
        uvint oeToPad   : 1; // 13: output enable to pad after register override is applied
        uint            : 3; // 16..14
        uvint inFromPad : 1; // 17: input signal from pad, before override is applied
        uint            : 8; // 25..18
        uvint irqToProc : 1; // 26: interrupt to processors, after override is applied
        uint            : 5; // 31..27
    };

    enum class Override {
        kNormal  = 0,
        kInvert  = 1,
        kLow     = 2,
        kDisable = 2,
        kHigh    = 3,
        kEnable  = 3,
    };

    // 9.4. Function Select; note that the correct funcsel depends on the GPIO number;
    // e.g. the value 10 might be either USBVBUS or USBOVRCUR; UART might be 2 or 11; and so on.
    enum class FuncSel {
        HSTX      = 0,
        SPI       = 1,
        UART2     = 2,
        I2C       = 3,
        PWM       = 4,
        SIO       = 5,
        PIO0      = 6,
        PIO1      = 7,
        PIO2      = 8,
        TRACEDATA = 9,
        QMI       = 9,
        CLOCK     = 9,
        USBVBUS   = 10,
        USBOVRCUR = 10,
        UART11    = 11,
        NULL_FN   = 31,
    };

    struct Control {
        FuncSel funcSel : 5;  // 4..0
        uint            : 7;  // 11..5
        uvint outOver   : 2;  // 13..12
        uvint oeOver    : 2;  // 15..14
        uvint inOver    : 2;  // 17..16
        uint            : 10; // 27..18
        uvint irqOver   : 2;  // 29..28
        uint            : 2;  // 31..30
    };

    struct GPIORegPair {
        Status  status;
        Control control;
    };

    GPIORegPair  array[32];
    GPIORegPair& operator[](uint i) { return array[i]; }
};
inline auto& gpio = *(GPIO*)(0x40028000);

// Section 9.8, Processor GPIO Controls (SIO)
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
//
// Note that for simplicity this only supports Bank0.
// We'll assume Bank1 is not dealt with; the pins in Bank1
// will then continue to be reserved only for QSPI.
struct SIO {
    uvint cpuID;         // 0xd0000000
    uvint gpioIn;        // 0xd0000004
    uvint z_008;         // 0xd0000008
    uvint z_00c;         // 0xd000000c
    uvint gpioOut;       // 0xd0000010
    uvint z_014;         // 0xd0000014
    uvint gpioOutSet;    // 0xd0000018
    uvint z_01c;         // 0xd000001c
    uvint gpioOutClr;    // 0xd0000020
    uvint z_0;           // 0xd0000024
    uvint gpioOutXor;    // 0xd0000028
    uvint z_02c;         // 0xd000002c
    uvint gpioOutEnb;    // 0xd0000030
    uvint z_034;         // 0xd0000034
    uvint gpioOutEnbSet; // 0xd0000038
    uvint z_03c;         // 0xd000003c
    uvint gpioOutEnbClr; // 0xd0000040
    uvint z_044;         // 0xd0000044
    uvint gpioOutEnbXor; // 0xd0000048

    // TODO:
    // fifo(Status,Write)
    // spinlockState
    // 0x080 - 0x0bc , INTERP0
    // 0x0c0 - 0x0fc , INTERP1
    // 0x100 - 0x17c , SPINLOCKn
    // lots more
};
inline auto& sio = *(SIO*)(0xd0000000);

} // namespace rp2
