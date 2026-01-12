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

#include "HDMI.Image.h"

// For 640x480 at appx. 60fps
static_assert(rp2350::sys::kSysHz == 150'000'000);
constexpr static unsigned kHActive     = 864;
constexpr static unsigned kVActive     = 486;
constexpr static unsigned kHBlankFront = 36;
constexpr static unsigned kHBlankSync  = 64;
constexpr static unsigned kHBlankBack  = 36;
constexpr static unsigned kHBlank      = kHBlankFront + kHBlankSync + kHBlankBack;
constexpr static unsigned kVBlankFront = 4;
constexpr static unsigned kVBlankSync  = 6;
constexpr static unsigned kVBlankBack  = 4;
constexpr static unsigned kVBlank      = kVBlankFront + kVBlankSync + kVBlankBack;
constexpr static unsigned kHTotal      = kHActive + kHBlank;
constexpr static unsigned kVTotal      = kVActive + kVBlank;

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

struct TERC {
    unsigned code : 10;
    constexpr operator unsigned() const { return unsigned(code); };
};

struct [[gnu::packed]] TMDS {
    unsigned ch0 : 10 {};
    unsigned ch1 : 10 {};
    unsigned ch2 : 10 {};
    unsigned     : 2;

    uint32_t u32() const { return *(uint32_t*)(this); }

    // Encode 6 bits (2 bits across 3 channels)
    constexpr static TMDS control(uint8_t ch2, uint8_t ch1, uint8_t ch0) {
        return {
            .ch0 = kControl[ch0 & 0x03].code,
            .ch1 = kControl[ch1 & 0x03].code,
            .ch2 = kControl[ch2 & 0x03].code,
        };
    }

    // Encode 6 bits (2 bits across 3 channels): CTL3..0, plus set CH0 ctl bits for sync
    constexpr static TMDS control(uint8_t ch2, uint8_t ch1, bool vsync, bool hsync) {
        uint8_t v = vsync ? 0b10 : 0b00;
        uint8_t h = hsync ? 0b01 : 0b00;
        return control(ch2, ch1, v | h);
    }

    constexpr static TMDS idle() { return control(0x00, 0x00, 0x00); }

    constexpr static TMDS sync(bool vsync, bool hsync) {
        return control(0x00, 0x00, vsync, hsync);
    }

    constexpr static TMDS hsync() { return control(0x00, 0x00, false, true); }

    // For encoding 4 data bits
    constexpr static TERC kTERC[16] {
        {.code = 0b1010011100}, {.code = 0b1001100011}, {.code = 0b1011100100},
        {.code = 0b1011100010}, {.code = 0b0101110001}, {.code = 0b0100011110},
        {.code = 0b0110001110}, {.code = 0b0100111100}, {.code = 0b1011001100},
        {.code = 0b0100111001}, {.code = 0b0110011100}, {.code = 0b1011000110},
        {.code = 0b1010001110}, {.code = 0b1001110001}, {.code = 0b0101100011},
        {.code = 0b1011000011},
    };

    // For encoding 2 control bits:
    // CH0: D0=HSYNC  D1=VSYNC
    // CH1: D0=CTL0   D1=CTL1
    // CH2: D0=CTL2   D1=CTL3
    constexpr static TERC kControl[4] {
        {.code = 0b1101010100},
        {.code = 0b0010101011},
        {.code = 0b0101010100},
        {.code = 0b1010101011},
    };
};
static_assert(sizeof(TMDS) == 4);

// RGB565
struct [[gnu::packed]] Pixel {
    unsigned b : 5 {};
    unsigned g : 6 {};
    unsigned r : 5 {};
    Pixel() = default;
    Pixel(uint32_t x) { u32() = x; }
    uint32_t& u32() { return *(uint32_t*)this; }
};
static_assert(sizeof(Pixel) == 2);

struct Buffer {
    constexpr static unsigned kMaxWords = 1022;
    unsigned size;
    unsigned foo;
    uint32_t words[kMaxWords];

