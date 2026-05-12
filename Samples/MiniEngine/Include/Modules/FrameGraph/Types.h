#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "ctti/nameof.hpp"

#include "Shared/Annotations.h"
#include "Math/Vector.h"

namespace FrameGraph
{
    using Name = std::uint64_t;

    using Location = size_t;

    using Bool = bool;

    using UInt = uint32_t;

    using Int = int;

    using Float = float;

    using Size2D = Math::Vector2t<uint32_t>;

    // Box 2D with an offset and a size as unsigned ints
    struct Rect {Math::Vector2t<uint32_t> Position; Math::Vector2t<uint32_t> Size;};

    // Convert a string to a name
    INLINE Name ToName(std::string_view Name) {return std::hash<std::string_view>{}(Name);}
}