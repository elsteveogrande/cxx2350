#pragma once

#include <cxx20/cxxabi.h>

namespace rp2350 {

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

        Control  control;
        Div      div;
        uint32_t selected;
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
        enum class AuxSource {
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
            unsigned kill       : 1;  // 10
            unsigned enable     : 1;  // 11
            unsigned            : 16; // 27..12
            unsigned enabled    : 1;  // 28
            unsigned            : 3;
        };

        Control  control;
        Div      div;
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
    Peri  peri;   // 0x40010048
    // TODO all the others
};
inline auto& clocks = *(Clocks*)(0x40010000);

} // namespace rp2350
