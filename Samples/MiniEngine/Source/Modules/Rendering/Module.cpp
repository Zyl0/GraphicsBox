#include "Modules/Rendering/Module.h"

// Dependency module
#include "Modules/Window/Module.h"

// Added components
#include "Modules/Rendering/World/Camera.h"
#include "Modules/Rendering/World/Mesh.h"

#include "Core/Spec.h"
#include "World/Component.h"

void Rendering::Module::RegisterDependencies(Engine::Spec& spec)
{
    spec.Register<Window::Module>();
}

void Rendering::Module::RegisterComponents()
{
    // Engine::World::RegisterComponentSystem<World::CameraComponentSystem>(Context());
    // Engine::World::RegisterComponentSystem<World::MeshComponentSystem>(Context());
    // Engine::World::RegisterComponentSystem<World::CameraComponentSystem>(Context());
}
