#include "World/Actor.h"

#include "Shared/Assertion.h"

#include "Runtime/Scene.h"

namespace Engine::World
{
    Handle SpawnActor(Context& Context, const Transform& Transform, Handle OwningActor)
    {
        AssertOrErrorCall(Context.IsSceneValid(), return NullActor, "Invalid context")
        
        return Context._GetScene()->SpawnActor(Transform, OwningActor);
    }

    bool IsValidActor(const Context& Context, Handle Actor)
    {
        AssertOrErrorCall(Context.IsSceneValid(), return false, "Invalid context")
        
        return Context._GetScene()->IsValidActor(Actor);
    }

    void DestroyActor(Context& Context, Handle Actor)
    {
        AssertOrErrorCall(Context.IsSceneValid(), return, "Invalid context")

        Context._GetScene()->DestroyActor(Actor);
    }
}
