#pragma once

#include <platform.h>
#include <rp2350/m33.h>

namespace rp2350 {

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
        Cycles cycles;
        Count count;
    };

    TickRegs proc0;
    TickRegs proc1;
    TickRegs timer0;
    TickRegs timer1;
    TickRegs watchdog;
    TickRegs riscv;
};
inline auto& ticks = *(Ticks*)(0x40108000);

inline void initSystemTicks() {
    // p569: SDK as well as Arm CPU expect nominal 1uS system ticks
    ticks.proc0.control.enabled = false;
    ticks.proc0.cycles.count    = 12;
    ticks.proc0.control.enabled = true;
    ticks.proc1.control.enabled = false;
    ticks.proc1.cycles.count    = 12;
    ticks.proc1.control.enabled = true;

    m33.rvr()         = 1000;
    m33.csr().enable  = 1;
    m33.csr().tickInt = 1;
}

} // namespace rp2350
