#include "Modules/Rendering/Module.h"
#include "Modules/Window/Module.h"

#include "Rendering/GLHelper.h"
#ifdef WINDOW_GLFW
#include <GLFW/glfw3.h>
#endif // WINDOW_GLFW

#include "Shared/Assertion.h"

#include "Core/Engine.h"
#include "Modules/Rendering/Module.h"

#include "imgui.h"

namespace Window
{
    // TODO expose
    constexpr size_t kBaseWidth = 1280;
    constexpr size_t kBaseHeight = 720;
    constexpr const char* kBaseWindowName = "Mini Engine";

    // states
    static bool RequestShaderReload = false;
    
#ifdef WINDOW_GLFW
    void error_callback(int error, const char* description)
    {
        EngineLoggerErrorF("GLFW Validation Error: [%d] %s", error, description);
        
        // fprintf(stderr, "Error: %s\n", description); <--- TODO binding to stderr maybe
        
#ifdef CONFIG_DEBUG
        EngineRuntimeBREAKPOINT
#endif // CONFIG_DEBUG
    }
    
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    
        if (key == GLFW_KEY_TAB && action == GLFW_PRESS)
        {
            RequestShaderReload = true;
        }
    }
#endif // WINDOW_GLFW
    
    void Module::Initialize()
    {
        using RMod = Rendering::Module;
        
        RMod* RenderingModule = Engine::GetModule<RMod>(Context());
        
#ifdef WINDOW_GLFW
        GLFWwindow* window = nullptr;
            
        glfwSetErrorCallback(error_callback);
        AssertOrError(glfwInit(), "Failed to initialise GLFW")
        
        if (RenderingModule != nullptr)
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, RenderingModule->OpenGLVersionMajor());
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, RenderingModule->OpenGLVersionmMinor());
            if (RenderingModule->OpenGLUseCoreProfile())
            {
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
            }
            else
            {
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
            }
        }
        window = glfwCreateWindow(kBaseWidth, kBaseHeight, kBaseWindowName, nullptr, nullptr);
        AssertOrErrorCall(window, goto terminate_glfw_window, "Failed to create GLFW window")
        
        EngineLoggerLog("Initialized GLFW window");
            
        glfwSetKeyCallback(window, key_callback);
        glfwMakeContextCurrent(window);
        
        glfwSetWindowUserPointer(window, this);
        
        m_Window = window;
            
        return;
        
terminate_glfw_window:
        Shutdown();
        EngineRuntimeCrash("Failed to create GLFW window")
