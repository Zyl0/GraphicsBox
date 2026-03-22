#include "Shared/Assertion.h"
#include "Math/RMath.h"
#include "Files/Files.h"
#include "Modeling/Mesh.h"
#include "Rendering/Rendering.h"

#include <imgui.h>

// Implementation using the mini engine
// #define USE_MINI_ENGINE

#ifdef USE_MINI_ENGINE
#else // USE_MINI_ENGINE

#include "backends/imgui_impl_opengl3.h"

#ifdef WINDOW_GLFW
#include <GLFW/glfw3.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_glfw.h"
#endif // WINDOW_GLFW


using namespace Math;

/* ____________________________________ Debug ____________________________________ */

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
        EngineRuntimeBREAKPOINT;
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

/* ____________________________________ Process ____________________________________ */

int main(void)
{
    int RC = EXIT_SUCCESS; // Ok
    
#ifdef WINDOW_GLFW
    glfwSetErrorCallback(error_callback);
    AssertOrErrorCall(glfwInit(), RC = EXIT_FAILURE; goto terminate_main, "Failed to initialise GLFW")
    

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    GLFWwindow* window = glfwCreateWindow(kBaseWidth, kBaseHeight, "ColorBox", monitor, share);
    AssertOrErrorCall(window, RC = EXIT_FAILURE; goto terminate_glfw_window, "Failed to create GLFW window")
    
    glfwSetKeyCallback(CurrentWindow, key_callback);
#endif // WINDOW_GLFW

    AssertOrErrorCall(glewInit() == GLEW_OK, RC = EXIT_FAILURE; goto terminate_context, "Failed to initialize GLEW")

#ifdef CONFIG_DEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Makes sure errors are displayed synchronously
    glDebugMessageCallback(MessageCallback, 0);
#endif // CONFIG_DEBUG

    ImGui::CreateContext();
#ifdef WINDOW_GLFW
    AssertOrErrorCall(ImGui_ImplGlfw_InitForOpenGL(CurrentWindow, true), RC = EXIT_FAILURE; goto terminate_glfw_ui, "Could not initialize ImGUI")
#endif // WINDOW_GLFW
    AssertOrErrorCall(ImGui_ImplOpenGL3_Init("#version 430"), RC = EXIT_FAILURE; goto terminate_ui, "Could not initialize ImGUI")
    
    // App content
    {
        
        
    }
    
terminate_ui:
    ImGui_ImplOpenGL3_Shutdown();
#ifdef WINDOW_GLFW
terminate_glfw_ui:
    ImGui_ImplGlfw_Shutdown();
#endif // WINDOW_GLFW
    
    ImGui::DestroyContext();

terminate_context:
#ifdef WINDOW_GLFW
    glfwDestroyWindow(window);
    
terminate_glfw_window:
    glfwTerminate();
#endif // WINDOW_GLFW
    
terminate_main:
    return RC;
}

#endif // !USE_MINI_ENGINE