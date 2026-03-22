#pragma once

#include <map>
#include <memory>

#include "Component.h"
#include "Actor.h"

namespace Engine::World
{    
    class Scene
    {
    public:
        template<ComponentSystem CS>
        void RegisterComponentSystem()
        {
            TypeHash SystemID = ctti::type_id<CS>();
            TypeHash ComponentID = ctti::type_id<CS::Component>();
            
            m_Systems.emplace_back(std::make_unique<CS>());
        }
        
        template<typename ComponentSystem>
        ComponentSystem& GetComponentSystem()
        {
            TypeHash SystemID = ctti::type_id<ComponentSystem>();
            
            return *dynamic_cast<ComponentSystem*>(m_Systems[m_CSTypeToSystems[SystemID]].get());
        }
        
        
        
    private:
        std::vector<std::unique_ptr<_ECS::IComponentArray>> m_Systems;
        std::map<TypeHash, size_t> m_CSTypeToSystems;
        std::map<TypeHash, size_t> m_CompTypeToSystems;
    };
    
    extern Scene g_Scene;

    template<typename Component>
    Component& GetComponent(Handle ComponentID);
    // {
    //     TypeHash TypeID = ctti::type_id<Component>();
    //     
    //     // Find array from type ID
    //     IComponentArray& Array = /* ... */;
    //     
    //     return *static_cast<Component*>(Array.GetComponent(ComponentID));
    // }
    
    template<typename ComponentSystem>
    ComponentSystem::Component& GetComponentFromSystem(Handle ComponentID);
    // {
    //     IComponentArray& Array = /* ... */;
    //     
    //     return *static_cast<ComponentSystem::Component*>(Array.GetComponent(ComponentID));
    // }
    
    template<typename ComponentSystem>
    ComponentSystem& GetComponentSystem();
    
}
