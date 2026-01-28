#pragma once

#include <rp2350/clocks.h>
#include <rp2350/common.h>

namespace rp2350::sys {

// Global config
constexpr static uint64_t kXOSC = 12'000'000;
constexpr static uint64_t kSysHz = 150'000'000;
constexpr static uint64_t kFBDiv = 125;
constexpr static uint64_t kDiv1 = 5;
constexpr static uint64_t kDiv2 = 2;

// Verify
static_assert(16 <= kFBDiv && kFBDiv <= 320);
static_assert(1 <= kDiv1 && kDiv1 <= 7);
static_assert(1 <= kDiv2 && kDiv2 <= 7);
static_assert(kDiv1 >= kDiv2);
static_assert(kXOSC * kFBDiv >= 750'000'000);
static_assert(kXOSC * kFBDiv <= 1600'000'000);
static_assert(kXOSC * kFBDiv / (kDiv1 * kDiv2) == kSysHz);

} // namespace rp2350::sys
