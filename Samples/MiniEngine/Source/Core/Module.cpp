#include "Core/Module.h"

#include "Core/Engine.h"

namespace Engine
{
    IModule* GetModule(Context& Context, TypeHash ModuleID)
    {
        return Context._GetEngine()->GetModule(ModuleID);
    }

}
