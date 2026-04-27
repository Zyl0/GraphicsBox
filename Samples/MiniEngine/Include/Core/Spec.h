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
            TypeHash ID = ctti::type_id<T>().hash();
            if (Modules.contains(ID))
            {
                if (CurrentReferencer != 0)
                {
                    PromoteInUpdateList(ID, CurrentReferencer);
                    RegisterDependencies(ID);
                }

                return;
            }
            
            auto it = Modules.try_emplace(ctti::type_id<T>().hash(), std::make_unique<T>());
            
            if (CurrentReferencer != 0)
            {
                PromoteInUpdateList(ID, CurrentReferencer);
            }
            else
            {
                UpdateOrder.push_back(ID);
            }
            
            RegisterDependencies(ID);
        }
    
    private:
        void RegisterDependencies(TypeHash ID)
        {
            TypeHash Referencer = CurrentReferencer;
            CurrentReferencer = ID;
            
            Modules[ID]->RegisterDependencies(*this);

            CurrentReferencer = Referencer;
        }
        
        // Push the id up in the update list so it gets updated before the referencer in the update list
        void PromoteInUpdateList(TypeHash ID, TypeHash Referencer)
        {
            auto InstIt = std::find(UpdateOrder.begin(), UpdateOrder.end(), ID);
            auto RefIt = std::find(UpdateOrder.begin(), UpdateOrder.end(), Referencer);

            size_t InstIndex = std::distance(UpdateOrder.begin(), InstIt);
            size_t RefIndex = std::distance(UpdateOrder.begin(), RefIt);

            if (RefIndex > InstIndex) return;

            UpdateOrder.erase(InstIt);
            UpdateOrder.insert(RefIt, ID);
        }
        
        std::unordered_map<TypeHash, std::unique_ptr<IModule>> Modules;
        std::list<TypeHash> UpdateOrder;

        TypeHash CurrentReferencer = 0;
    };
}
