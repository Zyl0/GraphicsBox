#pragma once

#include "Camera/FlyCamera.h"

#include "Core/Types.h"
#include "Core/Scene.h"
#include "World/Actor.h"

namespace Rendering::World
{
    struct CameraComponent
    {
        FlyCamera Camera;
    };

    class CameraComponentSystem : public Engine::World::IComponentSystem
    {
    public:
        COMPONENT_SYSTEM_EXPOSE_EVENTS(CameraComponentSystem);
        using Component = CameraComponent;

        void SetCurrentCamera(Engine::Handle Camera);

        CameraComponent& GetCurrentCamera();

    private:
        void Initialize(Component& Component, Engine::Handle OwningActor)
        {
            
        }

        void Update(Component& Component, Engine::Handle OwningActor, double DeltaTime)
        {
            GetContext();
            Engine::World::IsValidActor(GetContext(), OwningActor);
        }

        void Terminate(Component& Component, Engine::Handle OwningActor)
        {
            
        }

        void InitializeSystem()
        {
            
        }
        
        void UpdateSystem(double DeltaTime)
        {
        }
        
        void TerminateSystem()
        {
            
        }
        
    private:
        Engine::Handle m_CurrentCamera;
    };
}
