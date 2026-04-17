#pragma once

#include "Core/Context.h"
#include "Core/Types.h"
#include "Core/Scene.h"

namespace Engine::World
{
    Handle SpawnActor(Context& Context, const Transform& Transform, Handle OwningActor = NullActor);
    
    bool IsValidActor(const Context& Context, Handle Actor);

    void DestroyActor(Context& Context, Handle Actor);

    const Transform& GetWorldTransform(const Context& Context, Handle Actor);

    const Transform& GetLocalTransform(const Context& Context, Handle Actor);

    const Math::Point3f GetWorldPosition(const Context& Context, Handle Actor);
    
    const Math::Point3f GetLocalPosition(const Context& Context, Handle Actor);

    void SetWorldPosition(const Context& Context, Handle Actor, Math::Point3f Location);

    void AddWorldPosition(const Context& Context, Handle Actor, Math::Vector3f Displacement);

    void SetLocalPosition(const Context& Context, Handle Actor, Math::Point3f Location);

    void AddLocalPosition(const Context& Context, Handle Actor, Math::Vector3f Displacement);

    void SetWorldRotation(const Context& Context, Handle Actor, Math::QuaternionF Rotation);
    
    void SetLocalRotation(const Context& Context, Handle Actor, Math::QuaternionF Rotation);

    void SetWorldScale(const Context& Context, Handle Actor, Math::Vector3f Scale);

    void SetLocalScale(const Context& Context, Handle Actor, Math::Vector3f Scale);
}
