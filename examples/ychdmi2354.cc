#include <platform.h>
#include <rp2350/buscontrol.h>
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

using namespace rp2350;

void initGPIO() {
    resets.unreset(Resets::Bit::PADSBANK0, true);
    resets.unreset(Resets::Bit::IOBANK0, true);
    initOutput<25>(); // config LED
}

void initDMA() { resets.unreset(Resets::Bit::DMA, true); }

void initHSTX() {
    initHSTXClock();
    resets.unreset(Resets::Bit::HSTX, true);
    initOutput<12>(GPIO::FuncSel<12>::HSTX);
    initOutput<13>(GPIO::FuncSel<13>::HSTX);
    initOutput<14>(GPIO::FuncSel<14>::HSTX);
    initOutput<15>(GPIO::FuncSel<15>::HSTX);
    initOutput<16>(GPIO::FuncSel<16>::HSTX);
    initOutput<17>(GPIO::FuncSel<17>::HSTX);
    initOutput<18>(GPIO::FuncSel<18>::HSTX);
    initOutput<19>(GPIO::FuncSel<19>::HSTX);
}

void configHSTX() {
    // See: p.1206: "As a final, concrete example, take TMDS (used in DVI): ..."
    // and: p.1207: "For double-data-rate data, with active rising and active
    // falling
    // ..." (But note that the example project uses clkdiv of 5)
    // https://github.com/raspberrypi/pico-examples/blob/master/hstx/dvi_out_hstx_encoder/dvi_out_hstx_encoder.c
    //
    // This will use the same pinout:
    // Pico2 pin:   16    17    18    19    20    21    22    23    24    25
    // HSTX bit:     0     1   (gnd)   2     3     4     5   (gnd)   6     7
    // GPIO:        12    13   (gnd)  14    15    16    17   (gnd)  18    19
    // DVI signal:   + CHO -           + CLK -     + CH2 -           + CH1 -

    hstx.bits[0] = {.selectP = 0, .selectN = 1, .invert = 1};
    hstx.bits[1] = {.selectP = 0, .selectN = 1, .invert = 0};
    hstx.bits[2] = {.invert = 1, .clock = true};
    hstx.bits[3] = {.invert = 0, .clock = true};
    hstx.bits[4] = {.selectP = 20, .selectN = 21, .invert = 1};
    hstx.bits[5] = {.selectP = 20, .selectN = 21, .invert = 0};
    hstx.bits[6] = {.selectP = 10, .selectN = 11, .invert = 1};
    hstx.bits[7] = {.selectP = 10, .selectN = 11, .invert = 0};

    hstx.expandShift = {
        .rawShift = 0, .rawNShifts = 1, .encShift = 16, .encNShifts = 2};

    // ----RRRRGGGGBBBB (an RGB444 pixel)
    // ----RRRR-------- -> right 4 bits -> --------RRRR----
    // --------GGGG---- -> stays put    -> --------GGGG----
    // ------------BBBB -> left 4 bits  -> --------BBBB----
    hstx.expandTMDS = {
        .l0Rot = 28, .l0NBits = 3, .l1Rot = 0, .l1NBits = 3, .l2Rot = 4, .l2NBits = 3};

    hstx.csr = {
        .enable = true, .expandEnable = true, .shift = 2, .nShifts = 5, .clkDiv = 5};
}

struct DAP {
    uint32_t swdBit;
    uint32_t swclkBit;

    DAP(uint8_t swdGPIO, uint8_t swclkGPIO, uint32_t expectDeviceID)
        : swdBit(1u << swdGPIO), swclkBit(1u << swclkGPIO) {
        (void)expectDeviceID;
        // TODO
    };
};

struct SWD {
    DAP dap;

    explicit SWD(DAP dap) : dap(dap) {}

    void writeMemory(uintptr_t target, uint8_t const* prog, unsigned size) {
        (void)target;
        (void)prog;
        (void)size;
    }
};

[[gnu::used]] [[gnu::retain]] [[gnu::noreturn]] [[gnu::noinline]] void _start() {
    initResets();
    initInterrupts();
    initCPUBasic();
    initSystemClock();
    initSystemTicks();
    initRefClock();
    initPeriphClock();
    initGPIO();
    initBusControl();
    initDMA();
    initHSTX();
    configHSTX();

    // TODO
    DAP dap(42, 43, 0x01abcdef);
    SWD swd(dap);

    __builtin_trap();
}
