#pragma once

#include <cxx20/cxxabi.h>

template <uint16_t kWidth, uint16_t kHeight> struct HSTXPixels {
    constexpr uint16_t width() const { return kWidth; }
    constexpr uint16_t height() const { return kHeight; }

    struct Pixels {
        unsigned count : 12;    // 11..0
        unsigned cmd   : 4 {2}; // 15..12
        unsigned       : 16;
    };
    static_assert(sizeof(Pixels) == sizeof(uint32_t));

    struct Repeat {
        unsigned count : 12;    // 11..0
        unsigned cmd   : 4 {2}; // 15..12
        unsigned       : 16;
    };
    static_assert(sizeof(Repeat) == sizeof(uint32_t));

    struct Nop {
        // These mark line boundaries; one Nop immediately after the end of each row.
        // Also used to pad out the end of very short (when compressed) lines.
        unsigned count : 12 {0};   // 11..0
        unsigned cmd   : 4 {0x0f}; // 15..12
        unsigned       : 16;
    };

    size_t size;
    uint32_t words[1]; // actual size is `size`; is always > 0
};
