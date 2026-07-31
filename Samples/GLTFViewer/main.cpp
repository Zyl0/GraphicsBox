#include "Shared/Assertion.h"
#include "Math/RMath.h"
#include "Files/Files.h"
#include "Modeling/Mesh.h"
#include "Rendering/Rendering.h"
#include "Importers/GLTF/SceneLoader.h"
#include "Camera/FlyCamera.h"

#include <imgui.h>

#include "Modules/Rendering/Shaders/Camera.h"

#include "App.h"
#include "Core/Engine.h"

// Modules
#include "Modules/ImGui/Module.h"
#include "Modules/Rendering/Module.h"
#include "Modules/Rendering/Tools/FrustumCulling.h"
#include "Modules/FrameGraph/Module.h"
#include "Modules/Window/Module.h"

// for macro keys, TODO maybe abstract into an input system or module
#include <GLFW/glfw3.h>

#include "Modules/Editor/Module.h"

using namespace Math;

#include "Modules/FrameGraph/Nodes/NativeResolutionRadiance.h"
#include "Modules/FrameGraph/Nodes/PreviousFrameRadiance.h"
#include "Modules/FrameGraph/Nodes/MotionVectors.h"
#include "Modules/FrameGraph/Nodes/SkylightToRadiance.h"
#include "Modules/FrameGraph/Nodes/MeshToSceneRadiance.h"
#include "Modules/FrameGraph/Nodes/MeshToGBuffer.h"
#include "Modules/FrameGraph/Nodes/GBufferDirectionalLightRadiance.h"
#include "Modules/FrameGraph/Nodes/GBufferIndirectLightRadiance.h"
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

        FrameGraph::Module* FrameGraph = Engine::GetModule<FrameGraph::Module>(Context());
        AssertOrError(FrameGraph != nullptr, "FrameGraph is null")

        Rendering->EnableMSAA();

        uint32_t InitialWidth, InitialHeight;
        Window->GetFrameBufferSize(InitialWidth, InitialHeight);
        
        TexCubemap = FrameGraph->Resources().Add<TextureCube>("Cubemap Skylight", 0u, 0u, Texture::Byte, Texture::R);
        TexHDRi = FrameGraph->Resources().Add<Texture2D>("HDRi Skylight", 0u, 0u, Texture::Byte, Texture::R);
        
        // TODO introduce a proper light system
        VMainLightDirection = FrameGraph->Resources().AddVariable<Vector3f>("Light Direction", Normalize(Vector3f{0.8f, -1.0f, 0.9f}));
        VMainLightColor = FrameGraph->Resources().AddVariable<Vector3f>("Light Color", {1.0f, 1.0f, 1.0f});
        VMainLightIntensity = FrameGraph->Resources().AddVariable<FrameGraph::Float>("Light Intensity", 4.3f);
        
        m_ViewportCamera.SetProjection(InitialWidth, InitialHeight, Radians(45.0f), kZNear, kZFar);
        VMainCamera = FrameGraph->Resources().AddCamera();

        // Load scene data
        {
            std::filesystem::path path;
            
                        
            if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "ABeautifulGame" / "glTF-Binary" /"ABeautifulGame.gbs" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("RTXDI-Assets") / "bistro" / "bistro.gbs", path))
            {
                AssertOrError( GBS::LoadGPUScene(path, FrameGraph->Resources().Scene()), "Failed to load scene")
            }
            else 
            if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "ABeautifulGame" / "glTF-Binary" /"ABeautifulGame.glb" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "MetalRoughSpheres" / "glTF-Binary" /"MetalRoughSpheres.glb" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "MetalRoughSpheres" / "glTF" /"MetalRoughSpheres.gltf" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("Willy") / "Splash" /"splash.gltf" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("Willy") / "BistroGLTF" /"exterior.glb" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("RTXDI-Assets") / "bistro" / "bistro.gltf", path))
            // if (GetAbsoluteFilePath(std::filesystem::path("RTXDI-Assets") / "bistro" / "bistro.gbs", path))
            {
                AssertOrError( GLTF::LoadGPUScene(path, FrameGraph->Resources().Scene()), "Failed to load scene")
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

            FrameGraph->Resources().Get<TextureCube>(TexCubemap).Data(faces);
        }
        
        // Load HDRi
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Textures") / "HDRi" / "san_giuseppe_bridge_4k.hdr" ,path))
            {
                FrameGraph->Resources().Get<Texture2D>(TexHDRi).Data(ImageLoad(path, Image::Float));
            }
        }
        
        // Define rendering pipeline
        FrameGraph::Location NativeResolutionRadianceNode = FrameGraph->PushNode<FrameGraph::NativeResolutionRadiance>();
        FrameGraph::Location PreviousFrameNode = FrameGraph->PushNode<FrameGraph::PreviousRadiance>();
        FrameGraph::Location MotionVectorsNode = FrameGraph->PushNode<FrameGraph::MotionVectors>();
        FrameGraph::Location SkylightToRadianceNode = FrameGraph->PushNode<FrameGraph::SkylightToRadiance>();
        FrameGraph::Location MeshToSceneRadianceNode = FrameGraph->PushNode<FrameGraph::MeshToSceneRadiance>();
        FrameGraph::Location MeshToGBufferNode = FrameGraph->PushNode<FrameGraph::MeshToGBuffer>();
        FrameGraph::Location GBufferDirectionalLightRadianceNode = FrameGraph->PushNode<FrameGraph::GBufferDirectionalLightRadiance>();
        FrameGraph::Location GBufferIndirectLightRadianceNode = FrameGraph->PushNode<FrameGraph::GBufferIndirectLightRadiance>();
        FrameGraph::Location ToneMappingCommandNode = FrameGraph->PushNode<FrameGraph::ToneMappingCommand>();
        
        // Forward command list (frame pipeline 0)
        FrameForward.Add(NativeResolutionRadianceNode);
        FrameForward.Add(SkylightToRadianceNode);
        FrameForward.Add(MeshToSceneRadianceNode);
        FrameForward.Add(ToneMappingCommandNode);
        FrameForward.Add(PreviousFrameNode);
        FrameForward.Add(MotionVectorsNode);

        // Defered command list (frame pipeline 1)
        FrameDeffered.Add(NativeResolutionRadianceNode);
        FrameDeffered.Add(SkylightToRadianceNode);
        FrameDeffered.Add(MeshToGBufferNode);
        FrameDeffered.Add(GBufferDirectionalLightRadianceNode);
        FrameDeffered.Add(GBufferIndirectLightRadianceNode);
        FrameDeffered.Add(ToneMappingCommandNode);
        FrameDeffered.Add(PreviousFrameNode);
        FrameDeffered.Add(MotionVectorsNode);

        // Set current frame pipeline 
        NextFramePipeline = 0;
        RefreshActiveFramePipeline(*FrameGraph);

        // Graph exposed variables
        VSkyLightMethod = FrameGraph->Resources().GetLocation<FrameGraph::UInt>("Skylight Method");
        VUseFrustumCulling = FrameGraph->Resources().GetLocation<FrameGraph::Bool>("UseFrustumCulling");
        VIndirectLightSampleCount = FrameGraph->Resources().GetLocation<FrameGraph::UInt>("Indirect Sample Count");
        VUseScreenSpaceReflections = FrameGraph->Resources().GetLocation<FrameGraph::Bool>("Use Screen Space Reflections");
        
        CurrentAntiAliasing = 0;
        VUseMSAA = FrameGraph->Resources().GetLocation<FrameGraph::Bool>("Use MSAA");
        VMSAASampleCount = FrameGraph->Resources().GetLocation<FrameGraph::UInt>("MSAA Sample Count");
    }

    void Tick(double deltaTime) override
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        FrameGraph::Module* FrameGraph = Engine::GetModule<FrameGraph::Module>(Context());
        AssertOrError(FrameGraph != nullptr, "FrameGraph is null")

        // Handle Window resize
        uint32_t NextWidth, NextHeight;
        if (Window->GetFrameBufferSize(NextWidth, NextHeight))
        {
            m_ViewportCamera.SetProjection(NextWidth, NextHeight, Math::Radians(45.0f), kZNear, kZFar);
        }

        // Update scene    
        UpdateCamera(*Window, deltaTime, m_ViewportCamera);
        FrameGraph->Resources().UpdateCamera(VMainCamera, m_ViewportCamera);

        // Update frame pipeline if needed
        if (CurrentFramePipeline != NextFramePipeline)
        {
            RefreshActiveFramePipeline(*FrameGraph);
        }
    }

    void Shutdown() override
    {
    }

    Engine::EditorWindowParams EditorWindow() override
    {
        Engine::EditorWindowParams params;
        params.Valid = true;
        params.Category = Engine::EditorWindowParams::Visualisation;
        params.Layer = Engine::EditorWindowParams::Sim;
        return params;
    }

    void EditorUI() override
    {
        FrameGraph::Module* FrameGraph = Engine::GetModule<FrameGraph::Module>(Context());
        AssertOrError(FrameGraph != nullptr, "FrameGraph is null")

        
        {
            static const char* FramePipelineNames[] =
            {
                "Forward", "Deffered"
            };
            
            int Copy = NextFramePipeline;
            if (ImGui::ListBox("Frame Pipeline", &Copy, FramePipelineNames, 2))
            {
                Copy = Math::Clamp(Copy, 0, 1);
                NextFramePipeline = Copy;

                // TODO For now Deffered does not support MSAA, falling back to No AA
                if (NextFramePipeline == 1)
                {
                    FrameGraph->Resources().SetValue<FrameGraph::Bool>(VUseMSAA, false);
                    CurrentAntiAliasing = 0;
                }
            }
        }
        {
            static const char* AntiAliasingMethodNames[] =
            {
                "None", "MSAA"
            };
            int Copy = CurrentAntiAliasing;
            if (ImGui::ListBox("Anti Aliasing Method", (int*)&Copy, AntiAliasingMethodNames, 2))
            {
                CurrentAntiAliasing = Math::Clamp(Copy, 0, 1);
                switch (CurrentAntiAliasing)
                {
                case 0:
                    FrameGraph->Resources().SetValue<FrameGraph::Bool>(VUseMSAA, false);
                    break;

                case 1:
                    FrameGraph->Resources().SetValue<FrameGraph::Bool>(VUseMSAA, true);
                    break;
                }
            }
        }
        if (FrameGraph->Resources().GetValue<FrameGraph::Bool>(VUseMSAA))
        {
            static const char* SampleCountNames[] =
            {
                "1 Sample", "2 Samples", "4 Samples", "8 Samples", "16 Samples", "32 Samples", "64 Samples"
            };
            
            int CurrentSampleCountName;
            switch (FrameGraph->Resources().GetValue<FrameGraph::UInt>(VMSAASampleCount))
            {
            case 1: CurrentSampleCountName = 0; break;
            case 2: CurrentSampleCountName = 1; break;
            case 4: CurrentSampleCountName = 2; break;
            case 8: CurrentSampleCountName = 3; break;
            case 16: CurrentSampleCountName = 4; break;
            case 32:CurrentSampleCountName = 5; break;
            case 64:CurrentSampleCountName = 6; break;
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported MSAA sample count")
            }
            
            if (ImGui::ListBox("MSAA Sample count", &CurrentSampleCountName, SampleCountNames, 7))
            {
                FrameGraph::UInt NewSampleCount;
                switch (CurrentSampleCountName)
                {
                case 0: NewSampleCount = 1; break;
                case 1: NewSampleCount = 2; break;
                case 2: NewSampleCount = 4; break;
                case 3: NewSampleCount = 8; break;
                case 4: NewSampleCount = 16; break;
                case 5: NewSampleCount = 32; break;
                case 6: NewSampleCount = 64; break;
            
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported MSAA sample count")
                }

                NewSampleCount = std::min(static_cast<int>(std::clamp(NewSampleCount, 0u, (FrameGraph::UInt)(UINT8_MAX))), MaxSupportedMSAASamples);
                FrameGraph->Resources().SetValue<FrameGraph::UInt>(VMSAASampleCount, NewSampleCount);
            }
        }

        ImGui::Separator();


        // Directional Light
        // TODO do ImGUI wrappers of graph variables
        // TODO automate exposition of graph variables on demand
        {
            Vector3f Copy = FrameGraph->Resources().GetValue<Vector3f>(VMainLightDirection);
            if (ImGui::DragFloat3("Light Direction", Copy.data(), 0.1f))
            {
                FrameGraph->Resources().SetValue<Vector3f>(VMainLightDirection, Copy);
            }
        }
        {
            Vector3f Copy = FrameGraph->Resources().GetValue<Vector3f>(VMainLightColor);
            if (ImGui::ColorEdit3("Light Color", Copy.data()))
            {
                FrameGraph->Resources().SetValue<Vector3f>(VMainLightColor, Copy);
            }
        }
        {
            float Copy = FrameGraph->Resources().GetValue<FrameGraph::Float>(VMainLightIntensity);
            if (ImGui::DragFloat("Light Intensity", &Copy, 0.1f))
            {
                FrameGraph->Resources().SetValue<FrameGraph::Float>(VMainLightIntensity, Copy);
            }
        }

        ImGui::Separator();
        
        {
            static const char* SkyLightMethodNames[] =
            {
                "Cubemap", "HDRi"
            };
            
            int Copy = FrameGraph->Resources().GetValue<FrameGraph::UInt>(VSkyLightMethod);
            if (ImGui::ListBox("Sky Light Method", &Copy, SkyLightMethodNames, 2))
            {
                Copy = Math::Clamp(Copy, 0, 1);
                FrameGraph->Resources().SetValue<FrameGraph::UInt>(VSkyLightMethod, (FrameGraph::UInt)(Copy));
            }
        }
        {
            int Copy = FrameGraph->Resources().GetValue<FrameGraph::UInt>(VIndirectLightSampleCount);
            if (ImGui::SliderInt("Indirect Light Sample Count", &Copy, 1, 1024))
            {
                Copy = Math::Clamp(Copy, 1, 1024);
                FrameGraph->Resources().SetValue<FrameGraph::UInt>(VIndirectLightSampleCount, (FrameGraph::UInt)Copy);
            }
        }

        {
            bool Copy = FrameGraph->Resources().GetValue<FrameGraph::Bool>(VUseScreenSpaceReflections);
            if (ImGui::Checkbox("Use Screen Space Reflections", &Copy))
            {
                FrameGraph->Resources().SetValue<bool>(VUseScreenSpaceReflections, Copy);
            }
        }

        ImGui::Separator();

        ImGui::SliderFloat("Camera Speed", &CameraSpeed, 0.1f, 2.0f);
        {
            bool Copy = FrameGraph->Resources().GetValue<FrameGraph::Bool>(VUseFrustumCulling);
            if (ImGui::Checkbox("Use Frustum Culling", &Copy))
            {
                FrameGraph->Resources().SetValue<FrameGraph::Bool>(VUseFrustumCulling, Copy);
            }
        }

        // TODO make a profiling window
        // ImGui::Text("Frame time (CPU): %f ms", frameTimeCPU);
        // ImGui::Text("Frame time (GPU): %f ms", frameTimeGPU);
    }
    
