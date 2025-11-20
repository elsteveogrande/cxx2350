#pragma once

namespace rp2 {

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned long;

using uv32  = u32 volatile;
using uint  = u32;
using uvint = uint volatile;

using uptr = u32*;
typedef void (*vfunc)();

} // namespace rp2
