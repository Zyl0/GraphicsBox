#pragma once

#include <ctti/type_id.hpp>

#include "Math/RMath.h"

namespace Engine
{
    using Handle = size_t;
    using TypeHash = ctti::detail::hash_t;
    using Transform = Math::WorldTransformF;
}
