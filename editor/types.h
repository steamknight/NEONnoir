#pragma once
#include <stdint.h>

namespace NEONnoir
{
    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    // Convenience functions to make casting easier to type and read
    template<typename T, typename U>
    auto to(U value) -> T
    {
        return static_cast<T>(value);
    }

    template<typename T, typename U>
    auto force_to(U value) -> T
    {
        return reinterpret_cast<T>(value);
    }

    template<char... T>
    size_t operator"" _z()
    {
        return to<size_t>(T);
    }
}