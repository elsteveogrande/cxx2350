#pragma once

#include <cxx20/cxxabi.h>
#include <rp2350/common.h>
#include <rp2350/insns.h>

namespace rp2350::sys {

inline void reset() {
    // Clear BSS
    auto* bss    = reinterpret_cast<uint32_t*>(&__bss_base);
    auto* bssEnd = reinterpret_cast<uint32_t*>(&__bss_end);
    for (; bss < bssEnd; ++bss) { *bss = 0; }

    // Run static initializers
    auto* initArray    = reinterpret_cast<vfunc*>(&__init_array_base);
    auto* initArrayEnd = reinterpret_cast<vfunc*>(&__init_array_end);
    for (; initArray < initArrayEnd; ++initArray) { (**initArray)(); }

    // Call user's entry function
    _start();
}

// Defined in `Faults.s`:
[[gnu::interrupt("IRQ")]]
extern void hardFault();

[[gnu::interrupt("IRQ")]]
extern void memManage();

[[gnu::interrupt("IRQ")]]
extern void busFault();

[[gnu::interrupt("IRQ")]]
extern void usageFault();

[[gnu::interrupt("IRQ")]] [[gnu::weak]]
inline void nmi() {}

[[gnu::interrupt("IRQ")]] [[gnu::weak]]
inline void svCall() {}

[[gnu::interrupt("IRQ")]] [[gnu::weak]]
inline void debugMon() {}

[[gnu::interrupt("IRQ")]] [[gnu::weak]]
inline void pendingSV() {}

[[gnu::interrupt("IRQ")]]
inline void sysTick() {
    // TODO: a 64-bit counter in hardware for sys time
}

constexpr unsigned const kIRQHandlers = 52;

inline vfunc irqHandlers[kIRQHandlers] {};

[[gnu::interrupt("IRQ")]]
inline void irq() {
    auto intn = Insns().ipsr() & 0x1ff;
    if (intn >= 16 && intn < 16 + kIRQHandlers) {
        auto irq = intn - 16;
        if (irqHandlers[irq]) { irqHandlers[irq](); }
    }
}

struct ARMVectors {
    unsigned resetSP     = 0x20000c00;                  // 0 (__reset_sp)
    void (*reset)()      = (::rp2350::sys::reset);      // 1
    void (*nmi)()        = (::rp2350::sys::nmi);        // 2
    void (*hardFault)()  = (::rp2350::sys::hardFault);  // 3
    void (*memManage)()  = (::rp2350::sys::memManage);  // 4
    void (*busFault)()   = (::rp2350::sys::busFault);   // 5
    void (*usageFault)() = (::rp2350::sys::usageFault); // 6
    void (*_unknown07)() = (nullptr);                   // 7
    void (*_unknown08)() = (nullptr);                   // 8
    void (*_unknown09)() = (nullptr);                   // 9
    void (*_unknown0A)() = (nullptr);                   // 10
    void (*svCall)()     = (::rp2350::sys::svCall);     // 11
    void (*debugMon)()   = (::rp2350::sys::debugMon);   // 12
    void (*_unknown0B)() = (nullptr);                   // 13
    void (*pendingSV)()  = (::rp2350::sys::pendingSV);  // 14
    void (*sysTick)()    = (::rp2350::sys::sysTick);    // 15
    void (*irqs[52])()   = {
        // 16 through 67
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
        ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq, ::rp2350::sys::irq,
    };
};

inline void initInterrupts() {
    // Initially, null out all IRQ handler entries
    memset(irqHandlers, 0, sizeof(irqHandlers));
    Insns {}.enableIRQs();
}

} // namespace rp2350::sys
