#pragma once

#include <cxx20/cxxabi.h>

namespace rp2350 {

// 3.7. Cortex-M33 Processor
struct M33 {
    enum class ClockSource { EXT_REF_CLK = 0, PROC_CLK = 1 };
    struct CSR {
        unsigned enable  : 1; // 0
        unsigned tickInt : 1; // 1
        unsigned source  : 1; // 2
        unsigned         : 13;
        unsigned count   : 1; // 16
        unsigned         : 15;
    };

    struct ACTLR {
        unsigned v;
    };

    struct CPUID {
        unsigned v;
    };

    struct ICSR {
        unsigned vectActive          : 9; // 8..0
        unsigned                     : 2;
        unsigned retToBase           : 1; // 11
        unsigned vectPending         : 9; // 20..12
        unsigned                     : 1;
        unsigned sysTickSecure       : 1; // 22
        unsigned isrPreempt          : 1; // 23
        unsigned isrPending          : 1; // 24
        unsigned pendingSysTickClear : 1; // 25
        unsigned pendingSysTick      : 1; // 26
        unsigned pendingSVClear      : 1; // 27
        unsigned pendingSV           : 1; // 28
        unsigned                     : 1;
        unsigned pendingNMIClear     : 1; // 30
        unsigned pendingNMI          : 1; // 31
    };

    struct VTOR {
        // TODO
        unsigned v;
    };
    struct AIRCR {
        // TODO
        unsigned v;
    };
    struct SCR {
        // TODO
        unsigned v;
    };

    struct CCR {
        unsigned               : 3;
        unsigned unalignedTrap : 1; // 3
        unsigned div0Trap      : 1; // 4
        unsigned               : 27;
    };

    struct SHPR1 {
        // TODO
        unsigned v;
    };
    struct SHPR2 {
        // TODO
        unsigned v;
    };
    struct SHPR3 {
        // TODO
        unsigned v;
    };
    struct SHCSR {
        // TODO
        unsigned v;
    };
    struct CFSR {
        // TODO
        unsigned v;
    };
    struct HFSR {
        // TODO
        unsigned v;
    };
    struct MMFAR {
        // TODO
        unsigned v;
    };
    struct BFAR {
        // TODO
        unsigned v;
    };
    struct CPACR {
        // TODO
        unsigned v;
    };
    struct NSACR {
        // TODO
        unsigned v;
    };

    // clang-format off

    // Registers in space 0xe0000000 - 0xe000ffff:
    //  - See datasheet "3.7.5. List of registers", p149
    //  - https://developer.arm.com/documentation/100235/0100/The-Cortex-M33-Peripherals/System-Control-Block/System-control-block-registers-summary

    uint32_t regs[1 << 14];  // 64kB of space

    ACTLR&    actlr() { return *(ACTLR   *)(&regs[0xe008 >> 2]); }  // Auxiliary Control Register
    CSR&      csr()   { return *(CSR     *)(&regs[0xe010 >> 2]); }  // SysTick Control and Status Register
    uint32_t& rvr()   { return *(uint32_t*)(&regs[0xe014 >> 2]); }  // SysTick Reload Value Register

    uint32_t& ser0()  { return *(uint32_t*)(&regs[0xe100 >> 2]); }  // Interrupt (0..31) Set Enable Registers
    uint32_t& cer0()  { return *(uint32_t*)(&regs[0xe180 >> 2]); }  // Interrupt (0..31) Clear Enable Registers
    uint32_t& spr0()  { return *(uint32_t*)(&regs[0xe200 >> 2]); }  // Interrupt (0..31) Set Pending Registers
    uint32_t& cpr0()  { return *(uint32_t*)(&regs[0xe280 >> 2]); }  // Interrupt (0..31) Clear Pending Registers

    CPUID&    cpuid() { return *(CPUID   *)(&regs[0xed00 >> 2]); }  // CPUID Base Register
    ICSR&     icsr()  { return *(ICSR    *)(&regs[0xed04 >> 2]); }  // Interrupt Control and State Register
    VTOR&     vtor()  { return *(VTOR    *)(&regs[0xed08 >> 2]); }  // Vector Table Offset Register
    AIRCR&    aircr() { return *(AIRCR   *)(&regs[0xed0c >> 2]); }  // Application Interrupt and Reset Control Register
    SCR&      scr()   { return *(SCR     *)(&regs[0xed10 >> 2]); }  // System Control Register - Cortex-M33
    CCR&      ccr()   { return *(CCR     *)(&regs[0xed14 >> 2]); }  // Configuration and Control Register
    SHPR1&    shpr1() { return *(SHPR1   *)(&regs[0xed18 >> 2]); }  // System Handler Priority Register 1
    SHPR2&    shpr2() { return *(SHPR2   *)(&regs[0xed1c >> 2]); }  // System Handler Priority Register 2
    SHPR3&    shpr3() { return *(SHPR3   *)(&regs[0xed20 >> 2]); }  // System Handler Priority Register 3
    SHCSR&    shcsr() { return *(SHCSR   *)(&regs[0xed24 >> 2]); }  // System Handler Control and State Register
    CFSR&     cfsr()  { return *(CFSR    *)(&regs[0xed28 >> 2]); }  // Configurable Fault Status Register
    HFSR&     hfsr()  { return *(HFSR    *)(&regs[0xed2c >> 2]); }  // HardFault Status Register
    MMFAR&    mmfar() { return *(MMFAR   *)(&regs[0xed34 >> 2]); }  // MemManage Fault Address Register
    BFAR&     bfar()  { return *(BFAR    *)(&regs[0xed38 >> 2]); }  // BusFault Address Register
    CPACR&    cpacr() { return *(CPACR   *)(&regs[0xed88 >> 2]); }  // Coprocessor Access Control Register
    NSACR&    nsacr() { return *(NSACR   *)(&regs[0xed8c >> 2]); }  // Non-secure Access Control Register

    // clang-format on

    uint32_t& ser(unsigned m) { return *(&ser0() + m); }
    uint32_t& cer(unsigned m) { return *(&cer0() + m); }
    uint32_t& spr(unsigned m) { return *(&spr0() + m); }
    uint32_t& cpr(unsigned m) { return *(&cpr0() + m); }

    void enableIRQ(unsigned irq) { ser(irq >> 5) = uint32_t(1) << (irq & 31); };
    void disableIRQ(unsigned irq) { cer(irq >> 5) = uint32_t(1) << (irq & 31); };
    void triggerIRQ(unsigned irq) { spr(irq >> 5) = uint32_t(1) << (irq & 31); };
    void clrPendIRQ(unsigned irq) { cpr(irq >> 5) = uint32_t(1) << (irq & 31); };
};
inline auto& m33 = *(M33*)(0xe0000000);

} // namespace rp2350