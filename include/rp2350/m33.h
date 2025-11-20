#pragma once

#include <cxx20/cxxabi.h>

namespace rp2350 {

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

        CSR&      csr = *(CSR*)(0xe000e010);      // SysTick Control and Status Register
        uint32_t& rvr = *(uint32_t*)(0xe000e014); // SysTick Reload Value Register
    };

    struct NVIC {
        uint32_t& ser0 = *(uint32_t*)(0xe000e100); // Interrupt (0..31) Set Enable Registers
        uint32_t& cer0 = *(uint32_t*)(0xe000e180); // Interrupt (0..31) Clear Enable Registers
        uint32_t& spr0 = *(uint32_t*)(0xe000e200); // Interrupt (0..31) Set Pending Registers
        uint32_t& cpr0 = *(uint32_t*)(0xe000e280); // Interrupt (0..31) Clear Pending Registers

        void enableIRQ(uint8_t irq) { ser0 = uint32_t(1) << irq; }
        void disableIRQ(uint8_t irq) { cer0 = uint32_t(1) << irq; }
        void triggerIRQ(uint8_t irq) { spr0 = uint32_t(1) << irq; }
        void clearPendIRQ(uint8_t irq) { cpr0 = uint32_t(1) << irq; }
    };

    struct ACTLR {
        unsigned v;
    };

    struct CPUID {
        unsigned v;
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

} // namespace rp2350