#pragma once
#include "base.h"

namespace rp2350 {

struct PadsBank0 {
    struct Voltage {
        enum class Select {
            k3v3 = 0,
            k1v8 = 1,
        };
        Select select : 1; // 0
        uint          : 31;
    };

    enum class Drive : uint {
        k2mA  = 0,
        k4mA  = 1,
        k8mA  = 2,
        k12mA = 3,
    };

    struct GPIO {
        uvint slewFast      : 1 {}; // 0
        uvint schmitt       : 1 {}; // 1
        uvint pullDown      : 1 {}; // 2
        uvint pullUp        : 1 {}; // 3
        Drive drive         : 2 {}; // 5..4
        uvint inputEnable   : 1 {}; // 6
        uvint outputDisable : 1 {}; // 7
        uvint isolation     : 1 {}; // 8
        uint                : 23;
    };

    Voltage voltage;  // 0x40038000
    GPIO    gpio[48]; // 0x40038004...
    uv32    swclk;    // 0x400380c4
    uv32    swd;      // 0x400380c8
};
inline auto& padsBank0 = *(PadsBank0*)(0x40038000);

} // namespace rp2350
