#include "Modules/ImGui/Module.h"

#include "Core/Spec.h"

// Dependency module
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"

#include "Shared/Assertion.h"
#include "Rendering/GLHelper.h"

#include <imgui.h>

#include <backends/imgui_impl_opengl3.h>

#ifdef WINDOW_GLFW
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#endif // WINDOW_GLFW

namespace ImGui
{
    void Module::RegisterDependencies(Engine::Spec& spec)
    {
        spec.Register<Window::Module>();
        spec.Register<Rendering::Module>();
    }

    void Module::Initialize()
    {
        Window::Module* Window = GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        Rendering::Module* Rendering = GetModule<Rendering::Module>(Context());
        AssertOrError(Rendering != nullptr, "Rendering is null")

        ImGui::CreateContext();
        
#ifdef WINDOW_GLFW
        AssertOrError(ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)(Window->_Handle()), true), "Could not initialize ImGUI from Window")
#endif // WINDOW_GLFW

        AssertOrError(Rendering->OpenGLVersionMajor() > 0 && Rendering->OpenGLVersionmMinor() >= 0, "ImGUI requires a valid OpenGL version")

        std::string glsl_version = "#version ";
        glsl_version.append(std::to_string(Rendering->OpenGLVersionMajor()));
        glsl_version.append(std::to_string(Rendering->OpenGLVersionmMinor()));
        glsl_version.append("0");
        
        AssertOrError(ImGui_ImplOpenGL3_Init(glsl_version.data()), "Could not initialize ImGUI from OpenGL")
    }

    void Module::Shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        
#ifdef WINDOW_GLFW
            ImGui_ImplGlfw_Shutdown();
#endif // WINDOW_GLFW
    
        ImGui::DestroyContext();
    }

    void Module::_BeginFrame()
    {
#ifdef WINDOW_GLFW
        ImGui_ImplGlfw_NewFrame();
#endif // WINDOW_GLFW

        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
    }

    void Module::_EndFrame()
    {
        ImGui::Render();
        ImGui::EndFrame();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
}
