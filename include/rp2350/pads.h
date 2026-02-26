#pragma once

#include <platform.h>

namespace rp2350 {

struct PadsBank0 {
    struct Voltage {
        enum class Select {
            k3v3 = 0,
            k1v8 = 1,
        };
        Select select : 1; // 0
        unsigned      : 31;
    };

    enum class Drive : unsigned {
        k2mA  = 0,
        k4mA  = 1,
        k8mA  = 2,
        k12mA = 3,
    };

    struct GPIO {
        unsigned slewFast      : 1; // 0
        unsigned schmitt       : 1; // 1
        unsigned pullDown      : 1; // 2
        unsigned pullUp        : 1; // 3
        Drive drive            : 2; // 5..4
        unsigned inputEnable   : 1; // 6
        unsigned outputDisable : 1; // 7
        unsigned isolation     : 1; // 8
        unsigned               : 23;
    };

    Voltage voltage; // 0x40038000
    GPIO gpio[48];   // 0x40038004...
    uint32_t swclk;  // 0x400380c4
    uint32_t swd;    // 0x400380c8
};
inline auto& padsBank0 = *(PadsBank0*)(0x40038000);

} // namespace rp2350
