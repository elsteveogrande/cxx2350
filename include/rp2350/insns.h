#pragma once

#include <cxx20/cxxabi.h>

namespace rp2350::sys {

#if defined(__arm__)
struct ARMInsns {
    constexpr ARMInsns() = default;

    [[gnu::always_inline]]
    void nop() {
        asm volatile("nop" : : : "memory");
    }

    [[gnu::always_inline]]
    void wfi() {
        asm volatile("wfi");
    }

    [[gnu::always_inline]]
    void wfe() {
        asm volatile("wfe");
    }

    [[gnu::always_inline]]
    void cpsid() {
        asm volatile("cpsid i");
    }

    [[gnu::always_inline]]
    void cpsie() {
        asm volatile("cpsie i");
    }

    [[gnu::always_inline]]
    uint32_t ipsr() {
        uint32_t ret;
        asm volatile("mrs %0, ipsr" : "=r"(ret));
        return ret;
    }
};
struct Insns : ARMInsns {
    constexpr Insns() = default;

    void disableIRQs() { cpsid(); }
    void enableIRQs() { cpsie(); }

    unsigned currentInterrupt() {
        // Bits 0..8: Interrupt number (IRQs start at 16)
        return unsigned(ipsr() & 0x1ff);
    }
};
#endif

[[gnu::always_inline]]
inline void nop() {
    Insns {}.nop();
}

} // namespace rp2350::sys
