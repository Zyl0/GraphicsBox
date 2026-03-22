#pragma once

#include "Camera/FlyCamera.h"
#include "World/Component.h"

namespace Rendering::World
{
    struct CameraComponent
    {
        FlyCamera Camera;
    };

    class CameraComponentSystem
    {
    public:
        using Component = CameraComponent;
        static constexpr Engine::World::ECSFeatureFlags Flags = Engine::World::ECS_RequireActorRef;

        void Initialize(Component& Component, Engine::Handle OwningActor);

        void Update(Component& Component, double DeltaTime);

        void Terminate(Component& Component);

        void InitializeSystem();
        void UpdateSystem(double DeltaTime)
        {
            
        }
        void TerminateSystem();
        
        CameraComponent& GetCurrentCamera();

    private:
        Engine::Handle m_CurrentCamera;
    };
}