    constexpr static unsigned kRaw      = 0x00;
    constexpr static unsigned kRawRep   = 0x01;
    constexpr static unsigned kPixel    = 0x02;
    constexpr static unsigned kPixelRep = 0x03;
    constexpr static unsigned kNOP      = 0x0f;

    void push(uint32_t word) {
        if (size < kHTotal) { words[size++] = word; }
    }

    uint32_t cmd(unsigned cmd, unsigned size) {
        return (uint32_t(cmd & 0x0f) << 12) | (uint32_t(size) & 0xfff);
    }

    void clear() {
        size = 0;
        for (unsigned i = 0; i < 16; i++) { push(cmd(kNOP, 0)); }
    }

    void push(TMDS x, unsigned rep = 1) {
        push(cmd(kRawRep, rep));
        push(x.u32());
    }

    void push(Pixel _px, unsigned rep = 1) {
        auto px = _px.u32();
        push(cmd(kPixelRep, rep));
        push((px << 16) | px);
    }

    void pushHBlank(bool vsync) {
        push(TMDS::sync(vsync, false), kHBlankFront);
        push(TMDS::sync(vsync, true), kHBlankSync);
        push(TMDS::sync(vsync, false), kHBlankBack);
    }

    // Pushes an entire vblank line.
    // `this` should be empty.
    void pushVBlank(bool vsync) {
        pushHBlank(vsync);
        push(TMDS::sync(vsync, false), kHActive);
    }

    void blank() {
        clear();
        push(TMDS::idle(), 1000);
        while (size < 64) { push(cmd(kNOP, 0)); }
    }

    // TMDS pushDataPreamble(bool hsync = false, bool vsync = false) {
    //     // Table 5-2; watch out for the CTL bit order.  CTL3:0 = 0101
    //     return TMDS::control(0b01, 0b01, vsync, hsync);
    // }

    // TMDS pushVideoPreamble(bool hsync = false, bool vsync = false) {
    //     // Table 5-2; watch out for the CTL bit order.  CTL3:0 = 0001
    //     return TMDS::control(0b00, 0b01, vsync, hsync);
    // }

    // TMDS pushDataGuard(bool hsync = false, bool vsync = false) {
    //     auto v = vsync ? 0b0010 : 0;
    //     auto h = hsync ? 0b0001 : 0;
    //     return {.ch0 = TMDS::kTERC[0b1100 | v | h].code,
    //             .ch1 = 0b0100110011,
    //             .ch2 = 0b0100110011};
    // }

    // TMDS pushVideoGuard() {
    //     return {.ch0 = 0b1011001100, .ch1 = 0b0100110011, .ch2 = 0b1011001100};
    // }
};
static_assert(sizeof(Buffer) <= 4096);

// We'll claim SRAM8 and SRAM9 for two separately-bussed buffers (see Section 2.2.3
// "SRAM"). We purposely avoid accessing (for read or write) the region of memory
// currently being used by DMA so as to avoid stalls or jitter.
auto& bufA = *(Buffer*)(0x20080000); // Even lines
auto& bufB = *(Buffer*)(0x20081000); // Odd lines

unsigned currentLine  = 0;
unsigned currentFrame = 0;

// Return a reference to a TMDS buffer (one of `bufA` or `bufB`) depending on line
constexpr auto& lineBuffer(unsigned oline) { return (oline & 1) ? bufB : bufA; }

// Write line (of pixels, controls, sync) into `buf`.
// `displayLine` is in range 0 to 485.
void prepVideoLine(Buffer& buf, unsigned displayLine) {
    buf.pushHBlank(false);
    buf.push(Pixel(0x00f), kHActive);
    (void)displayLine;
}

