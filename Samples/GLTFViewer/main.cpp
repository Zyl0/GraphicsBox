#include "Shared/Assertion.h"
#include "Math/RMath.h"
#include "Files/Files.h"
#include "Modeling/Mesh.h"
#include "Rendering/Rendering.h"
#include "Importers/GLTF/SceneLoader.h"
#include "Image/ColorSpaces.h"
#include "Camera/FlyCamera.h"

#include <imgui.h>

#include "Modules/Rendering/Shaders/Camera.h"


#include "App.h"
#include "Core/Engine.h"

// Modules
#include "Modules/ImGui/Module.h"
#include "Modules/Rendering/Module.h"
#include "Modules/Rendering/Tools/FrustumCulling.h"
#include "Modules/Window/Module.h"

// for macro keys, TODO maybe abstract into an input system or module
#include <GLFW/glfw3.h>

#include "Modules/FrameGraph/Commands.h"

using namespace Math;

#define USE_FORWARD_RENDERING_PIPELINE

#include "Modules/FrameGraph/Nodes/NativeResolutionRadiance.h"
#include "Modules/FrameGraph/Nodes/SkylightToRadiance.h"
#ifdef USE_FORWARD_RENDERING_PIPELINE
#include "Modules/FrameGraph/Nodes/MeshToSceneRadiance.h"
#endif // USE_FORWARD_RENDERING_PIPELINE
#ifdef USE_DIFFERED_RENDERING_PIPELINE
#include "Modules/FrameGraph/Nodes/MeshToGBuffer.h"
#include "Modules/FrameGraph/Nodes/GBufferDirectionalLightRadiance.h"
#include "Modules/FrameGraph/Nodes/GBufferIndirectLightRadiance.h"
#endif // USE_DIFFERED_RENDERING_PIPELINE
#include "Modules/FrameGraph/Nodes/PostProcess.h"

/* ____________________________________ Constants ____________________________________ */

constexpr float kZNear = 0.01f;
constexpr float kZFar = 1000.0f;
constexpr uint32_t kBaseAntiAliasingMethod = 1; // MSAA
constexpr uint8_t kBaseSampleCount = 4;

/* ____________________________________ States ____________________________________ */

float CameraSpeed = 1.0f;

/* ____________________________________ Render Data ____________________________________ */

/* ____________________________________ Baking Passes ____________________________________ */

/* ____________________________________ Process ____________________________________ */

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
    }

    void Initialize() override
    {
        glGetIntegerv ( GL_MAX_SAMPLES, &MaxSupportedMSAASamples );

        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        Rendering::Module* Rendering = Engine::GetModule<Rendering::Module>(Context());
        AssertOrError(Rendering != nullptr, "Rendering is null")

        Rendering->EnableMSAA();

        uint32_t InitialWidth, InitialHeight;
        Window->GetFrameBufferSize(InitialWidth, InitialHeight);
        
        m_CommandList.emplace();
        TexOutput = m_CommandList->Context().Add<Texture2D>("Output", InitialWidth, InitialHeight,  Texture::UnsignedByte, Texture::RGB); // TODO introduce a way to have outputs to the graph
        VOutputSize = m_CommandList->Context().AddVariable<FrameGraph::Size2D>("Output", FrameGraph::Size2D{InitialWidth, InitialHeight});
        TexCubemap = m_CommandList->Context().Add<TextureCube>("Cubemap Skylight", 0u, 0u, Texture::Byte, Texture::R);
        TexHDRi = m_CommandList->Context().Add<Texture2D>("HDRi Skylight", 0u, 0u, Texture::Byte, Texture::R);
        
        // TODO introduce a proper light system
        VMainLightDirection = m_CommandList->Context().AddVariable<Vector3f>("Light Direction", Normalize(Vector3f{0.8f, -1.0f, 0.9f}));
        VMainLightColor = m_CommandList->Context().AddVariable<Vector3f>("Light Color", {1.0f, 1.0f, 1.0f});
        VMainLightIntensity = m_CommandList->Context().AddVariable<FrameGraph::Float>("Light Intensity", 1.0f);
        
        m_ViewportCamera.SetProjection(InitialWidth, InitialHeight, Radians(45.0f), kZNear, kZFar);
        VMainCamera = m_CommandList->Context().AddCamera();

        // Load scene data
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "ABeautifulGame" / "glTF-Binary" /"ABeautifulGame.glb" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "MetalRoughSpheres" / "glTF-Binary" /"MetalRoughSpheres.glb" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "MetalRoughSpheres" / "glTF" /"MetalRoughSpheres.gltf" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("Willy") / "Splash" /"splash.gltf" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("Willy") / "BistroGLTF" /"exterior.glb" ,path))
            {
                AssertOrError( GLTF::LoadGPUScene(path, m_CommandList->Context().Scene()), "Failed to load scene")
            }
        }

        // Load cubemap
        {
            const std::filesystem::path folder = std::filesystem::path("Textures") / "CubeMaps" / "LearnOpenGL";
            Image Front = ImageLoad(GetAbsoluteFilePath(folder / "front.jpg"), Image::UnsignedByte);
            Image Back = ImageLoad(GetAbsoluteFilePath(folder / "back.jpg"), Image::UnsignedByte);
            Image Left = ImageLoad(GetAbsoluteFilePath(folder / "left.jpg"), Image::UnsignedByte);
            Image Right = ImageLoad(GetAbsoluteFilePath(folder / "right.jpg"), Image::UnsignedByte);
            Image Top = ImageLoad(GetAbsoluteFilePath(folder / "top.jpg"), Image::UnsignedByte);
            Image Bottom = ImageLoad(GetAbsoluteFilePath(folder / "bottom.jpg"), Image::UnsignedByte);
            
            std::array faces{
                TextureCube::FacePair(TextureCube::Front, Front),
                TextureCube::FacePair(TextureCube::Back, Back),
                TextureCube::FacePair(TextureCube::Left, Left),
                TextureCube::FacePair(TextureCube::Right, Right),
                TextureCube::FacePair(TextureCube::Up, Top),
                TextureCube::FacePair(TextureCube::Down, Bottom),
            };

            m_CommandList->Context().Get<TextureCube>(TexCubemap).Data(faces);
        }
        
        // Load HDRi
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Textures") / "HDRi" / "san_giuseppe_bridge_4k.hdr" ,path))
            {
                m_CommandList->Context().Get<Texture2D>(TexHDRi).Data(ImageLoad(path, Image::Float));
            }
        }
        
        // Define rendering pipeline
        m_CommandList->PushNode<FrameGraph::NativeResolutionRadiance>();
        m_CommandList->PushNode<FrameGraph::SkylightToRadiance>();
