#include <imgui.h>

#include "App.h"
#include "Core/Spec.h"

// Modules
#include "Camera/FlyCamera.h"
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"
#include "Modules/ImGui/Module.h"

#include "Files/Files.h"
#include "Math/Box.h"
#include "Modeling/HierarchicalDistanceFields.h"
#include "Rendering/Rendering.h"

using namespace Math;

class AppModule : public Engine::IModule
{
public:
    AppModule() = default;
    ~AppModule() override = default;

    void RegisterDependencies(Engine::Spec& spec) override
    {
        spec.Register<Window::Module>();
        spec.Register<Rendering::Module>();
        spec.Register<ImGui::Module>();
    }

    void Initialize() override
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        uint32_t Width, Height;
        IGNORE_RETURN Window->GetFrameBufferSize(Width, Height);

        // Reset states
        m_FOV = 45.0f;
        m_ZNear = 0.15f;
        m_ZFar = 10.0f;
        m_Camera.SetProjection(Width, Height, Radians(m_FOV), m_ZNear, m_ZFar);
        m_Camera.SetTranslation(0, 0, 0);
        m_Camera.SetRotationDegrees(0,0);
        m_SDFSamplingVolume = Math::Box3d(1.0f);
        m_SDFSamplingResolution = 50;
        m_LightColor = {1.0f, 0.97f, 0.9f};
        m_LightDirection = Normalize(Math::Vector3f(.1, .9, .2));
        m_LightIntensity = 3.0f;
        m_AmbientColor = {0.7f, 0.78f, 1.0f};
        m_AmbientIntensity = 1.2;
        m_UseGrid = 1;
        m_GridLineWidth = 0.05;
        m_GridSize = 0.2;

        // Initialize rendering resources
        m_SamplePipeline.emplace(PipelineFromFile("Draw mesh", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToFrame.glsl"));
        m_MeshObject.emplace();

        DefaultDistanceFieldTree();
        UpdateMesh();
        m_DistanceFieldTree.SetTraversalType(HDFTree::TraversalType::Tree);
    }

    void Tick(double deltaTime) override
    {
        DebugScopeMarker scope("Example drawcall");

        Window::Module* Window = Engine::GetModule<Window::Module>(Context());

        // Handle shader reload
        if (Window->ShouldRecompileShaders())
        {
            PipelineUpdateFromFile(*m_SamplePipeline, "MeshToFrame.glsl");
        }

        // Handle window resize
        uint32_t Width, Height;
        if (Window->GetFrameBufferSize(Width, Height))
        {
            m_Camera.SetProjection(Width, Height, Radians(m_FOV), m_ZNear, m_ZFar);
        }

        Bind(*m_SamplePipeline);
        Bind(m_MeshObject->GetVAO());

        SetUniform(*m_SamplePipeline, "Model", Math::MakeHomogeneousIdentity<float>());
        SetUniform(*m_SamplePipeline, "ViewProjection", m_Camera.Projection() * m_Camera.View());

        SetUniform(*m_SamplePipeline, "lightColor", m_LightColor * m_LightIntensity);
        SetUniform(*m_SamplePipeline, "lightDirection", m_LightDirection);
        SetUniform(*m_SamplePipeline, "ambientColor",m_AmbientColor * m_AmbientIntensity);

        SetUniform(*m_SamplePipeline, "useGrid", m_UseGrid);
        SetUniform(*m_SamplePipeline, "gridSize", m_GridSize);
        SetUniform(*m_SamplePipeline, "gridLineWidth", m_GridLineWidth);

        // Draw screen quad
        const Mesh::VertexGroup& Group = m_MeshObject->GetGroups()[0]; // Sampled distance field should be only made of one group
        glDrawArrays(GL_TRIANGLES, Group.FirstVertex, Group.VertexCount);

        UnBind(m_MeshObject->GetVAO());
        UnBind(*m_SamplePipeline);
    }

    void Shutdown() override
    {
        m_SamplePipeline.reset();
        m_MeshObject.reset();
    }

    void EditorUI() override
    {

    }

    void UpdateMesh()
    {
        // Polygonize distance field
        Polygonise(m_DistanceFieldTree, m_SDFSamplingVolume, m_SDFSamplingResolution, m_Mesh);

        // Upload mesh
        m_MeshObject->Data(m_Mesh);
    }

    void DefaultDistanceFieldTree()
    {
        m_DistanceFieldTree.Clear();
        m_DistanceFieldTree.ShrinkToFit();

        m_DistanceFieldTree.SetHead(m_DistanceFieldTree.AddSphere());
    }

private:
    // States
    float m_FOV;
    float m_ZNear;
    float m_ZFar;
    Math::Box3d m_SDFSamplingVolume;
    size_t m_SDFSamplingResolution;
    Math::Vector3f m_LightColor;
    Math::Vector3f m_LightDirection;
    float m_LightIntensity;
    Math::Vector3f m_AmbientColor;
    float m_AmbientIntensity;
    uint32_t m_UseGrid;
    float m_GridLineWidth;
    float m_GridSize;

    // Rendering data (CPU)
    Mesh m_Mesh;
    HDFTree m_DistanceFieldTree;
    FlyCamera m_Camera;

    // Rendering objects
    std::optional<Pipeline> m_SamplePipeline;
    std::optional<MeshObject> m_MeshObject;
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
