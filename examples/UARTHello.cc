#include <cxx20/cxxabi.h>
#include <rp2350/clocks.h>
#include <rp2350/common.h>
#include <rp2350/gpio.h>
#include <rp2350/insns.h>
#include <rp2350/interrupts.h>
#include <rp2350/m33.h>
#include <rp2350/pads.h>
#include <rp2350/panic.h>
#include <rp2350/resets.h>
#include <rp2350/ticks.h>
#include <rp2350/uart.h>
#include <rp2350/xoscpll.h>

namespace rp2350::sys {

// Need to define a couple of structures in our main file so that they are baked into the ELF.
// These two are given `section` attributes so that they can be placed at specific flash
// addresses (see `layout.ld`).

// Interrupt vectors are needed for the thing to start; this will live at flash address
// `0x10000000`. It can live in a different address but the default is fine.
[[gnu::used]] [[gnu::retain]] [[gnu::section(".vec_table")]] ARMVectors const gARMVectors;

// Image definition is required for the RP2 bootloader; this will live at flash address
// `0x10000100`.
[[gnu::used]] [[gnu::retain]] [[gnu::section(
    ".image_def")]] constinit ImageDef2350ARM const gImageDef;

} // namespace rp2350::sys

using namespace rp2350;

template <uint16_t N> struct Buffer {
    uint8_t  buf[N];
    uint16_t head {0}; // next read / pop / dequeue pos
    uint16_t tail {0}; // next write / push / enqueue pos

    size_t size() const { return ((tail + N) - head) % N; }
    bool   empty() const { return head == tail; }
    bool   full() const { return size() == N - 1; }

    void push(uint8_t x) {
        if (!full()) { buf[tail++] = x; }
    }
    uint8_t pop() {
        if (!empty()) {
            return buf[head++];
        } else {
            return 0xff;
        }
    }
};

Buffer<64> txBuffer;
Buffer<64> rxBuffer;

void putC(char c) {
    txBuffer.push(c);
    m33.triggerIRQ(uart0.irqn());
}

void putHex(unsigned nibs, uint32_t x) {
    constexpr static char const* kHexTable = "0123456789abcdef";
    if (nibs >= 8) { putC(kHexTable[(x >> 28) & 0x0f]); }
    if (nibs >= 7) { putC(kHexTable[(x >> 24) & 0x0f]); }
    if (nibs >= 6) { putC(kHexTable[(x >> 20) & 0x0f]); }
    if (nibs >= 5) { putC(kHexTable[(x >> 16) & 0x0f]); }
    if (nibs >= 4) { putC(kHexTable[(x >> 12) & 0x0f]); }
    if (nibs >= 3) { putC(kHexTable[(x >> 8) & 0x0f]); }
    if (nibs >= 2) { putC(kHexTable[(x >> 4) & 0x0f]); }
    if (nibs >= 1) { putC(kHexTable[(x >> 0) & 0x0f]); }
}

void putU32(uint32_t x) { putHex(8, x); }
void putU16(uint32_t x) { putHex(4, x); }
void putU8(uint32_t x) { putHex(2, x); }
void putS(char const* s) {
    while (*s) { putC(*s++); }
}

[[clang::optnone]]
void uart0IRQ() {
    if (uart0.rxStatus.u32()) {
        uart0.rxStatus.u32() = 0x0f; // clear all bits
        return;
    }
    while (!uart0.flags.rxEmpty && !rxBuffer.full()) { rxBuffer.push(uart0.data.data); }
    while (!uart0.flags.txFull && !txBuffer.empty()) { uart0.data.data = txBuffer.pop(); }
    uart0.intClear.u32() = 0x7ff;
}

// The actual application startup code, called by reset handler
[[gnu::used]] [[gnu::retain]] [[gnu::noreturn]] [[gnu::noinline]] void _start() {
    sys::initInterrupts();
    xosc.init();
    sysPLL.init();

    clocks.ref.control = {.source = Clocks::Ref::Source::XOSC, .auxSource = {}};
    clocks.ref.div     = {.fraction = 0, .integer = 1};

    clocks.sys.control = {.source    = Clocks::Sys::Source::CLK_SYS_AUX,
                          .auxSource = Clocks::Sys::AuxSource::PLL_SYS};
    clocks.sys.div     = {.fraction = 0, .integer = 1};

    // p569: SDK expects nominal 1uS system ticks, as does Arm internals.
    // Although we don't use the SDK we'll assume 1uS everywhere as well.
    ticks.proc0.control.enabled = false; // disable while configuring
    ticks.proc0.cycles.count    = 12;
    ticks.proc0.control.enabled = true;
    ticks.proc1.control.enabled = false; // disable while configuring
    ticks.proc1.cycles.count    = 12;
    ticks.proc1.control.enabled = true;

    m33.ccr().unalignedTrap = true;
    m33.ccr().div0Trap      = true;

    m33.rvr()         = 1000;
    m33.csr().enable  = 1;
    m33.csr().tickInt = 1;

    resets.unreset(Resets::Bit::PADSBANK0);
    resets.unreset(Resets::Bit::IOBANK0);

    initGPIOOutput<25>();     // config LED
    sio.gpioOutSet = 1 << 25; // turn it on

    initGPIOOutput<0>(GPIO::FuncSel<0>::UART0TX);
    // initGPIOInput<1>(GPIO::FuncSel<1>::UART0RX);

    update(&clocks.peri.control, [&](auto& _) {
        _->auxSource = Clocks::Peri::AuxSource::PLL_SYS;
        _->kill      = false;
        _->enable    = true;
    });

    clocks.peri.div = {.fraction = 0, .integer = 1};

    sys::irqHandlers[uart0.irqn()] = uart0IRQ;
    resets.reset(Resets::Bit::UART0);
    resets.unreset(Resets::Bit::UART0);
    m33.enableIRQ(uart0.irqn());
    uart0.init(9600);

    sys::panic();
}
