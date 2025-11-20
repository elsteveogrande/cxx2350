#include <rp2350/clocks.h>
#include <rp2350/common.h>
#include <rp2350/gpio.h>
#include <rp2350/interrupts.h>
#include <rp2350/m33.h>
#include <rp2350/pads.h>
#include <rp2350/panic.h>
#include <rp2350/ticks.h>
#include <rp2350/xoscpll.h>

namespace rp2350::sys {

[[gnu::used]] [[gnu::retain]] [[gnu::section(".vec_table")]] ARMVectors const gARMVectors {};

[[gnu::used]] [[gnu::retain]] [[gnu::section(
    ".image_def")]] constinit ImageDef2350ARM const gImageDef {};

} // namespace rp2350::sys

using namespace rp2350;

[[gnu::used]] [[gnu::retain]] [[gnu::noreturn]] [[gnu::noinline]] void _start() {
    xosc.init();
    sysPLL.init150MHz();

    clocks.sys.control.auxSource = Clocks::Sys::AuxSource::PLL_SYS;
    clocks.sys.control.source    = Clocks::Sys::Source::CLK_SYS_AUX;
    clocks.sys.div               = {.fraction = 0, .integer = 1};
    clocks.ref.control.source    = Clocks::Ref::Source::XOSC;
    clocks.ref.div               = {.fraction = 0, .integer = 1};

    // p569: SDK expects nominal 1uS system ticks, as does Arm internals.
    // Although we don't use the SDK we'll assume 1uS everywhere as well.
    ticks.proc0.control.enabled = false; // disable while configuring
    ticks.proc0.cycles.count    = 12;
    ticks.proc0.control.enabled = true;
    ticks.proc1.control.enabled = false; // disable while configuring
    ticks.proc1.cycles.count    = 12;
    ticks.proc1.control.enabled = true;

    m33.ccr.div0Trap      = true;
    m33.ccr.unalignedTrap = true;

    m33.sysTick.rvr         = 1000;
    m33.sysTick.csr.tickInt = 1;
    m33.sysTick.csr.enable  = 1;

    resets.unreset(Resets::Bit::PADSBANK0);
    resets.unreset(Resets::Bit::IOBANK0);

    padsBank0.gpio[25] = {.slewFast      = 1,
                          .drive         = PadsBank0::Drive::k12mA,
                          .inputEnable   = 0,
                          .outputDisable = 0,
                          .isolation     = 0};

    gpio[25].control.funcSel = GPIO::FuncSel::SIO;
    sio.gpioOutEnbSet        = (1 << 25);
    sio.gpioOutClr           = (1 << 25);

    while (true) {
        sio.gpioOutXor = (1 << 25);
        for (uint i = 0; i < 250; i++) {
            xosc.count = 12'000;
            while (xosc.count) { sys::Insns().nop(); }
        }
    }
}
