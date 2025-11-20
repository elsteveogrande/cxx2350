#pragma once
#include "common.h"

namespace rp2 {
// 3.7. Cortex-M33 Processor
struct M33 {
    struct SysTick {
        enum class ClockSource { EXT_REF_CLK = 0, PROC_CLK = 1 };
        struct CSR {
            unsigned enable  : 1; // 0
            unsigned tickInt : 1; // 1
            unsigned source  : 1; // 2
            unsigned         : 13;
            unsigned count   : 1; // 16
            unsigned         : 15;
        };

        CSR& csr = *(CSR*)(0xe000e010); // SysTick Control and Status Register
        u32& rvr = *(u32*)(0xe000e014); // SysTick Reload Value Register
    };

    struct NVIC {
        u32& ser0 = *(u32*)(0xe000e100); // Interrupt (0..31) Set Enable Registers
        u32& cer0 = *(u32*)(0xe000e180); // Interrupt (0..31) Clear Enable Registers
        u32& spr0 = *(u32*)(0xe000e200); // Interrupt (0..31) Set Pending Registers
        u32& cpr0 = *(u32*)(0xe000e280); // Interrupt (0..31) Clear Pending Registers

        void enableIRQ(u8 irq) { ser0 = u32(1) << irq; }
        void disableIRQ(u8 irq) { cer0 = u32(1) << irq; }
        void triggerIRQ(u8 irq) { spr0 = u32(1) << irq; }
        void clearPendIRQ(u8 irq) { cpr0 = u32(1) << irq; }
    };

    struct ACTLR {
        uvint v;
    };

    struct CPUID {
        uvint v;
    };

    struct ICSR {
        unsigned                : 28;
        unsigned pendingSysTick : 1; // 28
        unsigned                : 1;
        unsigned pendingSV      : 1; // 30
        unsigned pendingNMI     : 1; // 31
    };

    struct VTOR {
        // TODO
        uvint v;
    };
    struct AIRCR {
        // TODO
        uvint v;
    };
    struct SCR {
        // TODO
        uvint v;
    };

    struct CCR {
        unsigned               : 3;
        unsigned unalignedTrap : 1; // 3
        unsigned div0Trap      : 1; // 4
        unsigned               : 27;
    };

    struct SHPR1 {
        // TODO
        uvint v;
    };
    struct SHPR2 {
        // TODO
        uvint v;
    };
    struct SHPR3 {
        // TODO
        uvint v;
    };
    struct SHCSR {
        // TODO
        uvint v;
    };
    struct CFSR {
        // TODO
        uvint v;
    };
    struct HFSR {
        // TODO
        uvint v;
    };
    struct MMFAR {
        // TODO
        uvint v;
    };
    struct BFAR {
        // TODO
        uvint v;
    };
    struct CPACR {
        // TODO
        uvint v;
    };
    struct NSACR {
        // TODO
        uvint v;
    };

    SysTick sysTick;
    NVIC    nvic;
    ACTLR&  actlr = *(ACTLR*)(0xE000E008); // Auxiliary Control Register - Cortex-M33
    CPUID&  cpuid = *(CPUID*)(0xE000ED00); // CPUID Base Register
    ICSR&   icsr  = *(ICSR*)(0xE000ED04);  // Interrupt Control and State Register
    VTOR&   vtor  = *(VTOR*)(0xE000ED08);  // Vector Table Offset Register
    AIRCR&  aircr = *(AIRCR*)(0xE000ED0C); // Application Interrupt and Reset Control Register
    SCR&    scr   = *(SCR*)(0xE000ED10);   // System Control Register - Cortex-M33
    CCR&    ccr   = *(CCR*)(0xE000ED14);   // Configuration and Control Register
    SHPR1&  shpr1 = *(SHPR1*)(0xE000ED18); // System Handler Priority Register 1
    SHPR2&  shpr2 = *(SHPR2*)(0xE000ED1C); // System Handler Priority Register 2
    SHPR3&  shpr3 = *(SHPR3*)(0xE000ED20); // System Handler Priority Register 3
    SHCSR&  shcsr = *(SHCSR*)(0xE000ED24); // System Handler Control and State Register
    CFSR&   cfsr  = *(CFSR*)(0xE000ED28);  // Configurable Fault Status Register
    HFSR&   hfsr  = *(HFSR*)(0xE000ED2C);  // HardFault Status Register
    MMFAR&  mmfar = *(MMFAR*)(0xE000ED34); // MemManage Fault Address Register
    BFAR&   bfar  = *(BFAR*)(0xE000ED38);  // BusFault Address Register
    CPACR&  cpacr = *(CPACR*)(0xE000ED88); // Coprocessor Access Control Register
    NSACR&  nsacr = *(NSACR*)(0xE000ED8C); // Non-secure Access Control Register
};
inline M33 m33;

} // namespace rp2