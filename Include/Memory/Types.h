#pragma once
#include <cstdint>

#include "Shared/Annotations.h"

INLINE constexpr uint64_t operator ""_KB(uint64_t Size)
{
    return Size * static_cast<uint64_t>(1024);
}

INLINE constexpr uint64_t operator ""_MB(uint64_t Size)
{
    return Size * 1024_KB;
}

INLINE constexpr uint64_t operator ""_GB(uint64_t Size)
{
    return Size * 1024_MB;
}

INLINE constexpr bool IsPowerOfTwo(uintptr_t x)
{
    return (x & (x-1)) == 0;
}

namespace Memory
{
    struct Block
    {
        Block(): Address(nullptr), Size(0) {}
        Block(void* ptr, size_t size): Address(ptr), Size(size) {}

        void* Address = nullptr;
        size_t Size = 0;
    };

    enum AllocType : uint8_t
    {
        AT_Physical,
        AT_Virtual
    };
    
    uintptr_t AlignForward(uintptr_t ptr, size_t align);

    size_t AlignedSize(uintptr_t ptr, size_t size, size_t Alignment);

    using SizeType = size_t;
    using AlignmentType = uint16_t;
    static constexpr size_t kAlignmentMax = UINT16_MAX;
    static constexpr size_t kDefaultAlignment = 8;
}

