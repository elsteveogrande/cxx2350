#include "old/Common.h"
#include <rp2350/rp2.h>

namespace rp2::sys {

[[gnu::used]] [[gnu::retain]] [[gnu::section(".vec_table")]] ARMVectors const gARMVectors {};

[[gnu::used]] [[gnu::retain]] [[gnu::section(
    ".image_def")]] constinit ImageDef2350ARM const gImageDef {};

} // namespace rp2::sys

namespace rp2 {

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
        FuncSel funcSel  : 5;  // 4..0
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
    GPIORegPair& operator[](uint i) { return array[i]; }
};
auto& gpio = *(GPIO*)(0x40028000);

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
auto& resets = *(Resets*)(0x40020000);

struct PadsBank0 {
    struct Voltage {
        enum class Select {
            k3v3 = 0,
            k1v8 = 1,
        };
        Select select : 1; // 0
        unsigned      : 31;
    };

    enum class Drive : uint {
        k2mA  = 0,
        k4mA  = 1,
        k8mA  = 2,
        k12mA = 3,
    };

    struct GPIO {
        unsigned slewFast      : 1 {}; // 0
        unsigned schmitt       : 1 {}; // 1
        unsigned pullDown      : 1 {}; // 2
        unsigned pullUp        : 1 {}; // 3
        Drive    drive         : 2 {}; // 5..4
        unsigned inputEnable   : 1 {}; // 6
        unsigned outputDisable : 1 {}; // 7
        unsigned isolation     : 1 {}; // 8
        unsigned               : 23;
    };

    Voltage voltage;  // 0x40038000
    GPIO    gpio[48]; // 0x40038004...
    uv32    swclk;    // 0x400380c4
    uv32    swd;      // 0x400380c8
};
auto& padsBank0 = *(PadsBank0*)(0x40038000);

// Section 9.8, Processor GPIO Controls (SIO)
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
//
// Note that for simplicity this only supports Bank0.
// We'll assume Bank1 is not dealt with; the pins in Bank1
// will then continue to be reserved only for QSPI.
struct SIO {
    uvint cpuID;         // 0xd0000000
    uvint gpioIn;        // 0xd0000004
    uvint z008;          // 0xd0000008
    uvint z00c;          // 0xd000000c
    uvint gpioOut;       // 0xd0000010
    uvint z014;          // 0xd0000014
    uvint gpioOutSet;    // 0xd0000018
    uvint z01c;          // 0xd000001c
    uvint gpioOutClr;    // 0xd0000020
    uvint z0;            // 0xd0000024
    uvint gpioOutXor;    // 0xd0000028
    uvint z02c;          // 0xd000002c
    uvint gpioOutEnb;    // 0xd0000030
    uvint z034;          // 0xd0000034
    uvint gpioOutEnbSet; // 0xd0000038
    uvint z03c;          // 0xd000003c
    uvint gpioOutEnbClr; // 0xd0000040
    uvint z044;          // 0xd0000044
    uvint gpioOutEnbXor; // 0xd0000048

    // TODO:
    // fifo(Status,Write)
    // spinlockState
    // 0x080 - 0x0bc , INTERP0
    // 0x0c0 - 0x0fc , INTERP1
    // 0x100 - 0x17c , SPINLOCKn
    // lots more
};
auto& sio = *(SIO*)(0xd0000000);

// Chapter 8, Clocks
// https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
// see: p524 (8.1.6.1) Configuring
//      p530 (8.1.7) List of Registers
struct Clocks {
    struct Div {
        unsigned fraction : 16; // 15..0
        unsigned integer  : 16; // 31..16
    };

    struct GPOut {
        enum class AuxSource {
            PLL_SYS              = 0,
            GPIN0                = 1,
            GPIN1                = 2,
            PLL_USB              = 3,
            PLL_USB_PRI_REF_OPCG = 4, // ???
            ROSC                 = 5,
            XOSC                 = 6,
            LPOSC                = 7,
            CLK_SYS              = 8,
            CLK_USB              = 9,
            CLK_ADC              = 10,
            CLK_REF              = 11,
            CLK_PERI             = 12,
            CLK_HSTX             = 13,
            OTP_CLK2FC           = 14,
        };

        struct Control {
            unsigned            : 5; // 4..0
            AuxSource auxSource : 3; // 7..5
            unsigned            : 2; // 9..8
            unsigned kill       : 1; // 10
            unsigned enable     : 1; // 11
            unsigned dc50       : 1; // 12
            unsigned            : 3; // 15..13
            unsigned phase      : 2; // 17..16
            unsigned            : 2; // 19..18
            unsigned nudge      : 1; // 20
            unsigned            : 7; // 27..21
            unsigned enabled    : 1; // 28
            unsigned            : 3; // 31..29
        };

        Control control;
        Div     div;
        u32     selected;
    };

