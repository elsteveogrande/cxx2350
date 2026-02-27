#pragma once

#include <platform.h>
#include <rp2350/common.h>
#include <rp2350/resets.h>

namespace rp2350 {

struct BusControl {
    struct Priority : R32 {
        unsigned proc0    : 1; // 0
        unsigned          : 3;
        unsigned proc1    : 1; // 4
        unsigned          : 3;
        unsigned dmaRead  : 1; // 8
        unsigned          : 3;
        unsigned dmaWrite : 1; // 12
        unsigned          : 19;
    };

    Priority priority; // 0x00
};
inline auto& busControl = *(BusControl*)(0x40068000);

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void initBusControl(bool prioritizeDMA = true) {
    resets.unreset(Resets::Bit::BUSCTRL, true);
    busControl.priority.dmaRead = prioritizeDMA;
}

} // namespace rp2350
