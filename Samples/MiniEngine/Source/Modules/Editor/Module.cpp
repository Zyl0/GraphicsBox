#include "Modules/Editor/Module.h"

#include "Core/Spec.h"
#include "Modules/ImGui/Module.h"

namespace Editor
{
    void Module::RegisterDependencies(Engine::Spec& spec)
    {
        spec.Register<ImGui::Module>();
    }

    void Module::EditorUI()
    {
        
    }
}
