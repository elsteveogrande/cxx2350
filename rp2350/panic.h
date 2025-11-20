#pragma once
#include "base.h"

namespace rp2350::sys {
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

} // namespace rp2350::sys
