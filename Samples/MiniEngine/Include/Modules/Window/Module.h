#pragma once
#include "Core/Module.h"

namespace Window
{
    class Module : public Engine::IModule
    {
    public:
        Module() = default;
        ~Module() override;
        void RegisterDependencies(Engine::Spec& spec) override {}
    };
}
