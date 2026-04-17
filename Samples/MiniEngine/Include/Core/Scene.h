#pragma once

#include <map>
#include <memory>
#include <span>
#include <vector>

#include "Context.h"
#include "Math/RMath.h"
#include "Core/Types.h"
#include "ctti/type_id.hpp"
#include "Shared/Annotations.h"

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

    template<typename ComponentSystem> concept HasComponent =               requires {typename ComponentSystem::Component; };
    
    // template<typename ComponentSystem> concept HasECSFlags =                requires {{ ComponentSystem::Flags } -> std::same_as<ECSFeatureFlags>; };
    
    template<typename CS> concept ComponentSystem =                         requires { {std::default_initializable<CS> }; HasComponent<CS>; std::is_base_of_v<IComponentSystem, CS>; }; // || std::is_default_constructible_v<CS>
    
    namespace _ECS
    {
        class IComponentArray
        {
        public:
            virtual void Initialize() = 0;
        
            virtual Handle Spawn(Handle OwningActor) = 0;
        
            virtual void Update(double DeltaTime) {};
        
            virtual void Destroy(Handle Component) = 0;
            
            virtual void Terminate() = 0;
        
            virtual void* GetComponent(Handle Component) = 0;

            virtual bool IsValidComponent(Handle Component) = 0;

            virtual Handle GetActor(Handle Component) = 0;
        };
        
        template<ComponentSystem CS>
        class ComponentArray : public IComponentArray
        {
        public:
            using Component = CS::Component;
            using System = CS;
            
            static constexpr TypeHash TypeIDHash = ctti::type_id<Component>().hash();
            static constexpr bool HasInitializeComponent =     requires {{static_cast<void (CS::*)(typename CS::Component&, Handle)>(&CS::Initialize)};};
            static constexpr bool HasUpdateComponent =         requires {{static_cast<void (CS::*)(typename CS::Component&, Handle, double)>(&CS::Update)};};
            static constexpr bool HasTerminateComponent =      requires {{static_cast<void (CS::*)(typename CS::Component&, Handle)>(&CS::Terminate)};};
            static constexpr bool HasInitializeSystem =        requires {{static_cast<void (CS::*)()>(&CS::InitializeSystem)};};
            static constexpr bool HasUpdateSystem =            requires {{static_cast<void (CS::*)(double)>(&CS::UpdateSystem)};};
            static constexpr bool HasTerminateSystem =         requires {{static_cast<void (CS::*)()>(&CS::TerminateSystem)};};
            
            void Initialize() override
            {
                if constexpr (HasInitializeSystem)
                {
                    m_System.InitializeSystem();
                }
            }
        
            Handle Spawn(Handle OwningActor) override
            {
                Handle Component = AddOrReuse();
                m_Components[Component].Validity = true;
                m_Components[Component].ActorRef = OwningActor;
            
                if constexpr (HasInitializeComponent)
                {
                    m_System.Initialize(m_Components[Component].Component, OwningActor);
                }
            
                return Component;
            }
        
            void Update(double DeltaTime) override
            {
                // Update system
                if constexpr (HasUpdateSystem)
                {
                    m_System.UpdateSystem(DeltaTime);
                }
                
                // Update components
                if constexpr (HasUpdateComponent)
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
                if constexpr (HasTerminateComponent)
                {
                    m_System.Terminate(m_Components[Component].Component);
                }
                
                m_Components[Component].Validity = false;
                m_InvalidComponentCount++;
            }
            
            void Terminate() override
            {
                if constexpr (HasTerminateSystem)
                {
                    m_System.TerminateSystem();
                }
            }
        
            void* GetComponent(Handle Component) override
            {
                return &m_Components[Component];
            }

            bool IsValidComponent(Handle Component) override
            {
                if (Component >= m_Components.size()) return false;

                return m_Components[Component].Validity;
            }
            
            Handle GetActor(Handle Component) override
            {
                if (IsValidComponent(Component)) return NullActor;
                
                return m_Components[Component].ActorRef;
            }
            
            INLINE System& GetSystem() {return m_System;}

            const Context& GetContext() const {return m_Context;}
            Context& GetContext() {return m_Context;}
        
        private:
            Handle AddOrReuse()
            {
                std::unique_ptr<int> pi = std::make_unique<int>(0);
            }

        private:
            struct Instance
            {
                Component Component;
                Handle ActorRef;
                bool Validity;
            };

            Context m_Context;
        
            std::vector<Instance> m_Components;
            size_t m_InvalidComponentCount = 0;
            
            System m_System;
        };
    }

    #define COMPONENT_SYSTEM_EXPOSE_EVENTS(ComponentSystem) friend class Engine::World::_ECS::ComponentArray<ComponentSystem>
    
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
            Scene(Context Context) : m_Context(Context) {}
            
            template<ComponentSystem CS>
            void RegisterComponentSystem()
            {
                TypeHash SystemID = ctti::type_id<CS>().hash();
                TypeHash ComponentID = ctti::type_id<typename CS::Component>().hash();
                
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
            bool IsValidComponent(Handle Component)
            {
                TypeHash ComponentID = ctti::type_id<C>().hash();
                
                std::unique_ptr<_ECS::IComponentArray>& System = m_Systems[m_CompTypeToSystems[ComponentID]];
                return *static_cast<C*>(System->IsValidComponent(Component));
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
            
            bool IsValidActor(Handle Actor) const;
            
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
            
            Transform& GetActorWorldTransform(Handle Actor) {return GetActor(Actor).WorldTransform;}
            
            Transform& GetActorLocalTransform(Handle Actor) {return GetActor(Actor).LocalTransform;}

            const Context& GetContext() const {return m_Context;}
            Context& GetContext() {return m_Context;}
            
        private:
            Handle AddOrReuseActor();
            
            INLINE Actor& GetActor(Handle Actor) {return m_Actors[Actor];}
            
            INLINE const Actor& GetActor(Handle Actor) const {return m_Actors[Actor];}

            Context m_Context;
            
            std::vector<Actor> m_Actors;
            std::vector<std::unique_ptr<_ECS::IComponentArray>> m_Systems;
            std::map<TypeHash, size_t> m_CSTypeToSystems;
            std::map<TypeHash, size_t> m_CompTypeToSystems;
        };
    }
}
