#include "Core/Engine.h"

#include "Shared/Assertion.h"

namespace Engine
{
    Engine::Engine(Spec&& spec):
        m_Scene(GetContext()),
        m_Modules(std::move(spec.Modules))
    {
        for (auto & module : m_Modules)
        {
            module.second->m_Context = GetContext();
        }
    }

    IModule* Engine::GetModule(TypeHash ModuleID)
    {
        AssertOrWarnCall(m_Modules.contains(ModuleID), return nullptr, "Could not find module")

        return m_Modules[ModuleID].get();
    }
}
