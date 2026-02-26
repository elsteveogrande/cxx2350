#pragma once

#include <platform.h>

namespace rp2350 {

[[gnu::always_inline]]
inline void __nop() {
    asm volatile("nop" : : : "memory");
}

[[gnu::always_inline]]
inline void __breakpoint() {
    asm volatile("bkpt 0" : : : "memory");
}

[[gnu::always_inline]]
inline void __wfi() {
    asm volatile("wfi");
}

[[gnu::always_inline]]
inline void __wfe() {
    asm volatile("wfe");
}

[[gnu::always_inline]]
inline void __cpsid() {
    asm volatile("cpsid i");
}

[[gnu::always_inline]]
inline void __cpsie() {
    asm volatile("cpsie i");
}

[[gnu::always_inline]]
inline uint32_t __ipsr() {
    uint32_t ret;
    asm volatile("mrs %0, ipsr" : "=r"(ret));
    return ret;
}

inline void __disableIRQs() { __cpsid(); }
inline void __enableIRQs() { __cpsie(); }

inline unsigned __currentInterrupt() {
    // Bits 0..8: Interrupt number (IRQs start at 16)
    return unsigned(__ipsr() & 0x1ff);
}

} // namespace rp2350
