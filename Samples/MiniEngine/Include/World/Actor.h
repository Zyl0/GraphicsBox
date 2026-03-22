#pragma once

#include <vector>

#include "Math/RMath.h"
#include "Scene.h"

namespace Engine::World
{
    INLINE Handle SpawnActor(const Math::WorldTransformF& WorldTransform, Handle OwningActor = NullActor) {return _World::g_Scene.SpawnActor(WorldTransform, OwningActor);}
    
    INLINE bool IsValidActor(Handle Actor) {return _World::g_Scene.IsValidActor(Actor);}
    
    void DestroyActor(Handle Actor);
}
