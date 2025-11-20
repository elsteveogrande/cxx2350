#pragma once

/*
https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
https://developer.arm.com/documentation/100235/0100/The-Cortex-M33-Processor/Exception-model/Vector-table
*/

namespace rp2 {
using u8    = unsigned char;
using u16   = unsigned short;
using u32   = unsigned long;
using uv32  = u32 volatile;
using uint  = u32;
using uvint = uint volatile;
using uptr  = u32*;
typedef void (*vfunc)();
} // namespace rp2

extern "C" {
// Defined in the linker script
extern rp2::uptr __vec_table;
extern rp2::uptr __reset_sp;
extern rp2::uptr __init_array_base;
extern rp2::uptr __init_array_end;
extern rp2::uptr __bss_base;
extern rp2::uptr __bss_end;
extern rp2::uptr __heap_base;
extern rp2::uptr __heap_end;
extern rp2::uptr __buf_x_base;
extern rp2::uptr __buf_x_end;
extern rp2::uptr __buf_y_base;
extern rp2::uptr __buf_y_end;

extern "C" {
// C/C++ ABI-specified functions

inline void __aeabi_memcpy(rp2::u8* dest, rp2::u8 const* src, rp2::u32 n) {
    for (rp2::u32 i = 0; i < n; i++) {
        // Read from source and write into dest; the do-nothing `asm volatile`
        // is only to separate the read and write, to prevent fusing them and optimizing
        // into a "memcpy" operation involving a call to `__aeabi_memcpy`, the very thing
        // we're trying to define.
        rp2::u8 x = src[i];
        // asm volatile("");  // XXX actually needed??
        dest[i]   = x;
    }
}

inline void __aeabi_memcpy4(rp2::u8* dest, rp2::u8 const* src, unsigned n) {
    __aeabi_memcpy(dest, src, n);
}
}

void _start();
} // extern "C"

namespace rp2 {

constexpr uint const kSysPLLMHz = 150000000;

namespace sys {

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
    u32 ipsr() {
        u32 ret;
        asm volatile("mrs %0, ipsr" : "=r"(ret));
        return ret;
    }
};
struct Insns : ARMInsns {
    constexpr Insns() = default;

    void disableIRQs() { cpsid(); }
    void enableIRQs() { cpsie(); }

