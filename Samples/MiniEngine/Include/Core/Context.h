#pragma once

#include "Shared/Annotations.h"

namespace Engine
{
    namespace World::_World
    {
        class Scene;
    }
    
    // A context is a view of the engine and components
    class Engine;
    class Context
    {
    public:
        friend Engine;
        Context() = default;
        
        INLINE bool IsValid() const {return m_Engine != nullptr;}
        INLINE const Engine* _GetEngine() const {return m_Engine;}
        INLINE Engine* _GetEngine() {return m_Engine;}
        
        INLINE bool IsSceneValid() const {return IsValid() && m_Scene != nullptr;}
        INLINE const World::_World::Scene* _GetScene() const {return m_Scene;}
        INLINE World::_World::Scene* _GetScene() {return m_Scene;}
        
    private:
        Engine* m_Engine = nullptr;
        World::_World::Scene* m_Scene = nullptr;
    };
}