#endif // WINDOW_GLFW
    }

    void Module::Tick(double deltaTime)
    {
        // Reset states
        RequestShaderReload = false;
        
#ifdef WINDOW_GLFW
        GLFWwindow* window = (GLFWwindow*)m_Window;
        
        // End frame
        glfwSwapBuffers(window);
         
        // Start next frame
        {
            glfwPollEvents();

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
        
            if (width <= 0 || height <= 0)
            {
                m_ShouldResize = false;
                m_IsReduced = true;
                m_Width = 0;
                m_Height = 0;
            }
            else
            {
                m_LastMouseX = m_CurrentMouseX; m_LastMouseY = m_CurrentMouseY;
                glfwGetCursorPos(window, &m_CurrentMouseX, &m_CurrentMouseY);
                
                if (m_IsReduced)
                {
                    m_LastMouseX = m_CurrentMouseX; m_LastMouseY = m_CurrentMouseY;
                }
                
                m_ShouldResize = false;
                m_IsReduced = false;
            
                if (static_cast<uint32_t>(width) != m_Width || static_cast<uint32_t>(height) != m_Height)
                {
                    m_ShouldResize = true;
                    m_LastMouseX = m_CurrentMouseX; m_LastMouseY = m_CurrentMouseY;
                }
            
                m_Width = width;
                m_Height = height;
                
            }
        }
#endif // WINDOW_GLFW
    }

    void Module::Shutdown()
    {
#ifdef WINDOW_GLFW
        GLFWwindow* window = (GLFWwindow*)m_Window;
        
        glfwDestroyWindow(window);
        glfwTerminate();
        
        m_Window = nullptr;
#endif // WINDOW_GLFW
    }

    void Module::EditorUI()
    {
        if (ImGui::Button("Recompile Shaders")) RequestShaderReload = true;
    }

    bool Module::ShouldClose()
    {
#ifdef WINDOW_GLFW
        GLFWwindow* window = (GLFWwindow*)m_Window;
    
        return glfwWindowShouldClose(window);
#endif // WINDOW_GLFW
        
        UNIMPLEMENTED_FEATURE
    }

    bool Module::HasFocus()
    {
        // TODO using glfwSetWindowFocusCallback()
        return true;
    }
    
    bool Module::GetFrameBufferSize(uint32_t& width, uint32_t& height)
    {
        if (m_IsCurrentViewportSubViewport)
        {
            width = m_SubWidth;
            height = m_SubHeight;
            
            return m_ShouldResizeSubViewport;
        }
        
        width = m_Width;
        height = m_Height;
        
        return m_ShouldResize;
    }

    bool Module::ShouldRecompileShaders()
    {
        return RequestShaderReload;
    }

    void Module::GetMousePosition(double& x, double& y) const
    {
        x = m_CurrentMouseX;
        y = m_CurrentMouseY;
    }

    void Module::GetMousePositionDelta(double& x, double& y) const
    {
        x = m_CurrentMouseX - m_LastMouseX;
        y = m_CurrentMouseY - m_LastMouseY;
    }

#ifdef WINDOW_GLFW
    bool Module::GLFWGetKey(int code) const
    {
        GLFWwindow* window = (GLFWwindow*)m_Window;
        
        return glfwGetKey(window, code) == GLFW_PRESS;
    }

    bool Module::GLFWGetMouseButton(int code) const
    {
        GLFWwindow* window = (GLFWwindow*)m_Window;
        
        return glfwGetMouseButton(window, code) == GLFW_PRESS;
    }

    void Module::_EnableSubViewport(uint32_t Width, uint32_t Height)
    {
        m_SubWidth = Width;
        m_SubHeight = Height;
        
        // TODO verify 
        m_SubViewportWriteBuffer.emplace(Width, Height, Texture::Type::UnsignedByte, Texture::RGB);
        m_SubViewportFrameBuffer.emplace(FrameBuffer::Attachment(*m_SubViewportWriteBuffer, FrameBuffer::ClearColor(0.0f)));
        
        m_ShouldResizeSubViewport = true;
    }

    void Module::_SetViewportMainWindow()
    {
        m_IsCurrentViewportSubViewport = false;
        m_ShouldResizeSubViewport = false;
    }

    void Module::_SetViewportSubViewport(uint32_t Width, uint32_t Height)
    {
        AssertOrErrorCall(m_SubViewportFrameBuffer.has_value() && m_SubViewportWriteBuffer.has_value(), return;, "Tried to set sub viewport as the main rendering viewport but sub viewport have not been initialised.")
        
        if (m_SubWidth != Width || m_SubHeight != Height)
        {
            m_SubViewportWriteBuffer->Data(Width, Height);
            m_SubViewportFrameBuffer->Resize(Width, Height);
            m_SubWidth = Width;
            m_SubHeight = Height;
            m_ShouldResizeSubViewport = true;
        }
        
        m_IsCurrentViewportSubViewport = true;
    }

    void Module::_DisableSubViewport()
    {
        m_SubViewportFrameBuffer.reset();
        m_SubViewportWriteBuffer.reset();
        m_SubWidth = 0;
        m_SubHeight = 0;
        m_IsCurrentViewportSubViewport = false;
    }
#endif // WINDOW_GLFW
}