void prepLine() {
    constexpr static unsigned kVFrontPorchLine = 0;
    constexpr static unsigned kVSyncLine       = kVFrontPorchLine + kVBlankFront;
    constexpr static unsigned kVBackPorchLine  = kVSyncLine + kVBlankSync;
    constexpr static unsigned kVActiveLine     = kVBackPorchLine + kVBlankBack;

    auto& buf = lineBuffer(currentLine);
    buf.clear();

    if (currentLine < kVSyncLine) {
        buf.pushVBlank(false);
    } else if (currentLine < kVBackPorchLine) {
        buf.pushVBlank(true);
    } else if (currentLine < kVActiveLine) {
        buf.pushVBlank(false);
    } else {
        prepVideoLine(buf, currentLine - kVActiveLine);
    }
}

namespace rp2350::sys {

void initCPUBasic() {
    m33.ccr().unalignedTrap = true;
    m33.ccr().div0Trap      = true;
}

void initSystemClock() {
    xosc.init();
    sysPLL.init();

    clocks.sys.control = {.source    = Clocks::Sys::Source::CLK_SYS_AUX,
                          .auxSource = Clocks::Sys::AuxSource::PLL_SYS};
    clocks.sys.div     = {.fraction = 0, .integer = 1};
}

void initRefClock() {
    clocks.ref.control = {.source = Clocks::Ref::Source::XOSC, .auxSource = {}};
    clocks.ref.div     = {.fraction = 0, .integer = 1};
}

void initPeriphClock() {
    clocks.peri.control = {
        .auxSource = Clocks::Peri::AuxSource::PLL_SYS, .kill = false, .enable = true};
    clocks.peri.div = {.fraction = 0, .integer = 1};
}

void initHSTXClock() {
    update(&clocks.hstx.control, [](auto& _) {
        _.zero();
        _->auxSource = Clocks::HSTX::AuxSource::CLK_SYS;
        _->kill      = false;
        _->enable    = true;
    });
    clocks.hstx.div = {.fraction = 0, .integer = 1};
}

void initSystemTicks() {
    // p569: SDK expects nominal 1uS system ticks, as does Arm internals.
    // Although we don't use the SDK we'll assume 1uS everywhere as well.
    ticks.proc0.control.enabled = false; // disable while configuring
    ticks.proc0.cycles.count    = 12;
    ticks.proc0.control.enabled = true;
    ticks.proc1.control.enabled = false; // disable while configuring
    ticks.proc1.cycles.count    = 12;
    ticks.proc1.control.enabled = true;

    m33.rvr()         = 1000;
    m33.csr().enable  = 1;
    m33.csr().tickInt = 1;
}

} // namespace rp2350::sys

using namespace rp2350;

constexpr static unsigned kHSTXDREQ    = 52; // p.1102
constexpr static unsigned kDMAChannelA = 0;
constexpr static unsigned kIRQDMA0     = DMA::kDMAIRQs[0];

void tx() {
    using rp2350::DMA;

    auto& buf = lineBuffer(currentLine);
    auto& ch  = rp2350::dma.channels[kDMAChannelA];

    ch.readAddr             = uintptr_t(&buf.words);
    ch.transCountTrig.count = buf.size;

    ++currentLine;
    if (currentLine == kVTotal) {
        currentLine = 0;
        ++currentFrame;
    }

    // prepLine();

    rp2350::dma.irqRegs(0).status = (1u << kDMAChannelA);
}

