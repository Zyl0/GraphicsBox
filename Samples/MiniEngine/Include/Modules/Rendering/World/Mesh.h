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

    class MeshComponentSystem : public Engine::World::IComponentSystem
    {
    public:
        using Component = MeshComponent;

        void Initialize(Component& Component, Engine::Handle OwningActor);

        void Update(Component& Component, Engine::Handle OwningActor, double DeltaTime)
        {
            
        }

        void Terminate(Component& Component, Engine::Handle OwningActor);

        void InitializeSystem();
        void UpdateSystem(double DeltaTime)
        {
            CameraComponent& Camera = Engine::World::GetComponentSystem<CameraComponentSystem>(GetContext()).GetCurrentCamera();
            
            CurrentVP = Camera.Camera.Projection() * Camera.Camera.View();
            CurrentInverseVP = Camera.Camera.InverseView() * Camera.Camera.InverseProjection();
        }
        void TerminateSystem();
        
    private:
        Math::Transform4f CurrentVP, CurrentInverseVP;
    };
}
