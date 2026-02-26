#pragma once

#pragma once

using int8_t   = char;
using int16_t  = short;
using int32_t  = long;
using int64_t  = long long;
using uint8_t  = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned long;
using uint64_t = unsigned long long;
static_assert(sizeof(int8_t) == 1);
static_assert(sizeof(int16_t) == 2);
static_assert(sizeof(int32_t) == 4);
static_assert(sizeof(int64_t) == 8);
static_assert(sizeof(uint8_t) == 1);
static_assert(sizeof(uint16_t) == 2);
static_assert(sizeof(uint32_t) == 4);
static_assert(sizeof(uint64_t) == 8);

using intptr_t  = int32_t;
using uintptr_t = uint32_t;

using size_t = uint32_t;
static_assert(sizeof(size_t) == sizeof(void*));

using ssize_t   = int32_t;
using ptrdiff_t = int32_t;

typedef void (*vfunc)();

void* operator new(unsigned, void* ptr) noexcept;

extern "C" {
void __cxa_pure_virtual();
void __cxa_deleted_virtual();
}

namespace __cxxabiv1 {} // namespace __cxxabiv1

extern "C" {

// C/C++ ABI-specified functions

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memcpy(uint8_t* dest, uint8_t const* src, uint32_t n) {
    for (uint32_t i = 0; i < n; i++) {
        // Read from source and write into dest; the do-nothing `asm volatile`
        // is only to separate the read and write, to prevent fusing them and optimizing
        // into a "memcpy" operation involving a call to `__aeabi_memcpy`, the very
        // thing we're trying to define.
        uint8_t x = src[i];
        // asm volatile("");  // XXX actually needed??
        dest[i]   = x;
    }
}

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memcpy4(uint8_t* dest, uint8_t const* src, unsigned n) {
    __aeabi_memcpy(dest, src, n);
}

inline void* memcpy(void* dst_, void const* src_, size_t n) {
    auto* dst = reinterpret_cast<char*>(dst_);
    auto* src = reinterpret_cast<char const*>(src_);
    for (size_t i = 0; i < n; i++) { dst[i] = src[i]; }
    return dst_;
}

inline void* memset(void* dst_, int c_, size_t len) {
    auto* dst = reinterpret_cast<char*>(dst_);
    auto c    = char(c_);
    for (size_t i = 0; i < len; i++) { dst[i] = c; }
    return dst_;
}

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memclr(void* dest, size_t n) {
    memset(dest, 0, n);
}

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memclr4(void* dest, size_t n) {
    memset(dest, 0, n);
}

[[gnu::used]] [[gnu::retain]]
inline void __aeabi_memclr8(void* dest, size_t n) {
    memset(dest, 0, n);
}

} // extern "C" ends
