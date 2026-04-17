#pragma once

#include "Spec.h"
#include "Scene.h"
#include "Runtime/Scene.h"

namespace Engine
{
    class Engine
    {
    public:
        Engine(Spec&& spec);

        IModule* GetModule(TypeHash ModuleID);

        template<class Module> requires std::is_base_of_v<IModule, Module>
        Module* GetModule()
        {
            IModule* Inst = GetModule(GetModuleID<Module>());

            return dynamic_cast<Module*>(Inst);
        }
        
        Context GetContext()
        {
            Context ctx = {};
            ctx.m_Engine = this;
            ctx.m_Scene = &m_Scene;
            return ctx;
        }
    private:
        World::_World::Scene m_Scene;
        std::unordered_map<TypeHash, std::unique_ptr<IModule>> m_Modules;
    };
}
