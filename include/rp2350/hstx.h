#pragma once

#include <platform.h>
#include <rp2350/clocks.h>
#include <rp2350/common.h>
#include <rp2350/gpio.h>
#include <rp2350/resets.h>

namespace rp2350 {

// 12.11. HSTX
struct HSTX {
    // 12.11.7. List of control registers [p.1209]

    struct CSR : R32 {
        unsigned enable       : 1 {}; // 0
        unsigned expandEnable : 1 {}; // 1
        unsigned              : 2;    //
        unsigned coupled      : 1 {}; // 4
        unsigned coupleSel    : 2 {}; // 6..5
        unsigned              : 1;    //
        unsigned shift        : 5 {}; // 12..8
        unsigned              : 3;    //
        unsigned nShifts      : 5 {}; // 20..16
        unsigned              : 3;    //
        unsigned clkPhase     : 4 {}; // 27..24
        unsigned clkDiv       : 4 {}; // 31..28
    };

    struct Bit : R32 {
        unsigned selectP : 5 {}; // 4..0
        unsigned         : 3;    //
        unsigned selectN : 5 {}; // 12..8
        unsigned         : 3;    //
        unsigned invert  : 1 {}; // 16
        unsigned clock   : 1 {}; // 17
        unsigned         : 14;   //
    };

    // p.1212
    struct ExShift : R32 {
        unsigned rawShift   : 5; // 4..0
        unsigned            : 3; //
        unsigned rawNShifts : 5; // 12..8
        unsigned            : 3; //
        unsigned encShift   : 5; // 20..16
        unsigned            : 3; //
        unsigned encNShifts : 5; // 28..24
        unsigned            : 3; //
    };

    // p.1212
    struct ExTMDS : R32 {
        unsigned l0Rot   : 5; // 4..0
        unsigned l0NBits : 3; // 7..5
        unsigned l1Rot   : 5; // 4..0
        unsigned l1NBits : 3; // 7..5
        unsigned l2Rot   : 5; // 4..0
        unsigned l2NBits : 3; // 7..5
        unsigned         : 8; //
    };

    CSR csr;
    Bit bits[8];
    ExShift expandShift;
    ExTMDS expandTMDS;

    // 12.11.8. List of FIFO registers [p.1213]

    struct Status {
        unsigned level       : 8;
        unsigned full        : 1;
        unsigned empty       : 1;
        unsigned wroteOnFull : 1; // Write 1 to this to clear
        unsigned             : 21;
    };

    struct FIFO {
        Status stat;
        uint32_t fifoWrite;
    };
    FIFO& fifo() { return *(FIFO*)(0x50600000); }
};
inline auto& hstx = *(HSTX*)(0x400c0000);

inline void initHSTX() {
    initHSTXClock();
    resets.unreset(Resets::Bit::HSTX, true);
    initOutput<12>(GPIO::FuncSel<12>::HSTX);
    initOutput<13>(GPIO::FuncSel<13>::HSTX);
    initOutput<14>(GPIO::FuncSel<14>::HSTX);
    initOutput<15>(GPIO::FuncSel<15>::HSTX);
    initOutput<16>(GPIO::FuncSel<16>::HSTX);
    initOutput<17>(GPIO::FuncSel<17>::HSTX);
    initOutput<18>(GPIO::FuncSel<18>::HSTX);
    initOutput<19>(GPIO::FuncSel<19>::HSTX);
}

} // namespace rp2350
