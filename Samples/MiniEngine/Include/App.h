#pragma once

#include "Core/Engine.h"

namespace Engine
{
    class App
    {
    public:
        App(Spec&& Specification) : m_Engine(std::move(Specification)) {}
        
        void Run();
        
    private:
        Engine m_Engine;
    };
}