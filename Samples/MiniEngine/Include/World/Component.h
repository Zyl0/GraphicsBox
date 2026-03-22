#pragma once

#include <vector>

#include "Core/Types.h"
#include "Scene.h"

namespace Engine::World
{
    template<ComponentSystem CS>
    INLINE void RegisterComponentSystem() {return _World::g_Scene.RegisterComponentSystem<CS>();}

    template<typename Component>
    INLINE Component& GetComponent(Handle ComponentID) {return _World::g_Scene.GetComponent<Component>(ComponentID);}
    
    template<ComponentSystem CS>
    INLINE CS::Component& GetComponentFromSystem(Handle ComponentID) {return _World::g_Scene.GetComponentFromSystem<CS>(ComponentID);}
    
    template<ComponentSystem CS>
    INLINE CS& GetComponentSystem() {return _World::g_Scene.GetComponentSystem<CS>();}
    
    template <typename Component>
    INLINE Component SpawnComponentOfClass(Handle OwningActor) {return _World::g_Scene.SpawnComponentOfClass<Component>(OwningActor);}
}
