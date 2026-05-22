#pragma once

#include "Core/Module.h"
#include <vector>
#include <chrono>

namespace Profiling
{
    void foo()
    {
        auto start = std::chrono::high_resolution_clock::now();

        auto end = std::chrono::high_resolution_clock::now();

        auto dtms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    }

    class Module : public Engine::IModule
    {
    public:
        Module() = default;
        ~Module() override = default;

        void RegisterDependencies(Engine::Spec& spec) override;

        void Initialize() override;

        void Tick(double deltaTime) override;

        void Shutdown() override;

        void ResetTimers();

        void DisplayTimers();

    private:
        std::vector<std::chrono::milliseconds> m_Timers;
    }
}