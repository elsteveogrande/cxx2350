#include <cxx20/cxxabi.h>
#include <rp2350/clocks.h>
#include <rp2350/common.h>
#include <rp2350/gpio.h>
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

template <uint8_t I> void initGPIOOutput(unsigned funcSel = GPIO::FuncSel<I>::SIO) {
    Update u {&padsBank0.gpio[I]};
    u->slewFast             = true;
    u->drive                = PadsBank0::Drive::k12mA;
    u->inputEnable          = false;
    u->outputDisable        = false;
    u->isolation            = false;
    gpio[I].control.funcSel = funcSel;
    sio.gpioOutEnbSet       = (1 << I);
    sio.gpioOutClr          = (1 << I);
}

// The actual application startup code, called by reset handler
[[gnu::used]] [[gnu::retain]] [[gnu::noreturn]] [[gnu::noinline]] void _start() {
    xosc.init();
    sysPLL.init150MHz();

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

    m33.ccr.unalignedTrap = true;
    m33.ccr.div0Trap      = true;

    m33.sysTick.rvr         = 1000;
    m33.sysTick.csr.enable  = 1;
    m33.sysTick.csr.tickInt = 1;

    resets.unreset(Resets::Bit::PADSBANK0);
    resets.unreset(Resets::Bit::IOBANK0);

    initGPIOOutput<25>();     // config LED
    sio.gpioOutSet = 1 << 25; // turn it on

    initGPIOOutput<0>(GPIO::FuncSel<0>::UART0TX);
    initGPIOOutput<1>(GPIO::FuncSel<1>::UART0RX);

    update(&clocks.peri.control, [&](auto& _) {
        _->auxSource = Clocks::Peri::AuxSource::PLL_SYS;
        _->kill      = false;
        _->enable    = true;
    });

    clocks.peri.div = {.fraction = 0, .integer = 1};

    resets.reset(Resets::Bit::UART0);
    resets.unreset(Resets::Bit::UART0);
    uart0.init();

    char buffer[256];
    memcpy(
        buffer,
        "hello world hello world hello world hello world hello world hello world hello world hello "
        "world hello world hello world hello world hello world hello world hello world hello world "
        "hello world hello world hello world hello world hello world hello 0123456789",
        sizeof(buffer));

    uint8_t index = 0; // index always fits in buffer

    while (true) {
        // delay: 250 * (12k / 12MHz) -> 250ms
        for (unsigned i = 0; i < 250; i++) {
            xosc.count = 12'000;
            while (xosc.count) { sys::Insns().nop(); }
        }

        // Toggle the little LED
        sio.gpioOutXor = (1 << 25);

        // Transmit a character
        uart0.data.data = buffer[index++];
    }
}