    struct Ref {
        enum class AuxSource {
            PLL_USB              = 0,
            PLL_GPIN0            = 1,
            PLL_GPIN1            = 2,
            PLL_USB_PRI_REF_OPCG = 3,
        };

        enum class Source {
            ROSC_PH     = 0,
            CLK_REF_AUX = 1,
            XOSC        = 2,
            LPOSC       = 3,
        };

        enum class Selected {
            ROSC_PH     = 1 << 0,
            CLK_REF_AUX = 1 << 1,
            XOSC        = 1 << 2,
            LPOSC       = 1 << 3,
        };

        struct Control {
            Source source       : 2;  // 1..0
            unsigned            : 3;  // 4..2
            AuxSource auxSource : 2;  // 6..5
            unsigned            : 25; // 31..7
        };

        Control  control;
        Div      div;
        Selected selected;
    };

    struct Sys {
        enum class AuxSource {
            PLL_SYS   = 0,
            PLL_USB   = 1,
            ROSC      = 2,
            XOSC      = 3,
            PLL_GPIN0 = 4,
            PLL_GPIN1 = 5,
        };

        enum class Source {
            CLK_REF     = 0,
            CLK_SYS_AUX = 1,
        };

        enum class Selected {
            CLK_REF     = 1 << 0,
            CLK_SYS_AUX = 1 << 1,
        };

        struct Control {
            Source source       : 1;  // 0
            unsigned            : 4;  // 4..1
            AuxSource auxSource : 3;  // 7..5
            unsigned            : 24; // 31..8
        };

        Control  control;
        Div      div;
        Selected selected;
    };

    struct Peri {
        enum class PeriSource {
            CLK_SYS = 0,
            PLL_SYS = 1,
            PLL_USB = 2,
            ROSC_PH = 3,
            XOSC    = 4,
            GPIN0   = 5,
            GPIN1   = 6,
        };
        // TODO
    };

    struct HSTX {
        enum class AuxSource {
            CLK_SYS = 0,
            PLL_SYS = 1,
            PLL_USB = 2,
            GPIN0   = 3,
            GPIN1   = 4,
        };
        // TODO
    };

    struct USB {
        enum class AuxSource {
            PLL_USB = 0,
            PLL_SYS = 1,
            ROSC_PH = 2,
            XOSC    = 3,
            GPIN0   = 4,
            GPIN1   = 5,
        };
        // TODO
    };

    struct ADC {
        enum class AuxSource {
            PLL_USB = 0,
            PLL_SYS = 1,
            ROSC_PH = 2,
            XOSC    = 3,
            GPIN0   = 4,
            GPIN1   = 5,
        };
        // TODO
    };

    GPOut gpOut0; // 0x40010000
    GPOut gpOut1; // 0x4001000c
    GPOut gpOut2; // 0x40010018
    GPOut gpOut3; // 0x40010024
    Ref   ref;    // 0x40010030
    Sys   sys;    // 0x4001003c
    // TODO all the others
};
auto& clocks = *(Clocks*)(0x40010000);

// Section 8.2, Crystal Oscillator (XOSC)
struct XOSC {
    struct Control {
        enum class FreqRange {
            kFreq1to15   = 0xaa0,
            kFreq10to30  = 0xaa1,
            kFreq25to60  = 0xaa2,
            kFreq40to100 = 0xaa3,
        };
        enum class Enable { kDisable = 0xd1e, kEnable = 0xfab };

        FreqRange freqRange : 12; // 11..0
        Enable    enable    : 12; // 23..12
        unsigned            : 8;
    };

    struct Status {
        unsigned freqRange : 2; // 1..0
        unsigned           : 10;
        unsigned enabled   : 1; // 12
        unsigned           : 11;
        unsigned badWrite  : 1; // 24
        unsigned           : 6;
        unsigned stable    : 1; // 31
    };

    struct Dormant {
        enum class Code : u32 {
            kWake    = 0x77616b65,
            // WARNING: stop the PLLs & setup the IRQ *before* selecting dormant mode
            kDormant = 0x636f6d61,
        };
        Code code;
    };

    struct Startup {
        unsigned delay : 14; // 13..0
        unsigned       : 6;
        unsigned x4    : 1; // 20
        unsigned       : 11;
    };

    Control control;
    Status  status;
    Dormant dormant;
    Startup startup;
    u32     count;

    void init() {
        control.freqRange = Control::FreqRange::kFreq1to15;
        startup           = {.delay = 500, .x4 = 1}; // around 40-50ms
        control.enable    = Control::Enable::kEnable;
        dormant.code      = Dormant::Code::kWake;
        while (!(status.stable)) { rp2::sys::nop(); }
    }
};
auto& xosc = *(XOSC*)(0x40048000);

