#pragma once

#include <examples/config.h>
#include <platform.h>
#include <rp2350/common.h>
#include <rp2350/gpio.h>
#include <rp2350/insns.h>
#include <rp2350/m33.h>
#include <rp2350/xoscpll.h>

namespace rp2350::sys {

struct PanicContext {
    uint32_t sp;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t exc;
    uint32_t type;
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
};

struct PanicTX {
    constexpr static unsigned kGPIO = 0;
    constexpr static unsigned kBaud = 19200;
    constexpr static unsigned kClocks = 12'000'000 / kBaud; // TODO un-hardcode

    static_assert(kGPIO < 32);

    PanicTX() {
        sio.gpioOutEnbSet = (1u << kGPIO);
        sio.gpioOutSet = (1u << kGPIO);

        update(&gpio[kGPIO].control, [](auto& u) {
            u.zero();
            u->funcSel = GPIO::FuncSel<kGPIO>::SIO;
        });

        update(&padsBank0.gpio[kGPIO], [](auto& u) {
            u->drive = PadsBank0::Drive::k12mA;
            u->inputEnable = false;
            u->outputDisable = false;
            u->isolation = false;
        });

        for (auto i = 0; i < 20; i++) { delay(); }
    }

    void delay() {
        xosc.count = kClocks;
        while (xosc.count) { __nop(); }
    }

    void signal(bool x) {
        (x ? sio.gpioOutSet : sio.gpioOutClr) = 1u << kGPIO;
        delay();
    }

    void txByte(uint8_t x) {
        // TX line will have already been high for some (sufficient) period.
        signal(0);
        signal(x & 0x01);
        signal(x & 0x02);
        signal(x & 0x04);
        signal(x & 0x08);
        signal(x & 0x10);
        signal(x & 0x20);
        signal(x & 0x40);
        signal(x & 0x80);
        signal(1);
    }

    friend PanicTX& operator<<(PanicTX& self, char c) {
        if (c == '\n') {
            self.txByte('\r');
            self.txByte('\n');
        } else {
            self.txByte(c);
        }
        return self;
    }

    friend PanicTX& operator<<(PanicTX& self, char const* s) {
        while (s && *s) { self << *(s++); }
        return self;
    }