#ifdef USE_FORWARD_RENDERING_PIPELINE
        m_CommandList->PushNode<FrameGraph::MeshToSceneRadiance>();
#endif // USE_FORWARD_RENDERING_PIPELINE
#ifdef USE_DIFFERED_RENDERING_PIPELINE
        m_CommandList->PushNode<FrameGraph::MeshToGBuffer>();
        m_CommandList->PushNode<FrameGraph::GBufferDirectionalLightRadiance>();
        m_CommandList->PushNode<FrameGraph::GBufferIndirectLightRadiance>();
#endif // USE_DIFFERED_RENDERING_PIPELINE
        m_CommandList->PushNode<FrameGraph::ToneMappingCommand>();
        
        // Graph exposed variables
        VSkyLightMethod = m_CommandList->Context().GetLocation<FrameGraph::UInt>("Skylight Method");
        VUseFrustumCulling = m_CommandList->Context().GetLocation<FrameGraph::Bool>("UseFrustumCulling");
        VIndirectLightSampleCount = m_CommandList->Context().GetLocation<FrameGraph::UInt>("Indirect Sample Count");
        
        m_OutputFrameBuffer.emplace(FrameBuffer::Attachment(m_CommandList->Context().Get<Texture2D>(TexOutput), FrameBuffer::ClearColor(0.0f)));
    }

    void Tick(double deltaTime) override
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        // Handle Window resize
        uint32_t NextWidth, NextHeight;
        if (Window->GetFrameBufferSize(NextWidth, NextHeight))
        {
            m_ViewportCamera.SetProjection(NextWidth, NextHeight, Math::Radians(45.0f), kZNear, kZFar);
            
            m_CommandList->Context().Get<Texture2D>(TexOutput).Data(NextWidth, NextHeight);
            m_CommandList->Context().SetValue<FrameGraph::Size2D>(VOutputSize, FrameGraph::Size2D{NextWidth, NextHeight});
        }

        // Handle Shader Reload
        if (Window->ShouldRecompileShaders())
        {
            m_CommandList->ReloadShaders();
        }

        // Update scene
        {                
            UpdateCamera(*Window, deltaTime, m_ViewportCamera);
            m_CommandList->Context().UpdateCamera(VMainCamera, m_ViewportCamera);
            
            m_CommandList->Update(deltaTime);
        }

        // Draw scene
        {
            m_CommandList->Render();
        }
            
        // Move results to viewport
        // TODO cleanup and integrate to the engine
        {
            glViewport(0, 0, NextWidth, NextHeight);
            glClear(GL_COLOR_BUFFER_BIT);

            glBlitNamedFramebuffer(m_OutputFrameBuffer->Handle(), /*Main Frame buffer ??*/ 0, 
                0, 0, NextWidth, NextHeight, 
                0, 0, NextWidth, NextHeight, 
                GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
    }

    void Shutdown() override
    {
        m_OutputFrameBuffer.reset();
        m_CommandList.reset();
    }

    void EditorUI() override
    {
        // Directional Light
        // TODO do ImGUI wrappers of graph variables
        // TODO automate exposition of graph variables on demand
        {
            Vector3f Copy = m_CommandList->Context().GetValue<Vector3f>(VMainLightDirection);
            if (ImGui::DragFloat3("Light Direction", Copy.data(), 0.1f))
            {
                m_CommandList->Context().SetValue<Vector3f>(VMainLightDirection, Copy);
            }
        }
        {
            Vector3f Copy = m_CommandList->Context().GetValue<Vector3f>(VMainLightColor);
            if (ImGui::ColorEdit3("Light Color", Copy.data()))
            {
                m_CommandList->Context().SetValue<Vector3f>(VMainLightColor, Copy);
            }
        }
        {
            float Copy = m_CommandList->Context().GetValue<FrameGraph::Float>(VMainLightIntensity);
            if (ImGui::DragFloat("Light Intensity", &Copy, 0.1f))
            {
                m_CommandList->Context().SetValue<FrameGraph::Float>(VMainLightIntensity, Copy);
            }
        }

        ImGui::Separator();
        
        {
            static const char* SkyLightMethodNames[] =
            {
                "Cubemap", "HDRi"
            };
            
            int Copy = m_CommandList->Context().GetValue<FrameGraph::UInt>(VSkyLightMethod);
            if (ImGui::ListBox("Sky Light Method", &Copy, SkyLightMethodNames, 2))
            {
                Copy = Math::Clamp(Copy, 0, 1);
                m_CommandList->Context().SetValue<FrameGraph::UInt>(VSkyLightMethod, (FrameGraph::UInt)(Copy));
            }
        }
        {
            int Copy = m_CommandList->Context().GetValue<FrameGraph::UInt>(VIndirectLightSampleCount);
            if (ImGui::SliderInt("Indirect Light Sample Count", &Copy, 1, 1024))
            {
                Copy = Math::Clamp(Copy, 1, 1024);
                m_CommandList->Context().SetValue<FrameGraph::UInt>(VIndirectLightSampleCount, (FrameGraph::UInt)Copy);
            }
        }

        ImGui::Separator();

        ImGui::SliderFloat("Camera Speed", &CameraSpeed, 0.1f, 2.0f);
        {
            bool Copy = m_CommandList->Context().GetValue<FrameGraph::Bool>(VUseFrustumCulling);
            if (ImGui::Checkbox("Use Frustum Culling", &Copy))
            {
                m_CommandList->Context().SetValue<FrameGraph::Bool>(VUseFrustumCulling, Copy);
            }
        }

        // TODO make a profiling window
        // ImGui::Text("Frame time (CPU): %f ms", frameTimeCPU);
        // ImGui::Text("Frame time (GPU): %f ms", frameTimeGPU);
    }
    
private:
    GLint MaxSupportedMSAASamples;
    
    std::optional<FrameGraph::CommandList> m_CommandList;
    std::optional<FrameBuffer> m_OutputFrameBuffer;
    FrameGraph::Location TexOutput;
    FrameGraph::Location VOutputSize;
    FrameGraph::Location VSkyLightMethod;
    FrameGraph::Location VUseFrustumCulling;
    FrameGraph::Location VIndirectLightSampleCount;
    FrameGraph::Location VMainCamera;
    FrameGraph::Location TexCubemap;
    FrameGraph::Location TexHDRi;
    
    // TODO introduce a proper light system
    FrameGraph::Location VMainLightDirection;
    FrameGraph::Location VMainLightColor;
    FrameGraph::Location VMainLightIntensity;
    
    FlyCamera m_ViewportCamera;
};


int main(int argc, char* argv[])
{
    // Search paths
    AddSearchPath(RESOURCES_GLOBAL);
    AddSearchPath(RESOURCES_PROJECT);
    AddSearchPath(RESOURCES_SAMPLE_SCENES);
    ShaderAddSearchPath(SHADERS_GLOBAL);
    ShaderAddSearchPath(SHADERS_PROJECT);
    
    Engine::Spec Specification;
    Specification.Register<Window::Module>();
    Specification.Register<Rendering::Module>();
    Specification.Register<AppModule>();
    Specification.Register<ImGui::Module>();
    
    Engine::App App(std::move(Specification));
    
    App.Run();
}