// Section 8.6, PLL
struct PLL {
    struct ControlStat {
        unsigned refDiv : 6; // 5..0
        unsigned        : 2;
        unsigned bypass : 1;  // 8
        unsigned        : 21; //
        unsigned lockN  : 1;  // 30
        unsigned lock   : 1;  // 31
    };

    struct PowerDown {
        unsigned pd        : 1; // 0
        unsigned           : 1;
        unsigned dsmPD     : 1; // 2
        unsigned postdivPD : 1; // 3
        unsigned           : 1;
        unsigned vcoPD     : 1; // 5
        unsigned           : 26;
    };

    // "Controls the PLL post dividers for the primary output
    // (note: this PLL does not have a secondary output)"
    struct Primary {
        unsigned          : 12;
        unsigned postDiv2 : 3; // 14..12
        unsigned          : 1;
        unsigned postDiv1 : 3; // 18..16
        unsigned          : 12;
    };

    ControlStat cs;
    PowerDown   powerDown;
    u32         fbDiv;
    Primary     prim;
    u32         intr; // TODO
    u32         inte; // TODO
    u32         intf; // TODO
    u32         ints; // TODO

    // Section 8.6, PLL, p583 describes the `pll_init` process
    void init(u16 fbd, u8 div1, u8 div2, u8 refDiv = 1) {
        if (div1 < div2) { return init(fbd, div2, div1, refDiv); }

        resets.reset(Resets::Bit::PLLSYS); // includes powering down
        resets.unreset(Resets::Bit::PLLSYS);
        cs.bypass       = false;
        cs.refDiv       = refDiv;
        fbDiv           = fbd;
        powerDown.pd    = false;
        powerDown.vcoPD = false;
        while (!cs.lock) { rp2::sys::nop(); } // wait for LOCK

        prim.postDiv1       = div1;
        prim.postDiv2       = div2;
        powerDown.postdivPD = false;
    }

    void init150MHz() { init(125, 5, 2, 1); }
};
auto& sysPLL = *(PLL*)(0x40050000);

// Section 8.5
struct Ticks {
    struct Control {
        unsigned enabled : 1;  // 0
        unsigned running : 1;  // 1
        unsigned         : 30; // 31..2
    };

    struct Cycles {
        unsigned count : 8;  // 7..0
        unsigned       : 24; // 31..24
    };

    struct Count {
        unsigned count : 8;  // 7..0
        unsigned       : 24; // 31..24
    };

    struct TickRegs {
        Control control;
        Cycles  cycles;
        Count   count;
    };

    TickRegs proc0;
    TickRegs proc1;
    TickRegs timer0;
    TickRegs timer1;
    TickRegs watchdog;
    TickRegs riscv;
};
auto& ticks = *(Ticks*)(0x40108000);

// 3.7. Cortex-M33 Processor
struct M33 {
    struct SysTick {
        enum class ClockSource { EXT_REF_CLK = 0, PROC_CLK = 1 };
        struct CSR {
            unsigned enable  : 1; // 0
            unsigned tickInt : 1; // 1
            unsigned source  : 1; // 2
            unsigned         : 13;
            unsigned count   : 1; // 16
            unsigned         : 15;
        };

        CSR& csr = *(CSR*)(0xe000e010); // SysTick Control and Status Register
        u32& rvr = *(u32*)(0xe000e014); // SysTick Reload Value Register
    };

    struct NVIC {
        u32& ser0 = *(u32*)(0xe000e100); // Interrupt (0..31) Set Enable Registers
        u32& cer0 = *(u32*)(0xe000e180); // Interrupt (0..31) Clear Enable Registers
        u32& spr0 = *(u32*)(0xe000e200); // Interrupt (0..31) Set Pending Registers
        u32& cpr0 = *(u32*)(0xe000e280); // Interrupt (0..31) Clear Pending Registers

        void enableIRQ(u8 irq) { ser0 = u32(1) << irq; }
        void disableIRQ(u8 irq) { cer0 = u32(1) << irq; }
        void triggerIRQ(u8 irq) { spr0 = u32(1) << irq; }
        void clearPendIRQ(u8 irq) { cpr0 = u32(1) << irq; }
    };

    struct ACTLR {
        uvint v;
    };

    struct CPUID {
        uvint v;
    };

    struct ICSR : R32<ICSR> {
        bool  pendingNMI() { return bit(31); }
        auto& pendingNMIClear(bool v) { return bit(31, v); }

        bool  pendingSV() { return bit(30); }
        auto& pendingSVClear(bool v) { return bit(30, v); }

        bool  pendingSysTick() { return bit(28); }
        auto& pendingSysTickClear(bool v) { return bit(28, v); }
    };

    struct VTOR {
        // TODO
        uvint v;
    };
    struct AIRCR {
        // TODO
        uvint v;
    };
    struct SCR {
        // TODO
        uvint v;
    };

