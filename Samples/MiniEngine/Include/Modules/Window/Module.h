#pragma once
#include "Core/Module.h"

namespace Window
{
    class Module : public Engine::IModule
    {
    public:
        Module() = default;
        ~Module() override {}
        void RegisterDependencies(Engine::Spec& spec) override {}
        
        void Initialize() override;

        void Tick(double deltaTime) override;

        void Shutdown() override;
        
        bool ShouldClose();
        
        bool HasFocus();
        
        INLINE bool IsNotReduced() const {return m_IsReduced;}
        
        INLINE bool ShouldResize() const {return m_ShouldResize;}
        
        bool GetFrameBufferSize(uint32_t& width, uint32_t& height);
        
#ifdef WINDOW_GLFW
        bool GLFWGetKey(int code);
#endif // WINDOW_GLFW
        
    private:
        void* m_Window;
        uint32_t m_Width, m_Height;
        bool m_IsReduced;
        bool m_ShouldResize;
    };
}
