#include "Modules/Rendering/World/Camera.h"

namespace Rendering::World
{
    CameraComponent& CameraComponentSystem::GetCurrentCamera()
    {
        return Engine::World::GetComponentFromSystem<CameraComponentSystem>(m_CurrentCamera);
    }
}
