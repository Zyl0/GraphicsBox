#pragma once

#include <map>
#include <memory>
#include <span>
#include <vector>

#include "Context.h"
#include "Math/RMath.h"
#include "Core/Types.h"

namespace Engine::World
{   
    static constexpr Handle NullActor = std::numeric_limits<Handle>::max();
    
    // enum ECSFeatureFlags
    // {
    //     ECS_Core =              0,
    //     ECS_RequireActorRef =   1 << 0
    // };
    //
    // INLINE consteval ECSFeatureFlags operator|(ECSFeatureFlags Flag1, ECSFeatureFlags Flag2) {return static_cast<ECSFeatureFlags>(static_cast<uint32_t>(Flag1) | static_cast<uint32_t>(Flag2));}
    // INLINE consteval ECSFeatureFlags operator&(ECSFeatureFlags Flag1, ECSFeatureFlags Flag2) {return static_cast<ECSFeatureFlags>(static_cast<uint32_t>(Flag1) & static_cast<uint32_t>(Flag2));}
    

    namespace _ECS
    {
        class IComponentArray;
    }
    
    namespace _World
    {
        class Scene;
    }
    
    class IComponentSystem
    {
    public:
        friend class _World::Scene;
        friend class _ECS::IComponentArray;

        const Context& GetContext() const {return m_Context;}
        Context& GetContext() {return m_Context;}

    protected:
        Handle GetActor(Handle Component) const;

        bool IsValidComponent(Handle Component) const;

        void* GetComponent(Handle Component) const;

    private:
        void Link(const Context& Context, _ECS::IComponentArray* Array);

        Context m_Context = Context();
        _ECS::IComponentArray* m_Array = nullptr;
    };

    #define COMPONENT_SYSTEM_EXPOSE_EVENTS(ComponentSystem) friend class Engine::World::_ECS::ComponentArray<ComponentSystem>
    
    namespace _ECS
    {
        template<typename CS>
        class ComponentArray;
    }
}
