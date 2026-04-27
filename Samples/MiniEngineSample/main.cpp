#include <imgui.h>

#include "App.h"
#include "Core/Spec.h"

// Modules
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"
#include "Modules/ImGui/Module.h"

#include "Files/Files.h"
#include "Rendering/Rendering.h"

using namespace Math;

class AppModule : public Engine::IModule
{
public:
    AppModule() {}

    ~AppModule() override = default;
    
    void RegisterDependencies(Engine::Spec& spec) override
    {
        spec.Register<Window::Module>();
        spec.Register<Rendering::Module>();
        spec.Register<ImGui::Module>();
    }

    void Initialize() override
    {
        m_SamplePipeline.emplace(PipelineFromFile("Draw example triangle", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SampleShader.glsl"));
    }

    void Tick(double deltaTime) override
    {
        DebugScopeMarker scope("Example drawcall");
        
        Bind(*m_SamplePipeline);
        
        SetUniform(*m_SamplePipeline, "ColorA", ColorA);
        SetUniform(*m_SamplePipeline, "ColorB", ColorB);
        SetUniform(*m_SamplePipeline, "ColorC", ColorC);
        
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        UnBind(*m_SamplePipeline);
    }

    void Shutdown() override
    {
        m_SamplePipeline.reset();
    }

    void EditorUI() override
    {
        ImGui::ColorEdit3("Color A", ColorA.data());
        ImGui::ColorEdit3("Color B", ColorB.data());
        ImGui::ColorEdit3("Color C", ColorC.data());
    }
    
private:
    std::optional<Pipeline> m_SamplePipeline;

    Vector3f ColorA = {1, 0, 0}, ColorB = {0, 1, 0}, ColorC = {0, 0, 1};
};


int main(int argc, char* argv[])
{
    // Search paths
    AddSearchPath(RESOURCES_GLOBAL);
    AddSearchPath(RESOURCES_PROJECT);
    ShaderAddSearchPath(SHADERS_GLOBAL);
    ShaderAddSearchPath(SHADERS_PROJECT);
    
    Engine::Spec Specification;
    Specification.Register<Window::Module>();
    Specification.Register<Rendering::Module>();
    Specification.Register<ImGui::Module>();
    Specification.Register<AppModule>();
    
    Engine::App App(std::move(Specification));
    
    App.Run();
}
