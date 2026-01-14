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

    constexpr uint32_t u32() const {
        // Avoid reinterpret-cast so we can make this constexpr.
        // return *(uint32_t const*)(this);
        return (unsigned(ch2) << 20) | (unsigned(ch1) << 10) | (unsigned(ch0) << 0);
    }

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

// RGB444
struct [[gnu::packed]] Pixel {
    unsigned b : 4 {};
    unsigned g : 4 {};
    unsigned r : 4 {};
};
static_assert(sizeof(Pixel) == 2);

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
constexpr static unsigned kDMAChannelB = 1;
constexpr static unsigned kIRQDMA0     = DMA::kDMAIRQs[0];

struct Buffer {
    uint32_t const* words; // array of words for FIFO
    uint16_t count;        // word count
};

struct [[gnu::aligned(4)]] VBlankLine {
    uint32_t const cmd0_  = (1u << 12) | kHBlankFront; // HSTX_CMD_RAW_REPEAT
    TMDS const frontPorch = TMDS::sync(0, 0);
    uint32_t const cmd1_  = (1u << 12) | kHBlankSync;
    TMDS const hsync      = TMDS::sync(0, 1);
    uint32_t const cmd2_  = (1u << 12) | kHBlankBack;
    TMDS const backPorch  = TMDS::sync(0, 0);
    uint32_t const foo[16] {
        (1u << 12) | kHActive >> 4, TMDS::sync(0, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(0, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(0, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(0, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(0, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(0, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(0, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(0, 0).u32(),
    };

    constexpr VBlankLine() = default;

    Buffer buf() const {
        return {.words = (uint32_t const*)(this),
                .count = sizeof(decltype(*this)) >> 2};
    }
};

struct [[gnu::aligned(4)]] VSyncLine {
    uint32_t const cmd0_  = (1u << 12) | kHBlankFront; // HSTX_CMD_RAW_REPEAT
    TMDS const frontPorch = TMDS::sync(1, 0);
    uint32_t const cmd1_  = (1u << 12) | kHBlankSync;
    TMDS const hsync      = TMDS::sync(1, 1);
    uint32_t const cmd2_  = (1u << 12) | kHBlankBack;
    TMDS const backPorch  = TMDS::sync(1, 0);
    uint32_t const foo[16] {
        (1u << 12) | kHActive >> 4, TMDS::sync(1, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(1, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(1, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(1, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(1, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(1, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(1, 0).u32(),
        (1u << 12) | kHActive >> 4, TMDS::sync(1, 0).u32(),
    };

    constexpr VSyncLine() = default;

    Buffer buf() const {
        return {.words = (uint32_t const*)(this),
                .count = sizeof(decltype(*this)) >> 2};
    }
};

struct [[gnu::packed]] [[gnu::aligned(4)]] Pixels {
    constexpr static Pixel const kDefault {.b = 2, .g = 2, .r = 2};

    uint32_t const cmd0_  = (1u << 12) | kHBlankFront; // HSTX_CMD_RAW_REPEAT
    TMDS const frontPorch = TMDS::sync(0, 0);
    uint32_t const cmd1_  = (1u << 12) | kHBlankSync;
    TMDS const hsync      = TMDS::sync(0, 1);
    uint32_t const cmd2_  = (1u << 12) | kHBlankBack;
    TMDS const backPorch  = TMDS::sync(0, 0);
    uint32_t const cmd_   = (2u << 12) | kHActive; // HSTX_CMD_TMDS
    Pixel pixels[kHActive];                        // packed RGB444 pixels follow

    void clear(Pixel px = kDefault) {
        for (auto i = 0u; i < kHActive; i++) { pixels[i] = px; }
    }

    Buffer buf() const {
        return {.words = (uint32_t const*)(this),
                .count = sizeof(decltype(*this)) >> 2};
    }
};

auto& line0 = *(Pixels*)(0x20080000); // Even lines' pixels
auto& line1 = *(Pixels*)(0x20081000); // Odd lines' pixels

unsigned nextLine  = 0;
unsigned thisFrame = 0;

void issueResets() {
    // Turn reset on for everything except QSPI (since we're running on flash).
    // clang-format off
    constexpr static uint32_t kMask = 0
        | unsigned(Resets::Bit::ADC      )
        | unsigned(Resets::Bit::BUSCTRL  )
        | unsigned(Resets::Bit::DMA      )
        | unsigned(Resets::Bit::HSTX     )
        | unsigned(Resets::Bit::I2C0     )
        | unsigned(Resets::Bit::I2C1     )
        | unsigned(Resets::Bit::IOBANK0  )
        | unsigned(Resets::Bit::PADSBANK0)
        | unsigned(Resets::Bit::PIO0      )
        | unsigned(Resets::Bit::PIO1      )
        | unsigned(Resets::Bit::PIO2      )
        | unsigned(Resets::Bit::PWM       )
        | unsigned(Resets::Bit::SHA256    )
        | unsigned(Resets::Bit::SPI0      )
        | unsigned(Resets::Bit::SPI1      )
        | unsigned(Resets::Bit::TIMER0    )
        | unsigned(Resets::Bit::TIMER1    )
        | unsigned(Resets::Bit::TRNG      )
        | unsigned(Resets::Bit::UART0     )
        | unsigned(Resets::Bit::UART1     )
        | unsigned(Resets::Bit::USBCTRL   )
        // We won't reset these, they are needed for even minimal operation.
        // If the application wants to mess with these it still can, but we
        // won't automatically reset these.
        // | unsigned(Resets::Bit::IOQSPI    )
        // | unsigned(Resets::Bit::PADSQSPI  )
        // | unsigned(Resets::Bit::PLLSYS    )
        // | unsigned(Resets::Bit::PLLUSB    )
        // | unsigned(Resets::Bit::JTAG      )
        // | unsigned(Resets::Bit::SYSCFG    )
        // | unsigned(Resets::Bit::SYSINFO   )
        // | unsigned(Resets::Bit::TBMAN     )
    ;
    // clang-format on

    resets.resets |= kMask;
    // Some components seem to need a little bit of time before un-reset.
    for (unsigned i = 0; i < 1000000; i++) { sys::Insns().nop(); }
}

void initGPIO() {
    resets.unreset(Resets::Bit::PADSBANK0, true);
    resets.unreset(Resets::Bit::IOBANK0, true);
    initOutput<25>(); // config LED
}

void initDMA() { resets.unreset(Resets::Bit::DMA, true); }

void initHSTX() {
    sys::initHSTXClock();
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

    hstx.bits[0] = {.selectP = 0, .selectN = 1, .invert = 0};
    hstx.bits[1] = {.selectP = 0, .selectN = 1, .invert = 1};
    hstx.bits[2] = {.invert = 0, .clock = true};
    hstx.bits[3] = {.invert = 1, .clock = true};
    hstx.bits[4] = {.selectP = 20, .selectN = 21, .invert = 0};
    hstx.bits[5] = {.selectP = 20, .selectN = 21, .invert = 1};
    hstx.bits[6] = {.selectP = 10, .selectN = 11, .invert = 0};
    hstx.bits[7] = {.selectP = 10, .selectN = 11, .invert = 1};

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

namespace rp2350::sys {
struct BusControl {
    struct Priority : R32 {
        unsigned proc0    : 1; // 0
        unsigned          : 3;
        unsigned proc1    : 1; // 4
        unsigned          : 3;
        unsigned dmaRead  : 1; // 8
        unsigned          : 3;
        unsigned dmaWrite : 1; // 12
        unsigned          : 19;
    };

    Priority priority; // 0x00
};
inline auto& busControl = *(BusControl*)(0x40068000);
} // namespace rp2350::sys

void initBusControl() { resets.unreset(Resets::Bit::BUSCTRL, true); }

void configBusControl() { rp2350::sys::busControl.priority.dmaRead = 1; }

constexpr VBlankLine const vblankLine {};
constexpr VSyncLine const vsyncLine {};

void prepLine(unsigned line, Pixels& pxs) {
    line += (thisFrame >> 4);
    auto i = 176u;
    Pixel px;
    px.g = (line >> 4) & 0b1111;
    px.r = line & 0b1111;
    while (i < 688) {
        px.b            = ((i & 0b1110) >> 1) << 1;
        pxs.pixels[i++] = px;
        pxs.pixels[i++] = px;
        ++px.b;
        pxs.pixels[i++] = px;
        pxs.pixels[i++] = px;
    }
}

void prepFrame(unsigned frame) { (void)frame; }

struct Entered final {
    bool v {false};

    struct RAII final {
        Entered& e;
        RAII(Entered& e) : e(e) {
            if (e.v) { sys::abort(); }
            e.v = true;
        }
        ~RAII() { e.v = false; }
    };

    auto enter() { return RAII {*this}; }
};

void tx() {
    static Entered entered;
    auto foo = entered.enter();

    auto& irq = rp2350::dma.irqRegs(0);

    // Detect which DMA just completed
    unsigned dmaChannel;
    uint32_t irqStatus = irq.status;
    if (irqStatus & (1u << kDMAChannelA)) {
        dmaChannel = kDMAChannelA;
    } else if (irqStatus & (1u << kDMAChannelB)) {
        dmaChannel = kDMAChannelB;
    } else {
        return;
    }

    auto& ch = rp2350::dma.channels[dmaChannel];
    if (ch.ctrl.ahbError) { sys::abort(); }

    // nextLine is the next one to "draw" out; prepare it.
    // Note that (nextLine - 1) is already being sent on the other channel.
    Buffer buf;
    if (nextLine < kVActive) {
        auto& line = (nextLine & 1) ? line1 : line0;
        prepLine(nextLine, line);
        buf = line.buf();
    } else if (nextLine < kVActive + kVBlankFront) {
        buf = vblankLine.buf();
    } else if (nextLine < kVActive + kVBlankFront + kVBlankSync) {
        buf = vsyncLine.buf();
    } else {
        buf = vblankLine.buf();
    }

    ch.readAddr   = uintptr_t(buf.words);
    ch.transCount = {.count = buf.count, .mode = DMA::Mode::NORMAL};

    // nextLine is now prepared and ready to be sent upon next invocation.

    ++nextLine;
    if (nextLine >= kVTotal) {
        nextLine = 0;
        ++thisFrame;
        prepFrame(thisFrame);
    }

    irq.status = (1u << dmaChannel); // clear flag
}

// // The actual application startup code, called by reset handler
[[gnu::used]] [[gnu::retain]] [[gnu::noreturn]] [[gnu::noinline]] void _start() {
    issueResets();
    sys::initInterrupts();
    sys::initCPUBasic();
    sys::initSystemClock();
    sys::initSystemTicks();
    sys::initRefClock();
    sys::initPeriphClock();
    initBusControl();
    initGPIO();
    initDMA();
    initHSTX();
    configHSTX();
    configBusControl();

    new (&line0) Pixels();
    new (&line1) Pixels();
    line0.clear();
    line1.clear();

    // Set up the two DMA channels; initially point them at dummy buffers (blank
    // lines).
    auto buf = vblankLine.buf();

    auto& chA = dma.channels[kDMAChannelA];
    update(&chA.ctrl, [](auto& _) {
        _.zero();
        _->chainTo  = kDMAChannelB;
        _->incrRead = true;
        _->treqSel  = kHSTXDREQ;
        _->dataSize = DMA::DataSize::_32BIT;
        _->enable   = true;
    });
    chA.writeAddr  = uintptr_t(&hstx.fifo().fifoWrite);
    chA.readAddr   = uintptr_t(buf.words);
    chA.transCount = {.count = buf.count, .mode = DMA::Mode::NORMAL};

    auto& chB = dma.channels[kDMAChannelB];
    update(&chB.ctrl, [](auto& _) {
        _.zero();
        _->chainTo  = kDMAChannelA;
        _->incrRead = true;
        _->treqSel  = kHSTXDREQ;
        _->dataSize = DMA::DataSize::_32BIT;
        _->enable   = true;
    });
    chB.writeAddr  = uintptr_t(&hstx.fifo().fifoWrite);
    chB.readAddr   = uintptr_t(buf.words);
    chB.transCount = {.count = buf.count, .mode = DMA::Mode::NORMAL};

    auto& irq  = rp2350::dma.irqRegs(0);
    irq.status = (1u << kDMAChannelA) | (1u << kDMAChannelB); // clear flags
    irq.enable = (1u << kDMAChannelA) | (1u << kDMAChannelB);

    sys::irqHandlers[kIRQDMA0] = tx;
    m33.clrPendIRQ(kIRQDMA0);
    m33.enableIRQ(kIRQDMA0);

    thisFrame = ~0u;         // will increment to first frame (0)
    nextLine  = kVTotal - 1; // will wrap back to 0, bumping frame

    rp2350::dma.multiChanTrigger.channels = 1u << kDMAChannelA;

    while (true) {
        auto f = thisFrame % 60;
        if (f < 30) {
            sio.gpioOutSet = 1 << 25;
        } else {
            sio.gpioOutClr = 1 << 25;
        }
    }
}
