#pragma once

#include <cxx20/cxxabi.h>

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

} // namespace rp2350
