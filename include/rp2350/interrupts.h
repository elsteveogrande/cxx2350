#pragma once

#include <platform.h>
#include <rp2350/common.h>
#include <rp2350/insns.h>
#include <rp2350/m33.h>
#include <rp2350/reset.h>

namespace rp2350 {

// Defined in `Faults.s`:
[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void hardFault() {
    // TODO
    __builtin_trap();
}

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void memManage() {
    // TODO
    __builtin_trap();
}

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void busFault() {
    // TODO
    __builtin_trap();
}

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void usageFault() {
    // TODO
    __builtin_trap();
}

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void nmi() {
    // TODO
    __builtin_trap();
}

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void svCall() {
    // TODO
    __builtin_trap();
}

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void debugMon() {
    // TODO
    __builtin_trap();
}

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void pendingSV() {
    // TODO
    __builtin_trap();
}

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void sysTick() {
    // TODO: a 64-bit counter in hardware for sys time
}

constexpr unsigned const kIRQHandlers = 52;

inline vfunc irqHandlers[kIRQHandlers] {};

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void irq() {
    auto intn = __currentInterrupt();
    if (intn >= 16 && intn < 16 + kIRQHandlers) {
        auto irq = intn - 16;
        if (irqHandlers[irq]) { irqHandlers[irq](); }
        m33.clrPendIRQ(irq);
    }
}

struct [[gnu::aligned(512)]] ARMVectors {
    void* resetSP = __stack + 1024;                // 0
    void (*reset)() = __reset;                     // 1
    void (*nmi)() = (::rp2350::nmi);               // 2
    void (*hardFault)() = (::rp2350::hardFault);   // 3
    void (*memManage)() = (::rp2350::memManage);   // 4
    void (*busFault)() = (::rp2350::busFault);     // 5
    void (*usageFault)() = (::rp2350::usageFault); // 6
    void (*_unknown07)() = (nullptr);              // 7
    void (*_unknown08)() = (nullptr);              // 8
    void (*_unknown09)() = (nullptr);              // 9
    void (*_unknown0A)() = (nullptr);              // 10
    void (*svCall)() = (::rp2350::svCall);         // 11
    void (*debugMon)() = (::rp2350::debugMon);     // 12
    void (*_unknown0B)() = (nullptr);              // 13
    void (*pendingSV)() = (::rp2350::pendingSV);   // 14
    void (*sysTick)() = (::rp2350::sysTick);       // 15
    void (*irqs[52])() = {
        // 16 through 67
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq, ::rp2350::irq,
        ::rp2350::irq, ::rp2350::irq,
    };
};

[[gnu::retain]] [[gnu::used]] [[gnu::section(".sysdata")]]
inline ARMVectors __vectorTable;

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void initInterrupts() {
    __enableIRQs();
}

} // namespace rp2350