private:
    void RefreshActiveFramePipeline(FrameGraph::Module& FrameGraph)
    {
        FrameGraph.ClearCommandLists();
        switch (NextFramePipeline)
        {
        case 0: FrameGraph.AddCommandList(FrameForward); break;
        case 1: FrameGraph.AddCommandList(FrameDeffered); break;
        }

        CurrentFramePipeline = NextFramePipeline;
    }

    GLint MaxSupportedMSAASamples;

    FrameGraph::UInt CurrentFramePipeline;
    FrameGraph::UInt NextFramePipeline;
    FrameGraph::CommandList FrameForward;
    FrameGraph::CommandList FrameDeffered;

    FrameGraph::Location VSkyLightMethod;
    FrameGraph::Location VUseFrustumCulling;
    FrameGraph::Location VIndirectLightSampleCount;
    FrameGraph::Location VMainCamera;
    FrameGraph::Location VUseScreenSpaceReflections;
    FrameGraph::Location TexCubemap;
    FrameGraph::Location TexHDRi;
    
    // TODO introduce a proper light system
    FrameGraph::Location VMainLightDirection;
    FrameGraph::Location VMainLightColor;
    FrameGraph::Location VMainLightIntensity;
    
    FrameGraph::UInt CurrentAntiAliasing;
    FrameGraph::Location VUseMSAA;
    FrameGraph::Location VMSAASampleCount;
    
    FlyCamera m_ViewportCamera;
};


int main(int argc, char* argv[])
{
    // Search paths
    AddSearchPath(RESOURCES_GLOBAL);
    AddSearchPath(RESOURCES_PROJECT);
    AddSearchPath(RESOURCES_SAMPLE_SCENES);
    AddSearchPath(TEMP_BAKED_SCENES);
    ShaderAddSearchPath(SHADERS_GLOBAL);
    ShaderAddSearchPath(SHADERS_PROJECT);
    
    Engine::Spec Specification;
    Specification.Register<Window::Module>();
    Specification.Register<Rendering::Module>();
    Specification.Register<FrameGraph::Module>();
    Specification.Register<AppModule>();
    Specification.Register<ImGui::Module>();
    Specification.Register<Editor::Module>();
    
    Engine::App App(std::move(Specification));
    
    App.Run();
    
    return 0;
}