    friend PanicTX& operator<<(PanicTX& self, uint32_t x) {
        constexpr static char const* kHex = "0123456789abcdef";
        self << kHex[(x >> 28) & 0x0f];
        self << kHex[(x >> 24) & 0x0f];
        self << kHex[(x >> 20) & 0x0f];
        self << kHex[(x >> 16) & 0x0f];
        self << kHex[(x >> 12) & 0x0f];
        self << kHex[(x >> 8) & 0x0f];
        self << kHex[(x >> 4) & 0x0f];
        self << kHex[(x >> 0) & 0x0f];
        return self;
    }
};

inline void delay1() {
    xosc.count = kXOSC / 1000;
    while (xosc.count) { __nop(); }
}

inline void delay(unsigned ms) {
    while (ms--) { delay1(); }
}

[[gnu::used]] [[gnu::noinline]] [[noreturn]]
inline void panic(PanicContext const& cx) {
    __disableIRQs();

    constexpr static char const* RED = "\x1b[0;41;1;37m";
    constexpr static char const* YEL = "\x1b[1;33;48;5;236m";
    constexpr static char const* NORMAL = "\x1b[0m";

    PanicTX tx;

    // Try to sync receiver
    for (uint32_t i = 0; i < 16; i++) {
        tx << '*';
        delay1();
    }

    while (true) {
        for (uint32_t i = 0; i < 16; i++) { tx << '\n'; }

        tx << RED << "=== panic" << NORMAL << "\n";
        tx << "  r0:" << cx.r0 << "  r1:" << cx.r1 << "  r2:" << cx.r2
           << "  r3:" << cx.r3 << "  r4:" << cx.r4 << "  r5:" << cx.r5 << '\n'
           << "  r6:" << cx.r6 << "  r7:" << cx.r7 << "  r8:" << cx.r8
           << "  r9:" << cx.r9 << " r10:" << cx.r10 << " r11:" << cx.r11 << '\n'
           << " r12:" << cx.r12 << "  sp:" << cx.sp << "  lr:" << cx.lr
           << "  pc:" << cx.pc << " psr:" << cx.psr << " exc:" << cx.exc << '\n'
           << " typ:" << cx.type << '\n';

        tx << "\n" << YEL << "M33 SCB:" << NORMAL << "\n";

        tx << "  ACTLR:" << m33.actlr().u32();
        tx << "  CPUID:" << m33.cpuid().u32();
        tx << "   ICSR:" << m33.icsr().u32();
        tx << "   VTOR:" << m33.vtor().u32();
        tx << "  AIRCR:" << m33.aircr().u32() << '\n';

        tx << "    SCR:" << m33.scr().u32();
        tx << "    CCR:" << m33.ccr().u32();
        tx << "  SHPR1:" << m33.shpr1().u32();
        tx << "  SHPR2:" << m33.shpr2().u32();
        tx << "  SHPR3:" << m33.shpr3().u32() << '\n';

        tx << "  SHCSR:" << m33.shcsr().u32();
        tx << "  CPACR:" << m33.cpacr().u32();
        tx << "  NSACR:" << m33.nsacr().u32();

        uint32_t sr;
        sr = m33.hfsr().u32();
        if (sr) {
            tx << "   HFSR:" << sr;
            if (sr & 0x80000000) { tx << " DEBUGEVT"; }
            if (sr & 0x40000000) { tx << " FORCED"; }
            if (sr & 0x00000002) { tx << " VECTTBL"; }
            tx << '\n';
        }

        auto cfsr = m33.cfsr().u32();

        sr = cfsr & 0xff;
        if (sr) {
            tx << "  MMFAR:" << m33.mmfar().u32();
            tx << " MFSR:" << sr;
            if (!(sr & 0x80)) { tx << " !MMARVALID"; }
            if (sr & 0x20) { tx << " MLSPERR"; }
            if (sr & 0x10) { tx << " MSTKERR"; }
            if (sr & 0x08) { tx << " MUNSTKERR"; }
            if (sr & 0x02) { tx << " DACCVIOL"; }
            if (sr & 0x01) { tx << " IACCVIOL"; }
            tx << '\n';
        }

        sr = (cfsr >> 8) & 0xff;
        if (sr) {
            tx << "   BFAR:" << m33.bfar().u32();
            tx << " BFSR:" << sr;
            if (!(sr & 0x80)) { tx << " !BFARVALID"; }
            if (sr & 0x20) { tx << " LSPERR"; }
            if (sr & 0x10) { tx << " STKERR"; }
            if (sr & 0x08) { tx << " UNSTKERR"; }
            if (sr & 0x02) { tx << " PRECISERR"; }
            if (sr & 0x01) { tx << " IBUSERR"; }
            tx << '\n';
        }

        sr = cfsr >> 16;
        if (sr) {
            tx << "   UFSR:" << sr;
            if (sr & 0x0200) { tx << " DIVBYZERO"; }
            if (sr & 0x0100) { tx << " UNALIGNED"; }
            if (sr & 0x0010) { tx << " STKOF"; }
            if (sr & 0x0008) { tx << " NOCP"; }
            if (sr & 0x0004) { tx << " INVPC"; }
            if (sr & 0x0002) { tx << " INVSTATE"; }
            if (sr & 0x0001) { tx << " UNDEFINSTR"; }
            tx << '\n';
        }

        tx << NORMAL << '\n';

        unsigned i = 10000;
        while (i--) {
            xosc.count = 12000;
            while (xosc.count) { __nop(); }
        }
    };
}

} // namespace rp2350::sys
