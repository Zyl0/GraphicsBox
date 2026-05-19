#pragma once
#include "Core/Module.h"

namespace Editor
{
    class Module : public Engine::IModule
    {
    public:
        ~Module() override = default;
        void RegisterDependencies(Engine::Spec& spec) override;
        void EditorUI() override;
    };
}
