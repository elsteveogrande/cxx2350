#pragma once

#include <cxx20/cxxabi.h>
#include <rp2350/common.h>
#include <rp2350/insns.h>
#include <rp2350/resets.h>

// Covers a few system-clock-related components: XOSC, PLL, and peripherals' clocks.

namespace rp2350 {

namespace sys {

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
        Enable enable       : 12; // 23..12
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
        enum class Code : uint32_t {
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
    Status status;
    Dormant dormant;
    Startup startup;
    uint32_t count;

    void init() {
        control.freqRange = Control::FreqRange::kFreq1to15;
        startup           = {.delay = 500, .x4 = 1}; // around 40-50ms
        control.enable    = Control::Enable::kEnable;
        dormant.code      = Dormant::Code::kWake;
        while (!(status.stable)) { rp2350::sys::nop(); }
    }
};
inline auto& xosc = *(XOSC*)(0x40048000);

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
    PowerDown powerDown;
    uint32_t fbDiv;
    Primary prim;
    uint32_t intr; // TODO
    uint32_t inte; // TODO
    uint32_t intf; // TODO
    uint32_t ints; // TODO

    // Section 8.6, PLL, p583 describes the `pll_init` process
    void init(unsigned fbDiv_, unsigned div1, unsigned div2) {
        resets.reset(Resets::Bit::PLLSYS); // includes powering down
        resets.unreset(Resets::Bit::PLLSYS);
        cs.bypass       = false;
        cs.refDiv       = 1;
        fbDiv           = fbDiv_;
        powerDown.pd    = false;
        powerDown.vcoPD = false;
        while (!cs.lock) { rp2350::sys::nop(); } // wait for LOCK

        prim.postDiv1       = div1;
        prim.postDiv2       = div2;
        powerDown.postdivPD = false;
    }
};
inline auto& sysPLL = *(PLL*)(0x40050000);

} // namespace sys

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
        Div div;
        uint32_t selected;
    };

    struct Ref {
        enum class AuxSource : unsigned {
            PLL_USB              = 0,
            PLL_GPIN0            = 1,
            PLL_GPIN1            = 2,
            PLL_USB_PRI_REF_OPCG = 3,
        };

        enum class Source : unsigned {
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

        Control control;
        Div div;
        Selected selected;
    };

    struct Sys {
        enum class AuxSource : unsigned {
            PLL_SYS   = 0,
            PLL_USB   = 1,
            ROSC      = 2,
            XOSC      = 3,
            PLL_GPIN0 = 4,
            PLL_GPIN1 = 5,
        };

        enum class Source : unsigned {
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

        Control control;
        Div div;
        Selected selected;
    };

    struct Peri {
        enum class AuxSource : unsigned {
            CLK_SYS = 0,
            PLL_SYS = 1,
            PLL_USB = 2,
            ROSC_PH = 3,
            XOSC    = 4,
            GPIN0   = 5,
            GPIN1   = 6,
        };

        struct Control {
            unsigned            : 5;
            AuxSource auxSource : 3; // 7..5
            unsigned            : 2;
            unsigned kill       : 1;    // 10
            unsigned enable     : 1;    // 11
            unsigned            : 16;   // 27..12
            unsigned enabled    : 1 {}; // 28
            unsigned            : 3;
        };

        Control control;
        Div div;
        uint32_t selected;
    };

    struct HSTX {
        enum class AuxSource {
            CLK_SYS = 0,
            PLL_SYS = 1,
            PLL_USB = 2,
            GPIN0   = 3,
            GPIN1   = 4,
        };

        struct Control {
            unsigned            : 5;    //
            AuxSource auxSource : 3;    // 7..5
            unsigned            : 2;    //
            unsigned kill       : 1;    // 10
            unsigned enable     : 1;    // 11
            unsigned            : 4;    //
            unsigned phase      : 2;    // 17..16
            unsigned            : 2;    //
            unsigned nudge      : 1 {}; // 20
            unsigned            : 7;    //
            unsigned enabled    : 1 {}; // 28
            unsigned            : 3;    //
        };

        Control control;
        Div div;
        uint32_t selected;
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
    Ref ref;      // 0x40010030
    Sys sys;      // 0x4001003c
    Peri peri;    // 0x40010048
    HSTX hstx;    // 0x40010054
    // TODO all the others
};
inline auto& clocks = *(Clocks*)(0x40010000);

namespace sys {
inline void initSystemClock(unsigned fbDiv, unsigned div1, unsigned div2) {
    sys::xosc.init();
    sys::sysPLL.init(fbDiv, div1, div2);

    clocks.sys.control = {.source    = Clocks::Sys::Source::CLK_SYS_AUX,
                          .auxSource = Clocks::Sys::AuxSource::PLL_SYS};
    clocks.sys.div     = {.fraction = 0, .integer = 1};
}

inline void initRefClock() {
    clocks.ref.control = {.source = Clocks::Ref::Source::XOSC, .auxSource = {}};
    clocks.ref.div     = {.fraction = 0, .integer = 1};
}

inline void initPeriphClock() {
    clocks.peri.control = {
        .auxSource = Clocks::Peri::AuxSource::PLL_SYS, .kill = false, .enable = true};
    clocks.peri.div = {.fraction = 0, .integer = 1};
}

inline void initHSTXClock() {
    update(&clocks.hstx.control, [](auto& _) {
        _.zero();
        _->auxSource = Clocks::HSTX::AuxSource::CLK_SYS;
        _->kill      = false;
        _->enable    = true;
    });
    clocks.hstx.div = {.fraction = 0, .integer = 1};
}
} // namespace sys

} // namespace rp2350
