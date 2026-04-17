#include "Core/Scene.h"

#include "Math/RMath.h"
#include "Shared/Assertion.h"

namespace Engine::World
{
    Handle _World::Scene::SpawnActor(const Math::WorldTransformF& WorldTransform, Handle OwningActor)
    {
        Handle ActorHandle = AddOrReuseActor();
        
        Actor& actor = GetActor(ActorHandle);
        actor.LocalTransform = WorldTransform;
        actor.Parent = OwningActor;
        
        if (OwningActor == NullActor)
        {
            actor.WorldTransform = WorldTransform;
        }
        else
        {
            actor.WorldTransform = GetActorWorldTransform(OwningActor) * WorldTransform;
        }
        
        return ActorHandle;
    }

    bool _World::Scene::IsValidActor(Handle Actor) const
    {
        return GetActor(Actor).IsValid;
    }

    void _World::Scene::DestroyActor(Handle ActorID)
    {
        Actor& actor = GetActor(ActorID);
        
        // Destroy children
        for (Handle child : actor.Children)
        {
            DestroyActor(child);
        }
        actor.Children.clear();
        
        // Release components
        for (Actor::Component& component : actor.Components)
        {
            m_Systems[m_CompTypeToSystems[component.Type]]->Destroy(component.Instance);
        }
        actor.Components.clear();
        
        // Release actor object
        actor.IsValid = false;
    }

    Handle IComponentSystem::GetActor(Handle Component) const
    {
        return m_Array->GetActor(Component);
    }

    bool IComponentSystem::IsValidComponent(Handle Component) const
    {
        return m_Array->IsValidComponent(Component);
    }

    void* IComponentSystem::GetComponent(Handle Component) const
    {
        return m_Array->GetComponent(Component);
    }

    void IComponentSystem::Link(const Context& Context, _ECS::IComponentArray* Array)
    {
        m_Context = Context;
        m_Array = Array;
    }
}