    struct CCR {
        unsigned               : 3;
        unsigned unalignedTrap : 1; // 3
        unsigned div0Trap      : 1; // 4
        unsigned               : 27;
    };

    struct SHPR1 {
        // TODO
        uvint v;
    };
    struct SHPR2 {
        // TODO
        uvint v;
    };
    struct SHPR3 {
        // TODO
        uvint v;
    };
    struct SHCSR {
        // TODO
        uvint v;
    };
    struct CFSR {
        // TODO
        uvint v;
    };
    struct HFSR {
        // TODO
        uvint v;
    };
    struct MMFAR {
        // TODO
        uvint v;
    };
    struct BFAR {
        // TODO
        uvint v;
    };
    struct CPACR {
        // TODO
        uvint v;
    };
    struct NSACR {
        // TODO
        uvint v;
    };

    SysTick sysTick;
    NVIC    nvic;
    ACTLR&  actlr = *(ACTLR*)(0xE000E008); // Auxiliary Control Register - Cortex-M33
    CPUID&  cpuid = *(CPUID*)(0xE000ED00); // CPUID Base Register
    ICSR&   icsr  = *(ICSR*)(0xE000ED04);  // Interrupt Control and State Register
    VTOR&   vtor  = *(VTOR*)(0xE000ED08);  // Vector Table Offset Register
    AIRCR&  aircr = *(AIRCR*)(0xE000ED0C); // Application Interrupt and Reset Control Register
    SCR&    scr   = *(SCR*)(0xE000ED10);   // System Control Register - Cortex-M33
    CCR&    ccr   = *(CCR*)(0xE000ED14);   // Configuration and Control Register
    SHPR1&  shpr1 = *(SHPR1*)(0xE000ED18); // System Handler Priority Register 1
    SHPR2&  shpr2 = *(SHPR2*)(0xE000ED1C); // System Handler Priority Register 2
    SHPR3&  shpr3 = *(SHPR3*)(0xE000ED20); // System Handler Priority Register 3
    SHCSR&  shcsr = *(SHCSR*)(0xE000ED24); // System Handler Control and State Register
    CFSR&   cfsr  = *(CFSR*)(0xE000ED28);  // Configurable Fault Status Register
    HFSR&   hfsr  = *(HFSR*)(0xE000ED2C);  // HardFault Status Register
    MMFAR&  mmfar = *(MMFAR*)(0xE000ED34); // MemManage Fault Address Register
    BFAR&   bfar  = *(BFAR*)(0xE000ED38);  // BusFault Address Register
    CPACR&  cpacr = *(CPACR*)(0xE000ED88); // Coprocessor Access Control Register
    NSACR&  nsacr = *(NSACR*)(0xE000ED8C); // Non-secure Access Control Register
};
M33 m33;

} // namespace rp2

using namespace rp2;

[[gnu::used]] [[gnu::retain]] [[gnu::noreturn]] [[gnu::noinline]] void _start() {
    xosc.init();
    sysPLL.init150MHz();

    clocks.sys.control.auxSource = Clocks::Sys::AuxSource::PLL_SYS;
    clocks.sys.control.source    = Clocks::Sys::Source::CLK_SYS_AUX;
    clocks.sys.div               = {.fraction = 0, .integer = 1};
    clocks.ref.control.source    = Clocks::Ref::Source::XOSC;
    clocks.ref.div               = {.fraction = 0, .integer = 1};

    // p569: SDK expects nominal 1uS system ticks, as does Arm internals.
    // Although we don't use the SDK we'll assume 1uS everywhere as well.
    ticks.proc0.control.enabled = false; // disable while configuring
    ticks.proc0.cycles.count    = 12;
    ticks.proc0.control.enabled = true;
    ticks.proc1.control.enabled = false; // disable while configuring
    ticks.proc1.cycles.count    = 12;
    ticks.proc1.control.enabled = true;

    m33.ccr.div0Trap      = true;
    m33.ccr.unalignedTrap = true;

    m33.sysTick.rvr         = 1000;
    m33.sysTick.csr.tickInt = 1;
    m33.sysTick.csr.enable  = 1;

    resets.unreset(Resets::Bit::PADSBANK0);
    padsBank0.gpio[25] = {.slewFast      = 1,
                          .drive         = PadsBank0::Drive::k12mA,
                          .inputEnable   = 0,
                          .outputDisable = 0,
                          .isolation     = 0};

    resets.unreset(Resets::Bit::IOBANK0);
    gpio[25].control.funcSel = GPIO::FuncSel::SIO;
    sio.gpioOutEnbSet        = (1 << 25);
    sio.gpioOutSet           = (1 << 25);

    while (true) {
        sio.gpioOutXor = (1 << 25);
        for (auto i = 0; i < 10000; i++) { sys::Insns().nop(); }
    }
}
