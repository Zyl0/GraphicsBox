#pragma once

#include "Core/Module.h"

namespace Rendering
{
    // Base renderer
    // Made to be overriden
    class Module : public Engine::IModule
    {
    public:
        Module() = default;
        ~Module() override = default;
        
        void RegisterDependencies(Engine::Spec& spec) override;

        void RegisterComponents() override;
    };
}