#pragma once

#include "Spec.h"
#include "Scene.h"
#include "Runtime/Scene.h"

namespace Engine
{
    class Engine
    {
    public:
        friend class App;
        
        Engine(Spec&& spec);

        IModule* GetModule(TypeHash ModuleID);
        std::string_view GetModuleName(TypeHash ModuleID);
        
        template<class Module> requires std::is_base_of_v<IModule, Module>
        Module* GetModule()
        {
            IModule* Inst = GetModule(GetModuleID<Module>());

            return dynamic_cast<Module*>(Inst);
        }
        
        template<class Module> requires std::is_base_of_v<IModule, Module>
        INLINE std::string_view GetModuleName()
        {
            return GetModuleName(GetModuleID<Module>());
        }
        
        Context GetContext()
        {
            Context ctx = {};
            ctx.m_Engine = this;
            ctx.m_Scene = &m_Scene;
            return ctx;
        }
        
    private:
        void InitializeScene() const;
        void TickScene(double DeltaTime) const;
        void TerminateScene() const;
        
        World::_World::Scene m_Scene;
        std::unordered_map<TypeHash, std::string_view> m_ModuleNames;
        std::unordered_map<TypeHash, std::unique_ptr<IModule>> m_Modules;
        std::vector<TypeHash> m_UpdateOrder;
    };
}
