#pragma once

#include <vector>

#include "Actor.h"
#include "Core/Types.h"

namespace Engine::World
{
    enum ECSFeatureFlags
    {
        ECS_Core =              0,
        ECS_RequireActorRef =   1 << 0
    };
    
    INLINE consteval ECSFeatureFlags operator|(ECSFeatureFlags Flag1, ECSFeatureFlags Flag2) {return static_cast<ECSFeatureFlags>(static_cast<uint32_t>(Flag1) | static_cast<uint32_t>(Flag2));}
    INLINE consteval ECSFeatureFlags operator&(ECSFeatureFlags Flag1, ECSFeatureFlags Flag2) {return static_cast<ECSFeatureFlags>(static_cast<uint32_t>(Flag1) & static_cast<uint32_t>(Flag2));}
    
    
    template<typename ComponentSystem> concept HasComponent =               requires {typename ComponentSystem::Component; };
    
    template<typename ComponentSystem> concept HasECSFlags =                requires {{ ComponentSystem::Flags } -> std::same_as<ECSFeatureFlags>; };
    
    template<typename ComponentSystem> concept ComponentSystem =            requires { {std::default_initializable<ComponentSystem> || std::is_default_constructible_v<ComponentSystem> }; HasComponent<ComponentSystem>; HasECSFlags<ComponentSystem>; };
    
    
    template<typename ComponentSystem> concept HasInitializeComponent =     requires {{static_cast<void (ComponentSystem::*)(typename ComponentSystem::Component&, Handle)>(&ComponentSystem::Initialize)};};
    
    template<typename ComponentSystem> concept HasUpdateComponent =         requires {{static_cast<void (ComponentSystem::*)(typename ComponentSystem::Component&, double)>(&ComponentSystem::Update)};};
    
    template<typename ComponentSystem> concept HasTerminateComponent =      requires {{static_cast<void (ComponentSystem::*)(typename ComponentSystem::Component&)>(&ComponentSystem::Terminate)};};
    
    template<typename ComponentSystem> concept HasInitializeSystem =        requires {{static_cast<void (ComponentSystem::*)()>(&ComponentSystem::InitializeSystem)};};
    
    template<typename ComponentSystem> concept HasUpdateSystem =            requires {{static_cast<void (ComponentSystem::*)(double)>(&ComponentSystem::UpdateSystem)};};
    
    template<typename ComponentSystem> concept HasTerminateSystem =         requires {{static_cast<void (ComponentSystem::*)()>(&ComponentSystem::TerminateSystem)};};
    
    
    /* ____________________________________ System ____________________________________ */
    
    namespace _ECS
    {        
        class IComponentArray
        {
        public:
            virtual TypeHash TypeID() const = 0;
            
            virtual void Initialize() = 0;
        
            virtual Handle Spawn(Handle OwningActor) = 0;
        
            virtual void Update(double DeltaTime) {};
        
            virtual void Destroy(Handle Component) = 0;
            
            virtual void Terminate() = 0;
        
            virtual void* GetComponent(Handle Component) = 0;
        };
    
        
        template <bool Enable = true>
        struct ActorRef
        {
            ActorRef(Handle OwningActor) : Ref(OwningActor) {}
            
            Handle Get() const {return Ref;}
            
        private:
            Handle Ref;
        };
        
        template <>
        struct ActorRef<false>
        {
            ActorRef(Handle OwningActor) {}
            
            Handle Get() const;
        };
        
        template<ComponentSystem CS>
        class ComponentArray : public IComponentArray
        {
        public:
            using Component = CS::Component;
            using System = CS;
            
            static constexpr ECSFeatureFlags Flags = CS::Flags;
            static constexpr TypeHash TypeIDHash = ctti::type_id<Component>();
        
            TypeHash TypeID() const override {return TypeIDHash;}
            
            void Initialize() override
            {
                if constexpr (HasInitializeSystem<CS>)
                {
                    m_System.InitializeSystem();
                }
            }
        
            Handle Spawn(Handle OwningActor) override
            {
                Handle Component = AddOrReuse();
                m_Components[Component].Validity = true;
            
                if constexpr (HasInitializeComponent<CS>)
                {
                    m_System.Initialize(Component, OwningActor);
                }
            
                return Component;
            }
        
            void Update(double DeltaTime) override
            {
                // Update system
                if constexpr (HasUpdateSystem<CS>)
                {
                    m_System.UpdateSystem(DeltaTime);
                }
                
                // Update components
                if constexpr (HasUpdateComponent<CS>)
                {
                    for (size_t i = 0; i < m_Components.size(); ++i)
                    {
                        if (!m_Components[i].Validity) continue;
                
                        m_System.Update(m_Components[i], DeltaTime);
                    }
                }
            }
        
            void Destroy(Handle Component) override
            {
                if constexpr (HasTerminateComponent<CS>)
                {
                    m_System.Terminate(Component);
                }
                
                m_Components[Component].Validity = false;
            }
            
            void Terminate() override
            {
                if constexpr (HasTerminateSystem<CS>)
                {
                    m_System.TerminateSystem();
                }
            }
        
            void* GetComponent(Handle Component)
            {
                return &m_Components[Component];
            }
        
        private:
            Handle AddOrReuse();
        
            struct Instance
            {
                Component Component;
                _ECS::ActorRef<Flags & ECS_RequireActorRef> ActorRef;
                bool Validity;
            };
        
            std::vector<Instance> m_Components;
        
            System m_System;
        };

        
    }


}
