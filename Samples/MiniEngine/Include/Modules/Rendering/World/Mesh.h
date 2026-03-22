#pragma once

#include "Camera.h"
#include "Modeling/Mesh.h"
#include "World/Component.h"

namespace Rendering::World
{
    struct MeshDrawCall
    {
        Mesh::VertexGroup VertexGroup;
        bool Visible;
    };
    
    struct MeshComponent
    {
        Engine::Handle DrawCall;
    };

    class MeshComponentSystem
    {
    public:
        using Component = MeshComponent;
        static constexpr Engine::World::ECSFeatureFlags Flags = Engine::World::ECS_RequireActorRef;

        void Initialize(Component& Component, Engine::Handle OwningActor);

        void Update(Component& Component, double DeltaTime)
        {
            
        }

        void Terminate(Component& Component);

        void InitializeSystem();
        void UpdateSystem(double DeltaTime)
        {
            CameraComponent& Camera = Engine::World::GetComponentSystem<CameraComponentSystem>().GetCurrentCamera();
            
            CurrentVP = Camera.Camera.Projection() * Camera.Camera.View();
            CurrentInverseVP = Camera.Camera.InverseView() * Camera.Camera.InverseProjection();
        }
        void TerminateSystem();
        
    private:
        Math::Transform4f CurrentVP, CurrentInverseVP;
    };
}
