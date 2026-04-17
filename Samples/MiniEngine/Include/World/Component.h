#pragma once

#include <vector>

#include "Core/Context.h"
#include "Core/Types.h"
#include "Core/Scene.h"
#include "Shared/Assertion.h"

namespace Engine::World
{
    template<ComponentSystem CS>
    INLINE void RegisterComponentSystem(Context& Context)
    {
        AssertOrErrorCall(Context.IsSceneValid(), return;, "Invalid Context")
        return Context._GetScene()->RegisterComponentSystem<CS>();
    }

    template<typename Component>
    INLINE bool IsValidComponent(Context& Context, Handle ComponentID)
    {
        AssertOrError(Context.IsSceneValid(), "Invalid Context")
        return Context._GetScene()->GetComponent<Component>(ComponentID);
    }

    template<typename Component>
    INLINE Component& GetComponent(Context& Context, Handle ComponentID)
    {
        AssertOrError(Context.IsSceneValid(), "Invalid Context")
        return Context._GetScene()->GetComponent<Component>(ComponentID);
    }
    
    template<ComponentSystem CS>
    INLINE CS::Component& GetComponentFromSystem(Context& Context, Handle ComponentID)
    {
        AssertOrError(Context.IsSceneValid(), "Invalid Context")
        return Context._GetScene()->GetComponentFromSystem<CS>(ComponentID);
    }
    
    template<ComponentSystem CS>
    INLINE CS& GetComponentSystem(Context& Context)
    {
        AssertOrError(Context.IsSceneValid(), "Invalid Context")
        return Context._GetScene()->GetComponentSystem<CS>();
    }
    
    template <typename Component>
    INLINE Component SpawnComponentOfClass(Context& Context, Handle OwningActor)
    {
        AssertOrError(Context.IsSceneValid(), "Invalid Context")
        return Context._GetScene()->SpawnComponentOfClass<Component>(OwningActor);
    }
}
