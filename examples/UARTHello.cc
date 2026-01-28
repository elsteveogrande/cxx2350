#include <cxx20/cxxabi.h>
#include <examples/config.h>
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

namespace rp2350::sys {

// Need to define a couple of structures in our main file so that they are baked into
// the ELF. These two are given `section` attributes so that they can be placed at
// specific flash addresses (see `layout.ld`).

// Interrupt vectors are needed for the thing to start; this will live at flash address
// `0x10000000`. It can live in a different address but the default is fine.
[[gnu::used]] [[gnu::retain]] [[gnu::section(
    ".vec_table")]] ARMVectors const gARMVectors;

// Image definition is required for the RP2 bootloader; this will live at flash address
// `0x10000100`.
[[gnu::used]] [[gnu::retain]] [[gnu::section(
    ".image_def")]] constinit ImageDef2350ARM const gImageDef;

} // namespace rp2350::sys

using namespace rp2350;

template <uint16_t N> struct Buffer {
    uint16_t head {0}; // next read / pop / dequeue pos
    uint16_t tail {0}; // next write / push / enqueue pos
    uint8_t buf[N];

    size_t size() const { return (tail - head) % N; }
    bool empty() const { return head == tail; }
    bool full() const { return size() == N - 1; }

    void push(uint8_t x) {
        while (full()) { sys::debug() << "push:full"; }
        buf[tail] = x;
        tail      = (tail + 1) % N;
    }

    uint8_t pop() {
        uint8_t ret = 0xff;
        while (empty()) { sys::debug() << "pop:empty"; }
        ret  = buf[head];
        head = (head + 1) % N;
        return ret;
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

void uart0IRQ() {
    if (!uart0.flags.rxEmpty) { rxBuffer.push(uart0.data.data); }
    if (!uart0.flags.txFull && !txBuffer.empty()) { uart0.data.data = txBuffer.pop(); }
    uart0.intClear.u32() = 0x7ff;
}

// The actual application startup code, called by reset handler
[[gnu::used]] [[gnu::retain]] [[gnu::noreturn]] [[gnu::noinline]] void _start() {
    resets.issueResets();
    sys::initInterrupts();
    sys::initCPUBasic();
    sys::initSystemClock(sys::kFBDiv, sys::kDiv1, sys::kDiv2);
    sys::initRefClock();
    sys::initPeriphClock();
    sys::initSystemTicks();
    sys::initGPIO();

    sio.gpioOutSet = 1 << 25; // turn it on

    initOutput<0>(GPIO::FuncSel<0>::UART0TX);
    initInput<1>(GPIO::FuncSel<1>::UART0RX);

    resets.reset(Resets::Bit::UART0);
    for (unsigned i = 0; i < 1000000; i++) { sys::Insns().nop(); }
    sys::irqHandlers[uart0.irqn()] = uart0IRQ;
    m33.enableIRQ(uart0.irqn());
    resets.unreset(Resets::Bit::UART0);
    uart0.init(sys::kSysHz, 19200);

    while (true) {
        putHex(4, 0xbeef);
        putS(" hello ");
        sys::Insns().nop();
    }
}
