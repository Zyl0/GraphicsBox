#pragma once
#include <type_traits>

#include "Context.h"
#include "Types.h"

namespace Engine
{
    class Spec;
    class IModule
    {
    public:
        friend class Engine;
        
        virtual ~IModule() = default;
        virtual void RegisterDependencies(Spec& spec) = 0;
        
        virtual void RegisterComponents() {}

        virtual void Initialize() {}

        virtual void Tick(double deltaTime) {}

        virtual void Shutdown() {}

        class Context& Context() {return m_Context;}
        const class Context& Context() const {return m_Context;}
        
    private:
        void SetContext(const class Context& context) {m_Context = context;}
        
        class Context m_Context = Context();
    };

    template<class Module> requires std::is_base_of_v<IModule, Module>
    TypeHash GetModuleID()
    {
        return ctti::type_id<Module>().hash();
    }

    IModule* GetModule(Context& Context, TypeHash ModuleID);

    template<class Module> requires std::is_base_of_v<IModule, Module>
    Module* GetModule(Context& Context)
    {
        IModule* Inst = GetModule(Context, GetModuleID<Module>());

        return dynamic_cast<Module*>(Inst);
    }
}