// // The actual application startup code, called by reset handler
[[gnu::used]] [[gnu::retain]] [[gnu::noreturn]] [[gnu::noinline]] void _start() {
    sys::initInterrupts();
    sys::initCPUBasic();
    sys::initSystemClock();
    sys::initSystemTicks();
    sys::initRefClock();
    sys::initPeriphClock();

    resets.unreset(Resets::Bit::PADSBANK0, true);
    resets.unreset(Resets::Bit::IOBANK0, true);

    initOutput<25>(); // config LED

    resets.reset(Resets::Bit::DMA);
    for (unsigned i = 0; i < 1000000; i++) { sys::Insns().nop(); }
    resets.unreset(Resets::Bit::DMA, true);

    sys::initHSTXClock();

    resets.reset(Resets::Bit::HSTX);
    sio.gpioOutClr = 0x000ff000;
    for (unsigned i = 0; i < 1000000; i++) { sys::Insns().nop(); }
    resets.unreset(Resets::Bit::HSTX, true);

    initOutput<12>(GPIO::FuncSel<12>::HSTX);
    initOutput<13>(GPIO::FuncSel<13>::HSTX);
    initOutput<14>(GPIO::FuncSel<14>::HSTX);
    initOutput<15>(GPIO::FuncSel<15>::HSTX);
    initOutput<16>(GPIO::FuncSel<16>::HSTX);
    initOutput<17>(GPIO::FuncSel<17>::HSTX);
    initOutput<18>(GPIO::FuncSel<18>::HSTX);
    initOutput<19>(GPIO::FuncSel<19>::HSTX);

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

    hstx.bits[0] = {.selectP = 0, .selectN = 1, .invert = 0};
    hstx.bits[1] = {.selectP = 0, .selectN = 1, .invert = 1};
    hstx.bits[2] = {.invert = 0, .clock = true};
    hstx.bits[3] = {.invert = 1, .clock = true};
    hstx.bits[4] = {.selectP = 20, .selectN = 21, .invert = 0};
    hstx.bits[5] = {.selectP = 20, .selectN = 21, .invert = 1};
    hstx.bits[6] = {.selectP = 10, .selectN = 11, .invert = 0};
    hstx.bits[7] = {.selectP = 10, .selectN = 11, .invert = 1};

    // Configure the HSTX expander for RGB565.
    // A packed 32-bit word has 2 pixels:
    //
    //  3         2         1
    // 10987654321098765432109876543210
    // RRRRRGGGGGGBBBBBRRRRRGGGGGGBBBBB
    //
    // Each 16-bit pixel looks like:
    // 1    1
    // 5432109876543210
    // RRRRRGGGGGGBBBBB; so to get R, G, and B into position (still 8-bit color
    // values):
    //         RRRRR--- (shift R right by 8)
    //         GGGGGG-- (shift G right by 3)
    //         BBBBB--- (shift B left by 3)
    //
    // we'll need L2/1/0 bit-width values (remember they're one less) of 4, 5, and
    // 4, and right-rotation values of 8, 3, and (32 - 3).

    hstx.expandShift = {
        .rawShift = 0, .rawNShifts = 1, .encShift = 16, .encNShifts = 2};

    hstx.expandTMDS = {
        .l0Rot = 29, .l0NBits = 4, .l1Rot = 3, .l1NBits = 5, .l2Rot = 8, .l2NBits = 4};

    hstx.csr = {
        .enable = true, .expandEnable = true, .shift = 2, .nShifts = 5, .clkDiv = 5};

    sys::irqHandlers[kIRQDMA0] = tx;

    lineBuffer(0).blank();
    lineBuffer(1).blank();

    currentFrame = currentLine = 0;

    auto& irq  = rp2350::dma.irqRegs(0);
    irq.status = (1u << kDMAChannelA); // clear flag
    irq.enable = (1u << kDMAChannelA);
    m33.enableIRQ(kIRQDMA0);

    auto& chA = dma.channels[kDMAChannelA];
    update(&chA.ctrl, [](auto& _) {
        _.zero();
        _->chainTo  = kDMAChannelA;
        _->incrRead = true;
        _->treqSel  = kHSTXDREQ;
        _->dataSize = DMA::DataSize::_32BIT;
        _->enable   = true;
    });
    chA.writeAddr       = uintptr_t(&hstx.fifo().fifoWrite);
    chA.readAddr        = uintptr_t(&bufA.words);
    chA.transCount.mode = DMA::Mode::NORMAL;

    tx();

    while (true) {
        auto f = currentFrame % 60;
        if (f < 30) {
            sio.gpioOutSet = 1 << 25;
        } else {
            sio.gpioOutClr = 1 << 25;
        }
    }
}
