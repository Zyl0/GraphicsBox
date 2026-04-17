#include "Modules/Rendering/World/Camera.h"

#include "World/Component.h"

namespace Rendering::World
{
    using namespace Engine::World;
    
    void CameraComponentSystem::SetCurrentCamera(Engine::Handle Camera)
    {
        if (!IsValidComponent(Camera)) return;

        m_CurrentCamera = Camera;
    }

    CameraComponent& CameraComponentSystem::GetCurrentCamera()
    {
        return GetComponentFromSystem<CameraComponentSystem>(GetContext(), m_CurrentCamera);
    }
}
