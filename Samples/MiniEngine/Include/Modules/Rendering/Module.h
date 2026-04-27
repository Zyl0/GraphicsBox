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
        
        INLINE int OpenGLVersionMajor() {return 4;}
        INLINE int OpenGLVersionmMinor() {return 5;}
        INLINE bool OpenGLUseCoreProfile() {return false;}

        void EnableMSAA();

        void Initialize() override;

        void Tick(double deltaTime) override;

        // Not needed
        // void Shutdown() override;

    private:
    };
}