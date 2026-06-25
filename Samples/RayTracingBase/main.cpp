#include <imgui.h>

#include "App.h"
#include "Core/Spec.h"

// Modules

#include "Camera/FlyCamera.h"
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"
#include "Modules/ImGui/Module.h"

#include "Files/Files.h"
#include "Importers/GLTF/SceneLoader.h"
#include "RayTracing/RayTrace.h"
#include "Rendering/Rendering.h"

#include <GLFW/glfw3.h>

using namespace Math;

float CameraSpeed = 1.0f;

void UpdateCamera(Window::Module& Window, double deltaTime, FlyCamera& camera)
{
    Vector3f PositionDir(0, 0, 0);
    float rotateDir = 0.0f;

    if (Window.GLFWGetKey(GLFW_KEY_LEFT_SHIFT))
        PositionDir.y += 1;
    if (Window.GLFWGetKey(GLFW_KEY_LEFT_CONTROL))
        PositionDir.y -= 1;
    if (Window.GLFWGetKey(GLFW_KEY_W))
        PositionDir.x += 1;
    if (Window.GLFWGetKey(GLFW_KEY_S))
        PositionDir.x -= 1;
    if (Window.GLFWGetKey(GLFW_KEY_A))
        PositionDir.z += 1;
    if (Window.GLFWGetKey(GLFW_KEY_D))
        PositionDir.z -= 1;
    
    PositionDir = PositionDir * static_cast<float>(deltaTime) * (CameraSpeed * 100) * 4.0f;
    camera.Translate(Transpose(camera.GetWorldRotation().GetRotationMatrix()) * PositionDir);
        
    if (Window.GLFWGetKey(GLFW_KEY_Q))
        rotateDir += 1;
    if (Window.GLFWGetKey(GLFW_KEY_E))
        rotateDir -= 1;
    
    camera.RotateRadians(0, rotateDir * Pi * deltaTime * (CameraSpeed * 1000.f) / 2.5);
}

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
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        
        uint32_t Width, Height;
        IGNORE_RETURN Window->GetFrameBufferSize(Width, Height);
        
        m_FOV = 45.0f;
        m_ZNear = 0.15f;
        m_ZFar = 10.0f;
        m_LightColor = {1.0f, 0.97f, 0.9f};
        m_LightDirection = Normalize(Math::Vector3f(.1, .9, .2));
        m_LightIntensity = 3.0f;
        m_AmbientColor = {0.7f, 0.78f, 1.0f};
        m_AmbientIntensity = 1.2f;
        m_Camera.SetProjection(Width, Height, Radians(m_FOV), m_ZNear, m_ZFar);
        m_Camera.SetTranslation(0,1,4);
        m_Camera.SetRotationDegrees(0,-90);
        
        
        // Load scene data
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Meshes") / "CornellBox-Original.glb" ,path))
            {
                m_Scene.emplace();
                AssertOrError( GLTF::LoadCPUScene(path, *m_Scene), "Failed to load scene")
                m_MeshObject.emplace(m_Scene->meshes[0]);
            }
        }
        
        // Load HDRi
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Textures") / "HDRi" / "san_giuseppe_bridge_4k.hdr" ,path))
            {
                m_SkyboxHDRiCpu = ImageLoad(path, Image::Float);
            }
        }
        
        m_SamplePipeline.emplace(PipelineFromFile("Draw example triangle", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToFrame.glsl"));
        
        m_WriteImage.emplace(Width, Height, Image::UnsignedByte, Image::RGB, nullptr);
    }

    void Tick(double deltaTime) override
    {
        DebugScopeMarker scope("Draw Raster");
        
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
            m_WriteImage.reset();
            m_WriteImage.emplace(Width, Height, Image::UnsignedByte, Image::RGB, nullptr);
        }
        
        UpdateCamera(*Window, deltaTime, m_Camera);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Bind(*m_SamplePipeline);
        
        Bind(m_MeshObject->GetVAO());
        
        SetUniform(*m_SamplePipeline, "ViewProjection", m_Camera.Projection() * m_Camera.View());
        
        SetUniform(*m_SamplePipeline, "lightColor", m_LightColor * m_LightIntensity);
        SetUniform(*m_SamplePipeline, "lightDirection", m_LightDirection);
        SetUniform(*m_SamplePipeline, "ambientColor",m_AmbientColor * m_AmbientIntensity);
        
        for (const auto & instance : m_Scene->instances)
        {
            AssertOrError(instance.mesh == 0, "Index out of range")
            const GLTF::Material& material = m_Scene->materials[instance.material];
            
            const Mesh::VertexGroup& Group = m_MeshObject->GetGroups()[instance.vertexGroup];
            const GLTF::Transform& Transform = m_Scene->transforms[instance.transform];
            const GLTF::Material& Material = m_Scene->materials[instance.material];
            
            // Transform
            switch (Transform.Type)
            {
            case GLTF::Transform::Properties:
                {
                    Math::Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
            
                    // if (UseFrustumCulling && !Rendering::frustumCullingTest(Resources.GetMainCameraData().Camera_WorldToProj(), TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
            
                    SetUniform(*m_SamplePipeline, "Model", TransformMatrix);
                }
                break;
                
            case GLTF::Transform::Matrix:
                {
                    // if (UseFrustumCulling && !Rendering::frustumCullingTest(Resources.GetMainCameraData().Camera_WorldToProj(), Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
            
                    SetUniform(*m_SamplePipeline, "Model", Transform.Value.asMatrix);
                }
                break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
            
            // Material
            SetUniform(*m_SamplePipeline, "baseColor", Material.color.XYZ());
            SetUniform(*m_SamplePipeline, "emissive", Material.emissive.XYZ());
            SetUniform(*m_SamplePipeline, "roughness", Material.roughness);
            SetUniform(*m_SamplePipeline, "metallic", Material.metallic);
            
            
            if (m_MeshObject->GetIndexBuffer().has_value())
            {
                const IndexBuffer& indexBuffer = m_MeshObject->GetIndexBuffer().value();
                Bind(indexBuffer);
            
                glDrawElements(ToGLGeometryType(m_MeshObject->GetVertexType()), Group.VertexCount, ToGLIndexType(indexBuffer.GetIndexType()), (void*)(Group.FirstVertex * ToGLIndexSize(indexBuffer.GetIndexType())));
            
                UnBind(indexBuffer);
            }
            else
            {
                glDrawArrays(ToGLGeometryType(m_MeshObject->GetVertexType()), Group.FirstVertex, Group.VertexCount);
            }
            
        }
        UnBind(m_MeshObject->GetVAO());
        
        UnBind(*m_SamplePipeline);
    }

    void Shutdown() override
    {
        m_SamplePipeline.reset();
        m_Scene.reset();
        m_SkyboxHDRiCpu.reset();
        m_WriteImage.reset();
        m_SkyboxHDRiGpu.reset();
        m_MeshObject.reset();
    }

    void EditorUI() override
    {
    }
    
    void RayTracedScreenshot()
    {
        const uint32_t Width = m_WriteImage->Width(); 
        const uint32_t Height = m_WriteImage->Height();
        const Matrix4f ProjToWorld = m_Camera.InverseView() * m_Camera.InverseProjection();
        
        for (uint32_t y = 0; y < Height; y++)
        for (uint32_t x = 0; x < Width; x++)
        {
            Vector4f Start = ProjToWorld * Vector4f(((static_cast<float>(x) + .5f) / static_cast<float>(Width)), ((static_cast<float>(y) + .5f) / static_cast<float>(Height)), 0, 1);
            Start.xyz() /= Start.w;
            Vector4f End = ProjToWorld * Vector4f(((static_cast<float>(x) + + .5f) / static_cast<float>(Width)), ((static_cast<float>(y) + .5f) / static_cast<float>(Height)), 1, 1);
            End.xyz() /= End.w;
            
            Ray PrimaryRay = {
                .origin = Start.xyz(), 
                .direction = Normalize(End.xyz() - Start.xyz()), 
                .distance = Magnitude(End.xyz() - Start.xyz())
            };
            TraceRay PrimaryRayTracer(m_Scene->meshes[0], PrimaryRay);
        
            for (Hit hit : PrimaryRayTracer)
            {
                // Any hit
            }
        
            if (Hit Closest = PrimaryRayTracer.ClosestHit(); Closest)
            {
                // Closest Hit
            }
            else
            {
                // Miss
            }
        }
    }
    
private:
    float m_FOV;
    float m_ZNear;
    float m_ZFar;
    Math::Vector3f m_LightColor;
    Math::Vector3f m_LightDirection;
    float m_LightIntensity;
    Math::Vector3f m_AmbientColor;
    float m_AmbientIntensity;
    
    std::optional<GLTF::CPUScene> m_Scene;
    std::optional<Image> m_SkyboxHDRiCpu;
    std::optional<Image> m_WriteImage;
    
    std::optional<Pipeline> m_SamplePipeline;
    std::optional<Texture2D> m_SkyboxHDRiGpu;
    std::optional<MeshObject> m_MeshObject;
    
    FlyCamera m_Camera;
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
