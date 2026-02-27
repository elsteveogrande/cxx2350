#pragma once

#include <platform.h>
#include <rp2350/common.h>
#include <rp2350/insns.h>
#include <rp2350/m33.h>

extern "C" {

constexpr unsigned kStackWords = 256;
[[gnu::retain]] [[gnu::used]] inline uint32_t __stack[kStackWords];

[[gnu::noinline]] [[gnu::retain]] [[gnu::used]] [[gnu::section(".systext")]]
inline void __reset() {
    void* sram = &__sram_begin;
    auto size = unsigned(__stack) - unsigned(sram);
    __builtin_memset(sram, 0, size);
    sram = __stack + kStackWords;
    size = unsigned(&__sram_end) - unsigned(sram);
    __builtin_memset(sram, 0, size);

    auto* dataFlash = &__data_flash_begin;
    auto* dataSRAM = &__data_sram_begin;
    auto dataSize = unsigned(&__data_sram_end) - unsigned(dataSRAM);
    __builtin_memcpy(dataSRAM, dataFlash, dataSize);

    // Run static initializers
    auto* init = reinterpret_cast<vfunc*>(&__init_array_begin);
    auto* initEnd = reinterpret_cast<vfunc*>(&__init_array_end);
    for (; init < initEnd; ++init) { (**init)(); }

    // Call user's entry function
    ::__start();
}
}
