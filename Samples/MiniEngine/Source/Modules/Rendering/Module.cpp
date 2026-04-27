#include "Modules/Rendering/Module.h"

#include "Rendering/GLHelper.h"

// Dependency module
#include "Modules/Window/Module.h"

// Added components
#include "Modules/Rendering/World/Camera.h"
#include "Modules/Rendering/World/Mesh.h"

#include "Core/Spec.h"
#include "World/Component.h"

namespace Rendering
{
    void GLAPIENTRY MessageCallback(GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam)
    {
        // Ignore non-significant error/warning codes
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        const char* NullString = "";
        const char* errSource = NullString;
        const char* errType = NullString;

        switch (source)
        {
        case GL_DEBUG_SOURCE_API:             errSource = "Source: API";                 break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   errSource = "Source: Window System";       break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: errSource = "Source: Shader Compiler";     break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     errSource = "Source: Third Party";         break;
        case GL_DEBUG_SOURCE_APPLICATION:     errSource = "Source: Application";         break;
        case GL_DEBUG_SOURCE_OTHER:           errSource = "Source: Other";               break;
        }

        switch (type)
        {
        case GL_DEBUG_TYPE_ERROR:               errType = "Type: Error";                 break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: errType = "Type: Deprecated Behaviour";  break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  errType = "Type: Undefined Behaviour";   break;
        case GL_DEBUG_TYPE_PORTABILITY:         errType = "Type: Portability";           break;
        case GL_DEBUG_TYPE_PERFORMANCE:         errType = "Type: Performance";           break;
        case GL_DEBUG_TYPE_MARKER:              errType = "Type: Marker";                break;
        case GL_DEBUG_TYPE_OTHER:               errType = "Type: Other";                 break;

        case GL_DEBUG_TYPE_PUSH_GROUP:
        case GL_DEBUG_TYPE_POP_GROUP:
            return;
        }

        switch (severity)
        {
        case GL_DEBUG_SEVERITY_HIGH:
            EngineLoggerErrorF("OpenGL Validation Error High [%s] [%s]: %s", errSource, errType, message);
    #ifdef CONFIG_DEBUG
            EngineRuntimeBREAKPOINT
    #endif // CONFIG_DEBUG
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            EngineLoggerErrorF("OpenGL Validation Error Medium [%s] [%s]: %s", errSource, errType, message);
            break;

        case GL_DEBUG_SEVERITY_LOW:
            EngineLoggerErrorF("OpenGL Validation Error Low [%s] [%s]: %s", errSource, errType, message);
            break;

        case GL_DEBUG_SEVERITY_NOTIFICATION:
            EngineLoggerErrorF("OpenGL Validation Notification [%s] [%s]: %s", errSource, errType, message);
            break;
        }
    }
    
    void Module::RegisterDependencies(Engine::Spec& spec)
    {
        spec.Register<Window::Module>();
    }

    void Module::RegisterComponents()
    {
        // Engine::World::RegisterComponentSystem<World::CameraComponentSystem>(Context());
        // Engine::World::RegisterComponentSystem<World::MeshComponentSystem>(Context());
        // Engine::World::RegisterComponentSystem<World::CameraComponentSystem>(Context());
    }

    void Module::EnableMSAA()
    {
        glEnable(GL_MULTISAMPLE);
    }

    void Module::Initialize()
    {
        AssertOrError(glewInit() == GLEW_OK, "Failed to initialize GLEW")

        EngineLoggerLog("Initialized GLEW");
        EngineLoggerLogF("OpenGL Version: %s", glGetString(GL_VERSION));
        EngineLoggerLogF("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
        EngineLoggerLogF("Vendor: %s", glGetString(GL_VENDOR));
        EngineLoggerLogF("Renderer: %s", glGetString(GL_RENDERER));

#ifdef CONFIG_DEBUG
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Makes sure errors are displayed synchronously
        glDebugMessageCallback(MessageCallback, 0);
#endif // CONFIG_DEBUG

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CW);
    
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glClearDepth(1.0f);
    }

    void Module::Tick(double deltaTime)
    {
        Window::Module* Window = GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        uint32_t Width, Height;
        if (Window->GetFrameBufferSize(Width, Height))
        {
            glViewport(0, 0, Width, Height);
        }
            
        glClear(GL_COLOR_BUFFER_BIT);
    }
}

