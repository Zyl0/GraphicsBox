#pragma once

#include "Modeling/Mesh.h"

#include "Core/Scene.h"

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
        COMPONENT_SYSTEM_EXPOSE_EVENTS(MeshComponentSystem);
        using Component = MeshComponent;

        // void Initialize(Component& Component, Engine::Handle OwningActor);

        // void Update(Component& Component, Engine::Handle OwningActor, double DeltaTime);

        //  void Terminate(Component& Component, Engine::Handle OwningActor);

        // void InitializeSystem();
        void UpdateSystem(double DeltaTime);
        // void TerminateSystem();
        
    private:
        Math::Transform4f CurrentVP, CurrentInverseVP;
    };
}
