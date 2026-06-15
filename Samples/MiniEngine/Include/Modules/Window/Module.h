#pragma once
#include "Core/Module.h"

#include <optional>

// TODO not sure if we want to have the complete Rendering lib as a dependency of the window module, maybe move to raw GL objects
#include "Rendering/Textures.h"
#include "Rendering/FrameBuffers.h"

namespace Window
{
    class Module : public Engine::IModule
    {
    public:
        Module() = default;
        ~Module() override = default;
        void RegisterDependencies(Engine::Spec& spec) override {}
        
        void Initialize() override;

        void Tick(double deltaTime) override;

        void Shutdown() override;

        void EditorUI() override;
        
        bool ShouldClose();
        
        bool HasFocus();
        
        INLINE bool IsNotReduced() const {return !m_IsReduced;}
        
        INLINE bool ShouldResize() const {return m_ShouldResize;}
        
        bool GetFrameBufferSize(uint32_t& width, uint32_t& height);

        bool ShouldRecompileShaders();
        
        void GetMousePosition(double& x, double& y) const;
        void GetMousePositionDelta(double& x, double& y) const;
        
        INLINE GLuint ViewportFrameBuffer() const {return m_IsCurrentViewportSubViewport ? SubViewportFrameBuffer() : MainWindowFrameBuffer();}
        INLINE GLuint MainWindowFrameBuffer() const {return 0;}
        INLINE GLuint SubViewportFrameBuffer() const {return m_SubViewportFrameBuffer.has_value() ? m_SubViewportFrameBuffer->Handle() : 0;}
        INLINE GLuint SubViewportWriteBuffer() const {return m_SubViewportWriteBuffer.has_value() ? m_SubViewportWriteBuffer->Handle() : 0;}
        
#ifdef WINDOW_GLFW
        bool GLFWGetKey(int code) const;
        bool GLFWGetMouseButton(int code) const;
#endif // WINDOW_GLFW

        INLINE void* _Handle() {return m_Window;}
        
        void _EnableSubViewport(uint32_t Width, uint32_t Height);
        void _SetViewportMainWindow();
        void _SetViewportSubViewport(uint32_t Width = 0, uint32_t Height = 0);
        void _DisableSubViewport();
        
    private:
        void* m_Window;
        uint32_t m_Width, m_Height;
        double m_CurrentMouseX, m_CurrentMouseY, m_LastMouseX, m_LastMouseY;
        bool m_IsReduced;
        bool m_ShouldResize;
        
        bool m_IsCurrentViewportSubViewport = false;
        bool m_ShouldResizeSubViewport = false;
        uint32_t m_SubWidth = 0, m_SubHeight = 0;
        
        std::optional<Texture2D> m_SubViewportWriteBuffer;
        std::optional<FrameBuffer> m_SubViewportFrameBuffer;
    };
}
