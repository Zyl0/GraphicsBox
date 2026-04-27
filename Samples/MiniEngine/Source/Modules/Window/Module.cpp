#include "Modules/Rendering/Module.h"
#include "Modules/Window/Module.h"

#include "Rendering/GLHelper.h"
#ifdef WINDOW_GLFW
#include <GLFW/glfw3.h>
#endif // WINDOW_GLFW

#include "Shared/Assertion.h"

#include "Core/Engine.h"
#include "Modules/Rendering/Module.h"

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
                m_ShouldResize = false;
                m_IsReduced = false;
            
                if (static_cast<uint32_t>(width) != m_Width || static_cast<uint32_t>(height) != m_Height)
                {
                    m_ShouldResize = true;
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

    bool Module::ShouldClose()
    {
#ifdef WINDOW_GLFW
        GLFWwindow* window = (GLFWwindow*)m_Window;
    
        return glfwWindowShouldClose(window);
#endif // WINDOW_GLFW
    }

    bool Module::HasFocus()
    {
        // TODO using glfwSetWindowFocusCallback()
        return true;
    }
    
    bool Module::GetFrameBufferSize(uint32_t& width, uint32_t& height)
    {
        width = m_Width;
        height = m_Height;
        
        return m_ShouldResize;
    }

    bool Module::ShouldRecompileShaders()
    {
        return RequestShaderReload;
    }

#ifdef WINDOW_GLFW
    bool Module::GLFWGetKey(int code)
    {
        GLFWwindow* window = (GLFWwindow*)m_Window;
        
        return glfwGetKey(window, code);
    }
#endif // WINDOW_GLFW
}

