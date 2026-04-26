#pragma once

#include <memory>
#include <type_traits>
#include <unordered_map>

#include "Core/Types.h"
#include "Core/Module.h"

namespace Engine
{
    // A spec is the specifications of module an engine have to contain
    class Spec
    {
    public:
        friend class Engine;
        
        template<class T> requires(std::is_base_of_v<IModule, T>) // && (std::is_default_constructible_v<T>() || std::is_trivially_constructible_v<T>()))
        void Register()
        {
            if (Modules.contains(ctti::type_id<T>().hash())) return;
            
            auto it = Modules.try_emplace(ctti::type_id<T>().hash(), std::make_unique<T>());
            
            it.first->second->RegisterDependencies(*this);
        }
    
    private:
        std::unordered_map<TypeHash, std::unique_ptr<IModule>> Modules;
    };
}
