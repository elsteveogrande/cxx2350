#pragma once
#include "common.h"
#include "insns.h"
#include "resets.h"

namespace rp2 {

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
inline auto& sysPLL = *(PLL*)(0x40050000);

} // namespace rp2
