#pragma once

#include <map>
#include <memory>
#include <span>

#include "Math/RMath.h"
#include "Core/Types.h"
#include "Shared/Annotations.h"

namespace Engine::World
{   
    static constexpr Handle NullActor = std::numeric_limits<Handle>::max();
    
        enum ECSFeatureFlags
    {
        ECS_Core =              0,
        ECS_RequireActorRef =   1 << 0
    };
    
    INLINE consteval ECSFeatureFlags operator|(ECSFeatureFlags Flag1, ECSFeatureFlags Flag2) {return static_cast<ECSFeatureFlags>(static_cast<uint32_t>(Flag1) | static_cast<uint32_t>(Flag2));}
    INLINE consteval ECSFeatureFlags operator&(ECSFeatureFlags Flag1, ECSFeatureFlags Flag2) {return static_cast<ECSFeatureFlags>(static_cast<uint32_t>(Flag1) & static_cast<uint32_t>(Flag2));}
    
    
    template<typename ComponentSystem> concept HasComponent =               requires {typename ComponentSystem::Component; };
    
    template<typename ComponentSystem> concept HasECSFlags =                requires {{ ComponentSystem::Flags } -> std::same_as<ECSFeatureFlags>; };
    
    template<typename CS> concept ComponentSystem =            requires { {std::default_initializable<CS> || std::is_default_constructible_v<CS> }; HasComponent<CS>; HasECSFlags<CS>; };
    
    
    template<typename ComponentSystem> concept HasInitializeComponent =     requires {{static_cast<void (ComponentSystem::*)(typename ComponentSystem::Component&, Handle)>(&ComponentSystem::Initialize)};};
    
    template<typename ComponentSystem> concept HasUpdateComponent =         requires {{static_cast<void (ComponentSystem::*)(typename ComponentSystem::Component&, double)>(&ComponentSystem::Update)};};
    
    template<typename ComponentSystem> concept HasTerminateComponent =      requires {{static_cast<void (ComponentSystem::*)(typename ComponentSystem::Component&)>(&ComponentSystem::Terminate)};};
    
    template<typename ComponentSystem> concept HasInitializeSystem =        requires {{static_cast<void (ComponentSystem::*)()>(&ComponentSystem::InitializeSystem)};};
    
    template<typename ComponentSystem> concept HasUpdateSystem =            requires {{static_cast<void (ComponentSystem::*)(double)>(&ComponentSystem::UpdateSystem)};};
    
    template<typename ComponentSystem> concept HasTerminateSystem =         requires {{static_cast<void (ComponentSystem::*)()>(&ComponentSystem::TerminateSystem)};};
    
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
            static constexpr TypeHash TypeIDHash = ctti::type_id<Component>().hash();
        
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
                m_Components[Component].ActorRef = ActorRef<Flags & ECS_RequireActorRef>(OwningActor);
            
                if constexpr (HasInitializeComponent<CS>)
                {
                    m_System.Initialize(m_Components[Component].Component, OwningActor);
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
                
                        m_System.Update(m_Components[i].Component, DeltaTime);
                    }
                }
            }
        
            void Destroy(Handle Component) override
            {
                if constexpr (HasTerminateComponent<CS>)
                {
                    m_System.Terminate(m_Components[Component].Component);
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
            
            INLINE System& GetSystem() {return m_System;}
        
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
    
    namespace _World
    {
        struct Actor
        {
            struct Component {TypeHash Type; Handle Instance;};
            
            Handle Parent;
            Math::WorldTransformF LocalTransform;
            Math::WorldTransformF WorldTransform;
            std::vector<Component> Components;
            std::vector<Handle> Children;
            bool IsValid;
        };
        
        class Scene
        {
        public:
            template<ComponentSystem CS>
            void RegisterComponentSystem()
            {
                TypeHash SystemID = ctti::type_id<CS>().hash();
                TypeHash ComponentID = ctti::type_id<CS::Component>().hash();
                
                if (m_CSTypeToSystems.contains(SystemID)) return;
                
                size_t index = m_Systems.size();
                m_Systems.emplace_back(std::make_unique<_ECS::ComponentArray<CS>>());
                m_CSTypeToSystems.emplace(SystemID, index);
                m_CompTypeToSystems.emplace(ComponentID, index);
            }
            
            template<ComponentSystem CS>
            CS& GetComponentSystem()
            {
                TypeHash SystemID = ctti::type_id<CS>().hash();
                
                std::unique_ptr<_ECS::IComponentArray>& System = m_Systems[m_CSTypeToSystems[SystemID]];
                return dynamic_cast<_ECS::ComponentArray<CS>*>(System.get())->GetSystem();
            }
            
            template<typename C>
            C& GetComponent(Handle Component)
            {
                TypeHash ComponentID = ctti::type_id<C>().hash();
                
                std::unique_ptr<_ECS::IComponentArray>& System = m_Systems[m_CompTypeToSystems[ComponentID]];
                return *static_cast<C*>(System->GetComponent(Component));
            }
            
            template<ComponentSystem CS>
            CS::Component& GetComponentFromSystem(Handle Component)
            {
                TypeHash SystemID = ctti::type_id<CS>().hash();

                std::unique_ptr<_ECS::IComponentArray>& System = m_Systems[m_CSTypeToSystems[SystemID]];
                return *static_cast<CS::Component*>(System->GetComponent(Component));
            }
            
            Handle SpawnActor(const Math::WorldTransformF& WorldTransform, Handle OwningActor);
            
            bool IsValidActor(Handle Actor);
            
            template <typename Component>
            Component& SpawnComponentOfClass(Handle OwningActor)
            {
                TypeHash ComponentID = ctti::type_id<Component>().hash();
                Actor& actor = m_Actors[OwningActor];
                
                std::unique_ptr<_ECS::IComponentArray>& System = m_Systems[m_CompTypeToSystems[ComponentID]];
                
                Handle C = System->Spawn(OwningActor);
                
                actor.Components.emplace_back(ComponentID, C);
                
                return GetComponent<Component>(C);
            }
            
            void DestroyActor(Handle Actor);
            
            std::span<const Handle> GetChildren(Handle Actor) {return GetActor(Actor).Children;}
            
            Math::WorldTransformF& GetActorWorldTransform(Handle Actor) {return GetActor(Actor).WorldTransform;}
            
            Math::WorldTransformF& GetActorLocalTransform(Handle Actor) {return GetActor(Actor).LocalTransform;}
            
        private:
            Handle AddOrReuseActor();
            
            INLINE Actor& GetActor(Handle Actor) {return m_Actors[Actor];}
            
            std::vector<Actor> m_Actors;
            std::vector<std::unique_ptr<_ECS::IComponentArray>> m_Systems;
            std::map<TypeHash, size_t> m_CSTypeToSystems;
            std::map<TypeHash, size_t> m_CompTypeToSystems;
        };
        
        // TODO find a way to move
        extern Scene g_Scene;
    }
    
    
}
