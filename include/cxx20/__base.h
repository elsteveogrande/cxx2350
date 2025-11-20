#pragma once

// Declare these basic types into the global namespace.

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

using ssize_t   = int32_t;
using size_t    = uint32_t;
using ptrdiff_t = int32_t;

static_assert(sizeof(size_t) == sizeof(void*));

typedef void (*vfunc)();
