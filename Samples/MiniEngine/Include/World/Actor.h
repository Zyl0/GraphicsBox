#pragma once

#include <vector>

#include "Math/RMath.h"
#include "Core/Types.h"

namespace Engine::World
{
    static constexpr Handle NullActor = std::numeric_limits<Handle>::max();
    
    struct Actor
    {
        Handle Parent;
        Math::WorldTransformF LocalTransform;
        Math::WorldTransformF WorldTransform;
        std::vector<Handle> Components;
        std::vector<Handle> Children;
    };
}
