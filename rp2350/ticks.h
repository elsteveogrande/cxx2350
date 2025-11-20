#pragma once
#include "base.h"

namespace rp2 {

// Section 8.5
struct Ticks {
    struct Control {
        uvint enabled : 1;  // 0
        uvint running : 1;  // 1
        uint          : 30; // 31..2
    };

    struct Cycles {
        uvint count : 8;  // 7..0
        uint        : 24; // 31..24
    };

    struct Count {
        uvint count : 8;  // 7..0
        uint        : 24; // 31..24
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
inline auto& ticks = *(Ticks*)(0x40108000);

} // namespace rp2