    uint currentInterrupt() {
        // Bits 0..8: Interrupt number (IRQs start at 16)
        return uint(ipsr() & 0x1ff);
    }
};
#endif

[[gnu::always_inline]]
inline void nop() {
    Insns {}.nop();
}

struct PanicContext {
    u32 sp;
    u32 r4;
    u32 r5;
    u32 r6;
    u32 r7;
    u32 r8;
    u32 r9;
    u32 r10;
    u32 r11;
    u32 exc;
    u32 type;
    u32 r0;
    u32 r1;
    u32 r2;
    u32 r3;
    u32 r12;
    u32 lr;
    u32 pc;
    u32 psr;
};

[[gnu::used]] [[gnu::noinline]] [[noreturn]]
inline void defaultPanic(PanicContext const& cx) {
    (void)cx;
    while (1) {}

    //   m33::M33 m33;

    //   // Disable all interrupts immediately
    //   // m33.nvic.cer0.set(31, 0, 0xffffffff);

    //   constexpr static char const* RED    = "\x1b[0;41;1;37m";
    //   constexpr static char const* YEL    = "\x1b[1;33;48;5;236m";
    //   constexpr static char const* NORMAL = "\x1b[0m";

    //   PanicTX tx;
    //   for (u32 i = 0; i < 16; i++) { tx << '\n'; }

    //   auto& rd = runtimeData;

    //   tx << RED << "=== panic @ " << rd.millis << NORMAL << "ms\n";
    //   tx << "  r0:" << pd.r0 << "  r1:" << pd.r1 << "  r2:" << pd.r2 << "  r3:" << pd.r3 << '\n'
    //      << "  r4:" << pd.r4 << "  r5:" << pd.r5 << "  r6:" << pd.r6 << "  r7:" << pd.r7 << '\n'
    //      << "  r8:" << pd.r8 << "  r9:" << pd.r9 << " r10:" << pd.r10 << " r11:" << pd.r11 <<
    //      '\n'
    //      << " r12:" << pd.r12 << "  sp:" << pd.sp << "  lr:" << pd.lr << "  pc:" << pd.pc << '\n'
    //      << " psr:" << pd.psr << " exc:" << pd.exc << "  type:" << pd.type << '\n';

    //   tx << "\nM33 SCB:\n";
    //   tx << "  ACTLR:" << m33.actlr.val();
    //   tx << "  CPUID:" << m33.cpuid.val();
    //   tx << "   ICSR:" << m33.icsr.val();
    //   tx << "   VTOR:" << m33.vtor.val();
    //   tx << "  AIRCR:" << m33.aircr.val() << '\n';
    //   tx << "    SCR:" << m33.scr.val();
    //   tx << "    CCR:" << m33.ccr.val();
    //   tx << "  SHPR1:" << m33.shpr1.val();
    //   tx << "  SHPR2:" << m33.shpr2.val();
    //   tx << "  SHPR3:" << m33.shpr3.val() << '\n';
    //   tx << "  SHCSR:" << m33.shcsr.val();
    //   tx << "  CPACR:" << m33.cpacr.val();
    //   tx << "  NSACR:" << m33.nsacr.val() << '\n';
    //   tx << YEL;
    //   u32 sr;

    //   sr = m33.hfsr.val();
    //   tx << "   HFSR:" << sr;
    //   if (sr & 0x80000000) { tx << " DEBUGEVT"; }
    //   if (sr & 0x40000000) { tx << " FORCED"; }
    //   if (sr & 0x00000002) { tx << " VECTTBL"; }
    //   tx << '\n';

    //   auto cfsr = m33.cfsr.val();
    //   sr        = cfsr & 0xff;
    //   tx << "   MFSR:" << sr;
    //   if (!(sr & 0x80)) { tx << " !MMARVALID"; }
    //   if (sr & 0x20) { tx << " MLSPERR"; }
    //   if (sr & 0x10) { tx << " MSTKERR"; }
    //   if (sr & 0x08) { tx << " MUNSTKERR"; }
    //   if (sr & 0x02) { tx << " DACCVIOL"; }
    //   if (sr & 0x01) { tx << " IACCVIOL"; }
    //   tx << '\n';
    //   if (sr ^ 0x80) { tx << "  MMFAR:" << m33.mmfar.get(31, 0) << '\n'; }

    //   sr = (cfsr >> 8) & 0xff;
    //   tx << "   BFSR:" << sr;
    //   if (!(sr & 0x80)) { tx << " !BFARVALID"; }
    //   if (sr & 0x20) { tx << " LSPERR"; }
    //   if (sr & 0x10) { tx << " STKERR"; }
    //   if (sr & 0x08) { tx << " UNSTKERR"; }
    //   if (sr & 0x02) { tx << " PRECISERR"; }
    //   if (sr & 0x01) { tx << " IBUSERR"; }
    //   tx << '\n';
    //   if (sr ^ 0x80) { tx << "   BFAR:" << m33.bfar.get(31, 0) << '\n'; }

    //   sr = cfsr >> 16;
    //   tx << "   UFSR:" << sr;
    //   if (sr & 0x0200) { tx << " DIVBYZERO"; }
    //   if (sr & 0x0100) { tx << " UNALIGNED"; }
    //   if (sr & 0x0010) { tx << " STKOF"; }
    //   if (sr & 0x0008) { tx << " NOCP"; }
    //   if (sr & 0x0004) { tx << " INVPC"; }
    //   if (sr & 0x0002) { tx << " INVSTATE"; }
    //   if (sr & 0x0001) { tx << " UNDEFINSTR"; }
    //   tx << NORMAL;
    //   tx << '\n';

    //   Reg32& bork = *(Reg32*)(u32(0x400d800c));
    //   if (bork.val()) { tx << RED << "BORK " << bork.val() << NORMAL << '\n'; }

    //   SIO sio;
    //   u32 i = 0;
    //   while (true) { sio.gpioOut.bit(kPicoLED, (i++ >> 21) & 1); };

    //   __builtin_unreachable();
}

inline void reset() {
    // Clear BSS
    auto* bss    = reinterpret_cast<u32*>(__bss_base);
    auto* bssEnd = reinterpret_cast<u32*>(__bss_end);
    for (; bss < bssEnd; ++bss) { *bss = 0; }

    // Clear words just past our stack frame, to avoid "corrupt stack?" in gcc
    // auto* stackTop = (u32*)(__reset_sp);
    // stackTop[0]    = 0;
    // stackTop[1]    = 0;

    // Run static initializers
    auto** initArray    = (vfunc**)__init_array_base;
    auto** initArrayEnd = (vfunc**)__init_array_end;
    for (; initArray < initArrayEnd; ++initArray) { (**initArray)(); }

    // Call user's entry function
    _start();
}

// Defined in `Faults.s`:
extern void hardFault();
extern void memManage();
extern void busFault();
extern void usageFault();

[[gnu::weak]]
inline void nmi() {}

[[gnu::weak]]
inline void svCall() {}

[[gnu::weak]]
inline void debugMon() {}

[[gnu::weak]]
inline void pendingSV() {}

inline void sysTick() {
    // TODO: a 64-bit counter in hardware for sys time
}

constexpr uint const kIRQHandlers = 4;

// Initially null.  Can add more; update `ARMVectors` if increasing
inline vfunc irqHandlers[kIRQHandlers] {};

inline void irq() {
    auto intn = Insns().ipsr() & 0x1ff;
    if (intn >= 16 && intn < 16 + kIRQHandlers) {
        auto irq = intn - 16;
        if (irqHandlers[irq]) { irqHandlers[irq](); }
    } else {
        hardFault();
    }
}

struct ARMVectors {
    uint resetSP         = 0x20000c00;               // 0 (__reset_sp)
    void (*reset)()      = (::rp2::sys::reset);      // 1
    void (*nmi)()        = (::rp2::sys::nmi);        // 2
    void (*hardFault)()  = (::rp2::sys::hardFault);  // 3
    void (*memManage)()  = (::rp2::sys::memManage);  // 4
    void (*busFault)()   = (::rp2::sys::busFault);   // 5
    void (*usageFault)() = (::rp2::sys::usageFault); // 6
    void (*_unknown07)() = (nullptr);                // 7
    void (*_unknown08)() = (nullptr);                // 8
    void (*_unknown09)() = (nullptr);                // 9
    void (*_unknown0A)() = (nullptr);                // 10
    void (*svCall)()     = (::rp2::sys::svCall);     // 11
    void (*debugMon)()   = (::rp2::sys::debugMon);   // 12
    void (*_unknown0B)() = (nullptr);                // 13
    void (*pendingSV)()  = (::rp2::sys::pendingSV);  // 14
    void (*sysTick)()    = (::rp2::sys::sysTick);    // 15
    void (*irq00)()      = (::rp2::sys::irq);
    void (*irq01)()      = (::rp2::sys::irq);
    void (*irq02)()      = (::rp2::sys::irq);
    void (*irq03)()      = (::rp2::sys::irq);
};

// Image definition [IMAGE_DEF]: section 5.9, "Metadata Block Details".

struct [[gnu::packed]] ImageDef2350ARM {
    u32 start     = 0xffffded3; // Start magic
    u8  type      = 0x42;       //
    u8  size      = 0x01;       //
    u16 flags     = 0x1021;     // Item 0: CHIP=2350, CPU=ARM, EXE=1, S=2
    u8  sizeType  = 0xff;       // BLOCK_ITEM_LAST has a 2-byte size
    u16 totalSize = 1;          // Total of preceding items' sizes (in words)
    u8  _pad      = 0;          //
    u32 link      = 0;          // No link since this is only 1 block
    u32 end       = 0xab123579; // End magic
};

} // namespace sys
} // namespace rp2
