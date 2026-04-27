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

using namespace Math;

/* ____________________________________ Constants ____________________________________ */

constexpr float kZNear = 0.01f;
constexpr float kZFar = 1000.0f;
constexpr uint32_t kBaseAntiAliasingMethod = 1; // MSAA
constexpr uint8_t kBaseSampleCount = 4;

/* ____________________________________ States ____________________________________ */

float CameraSpeed = 1.0f;
bool UseFrustumCulling = false;

/* ____________________________________ Render Data ____________________________________ */

struct DirectionalLight
{
    AlignedVector3f LightDir = AlignedVector3f(Normalize(Vector3f{0.8f, -1.0f, 0.9f}));
    
    Vector3f LightColor = {1.0f, 1.0f, 1.0f};
    float LightIntensity = 1.0f;
};

struct CPUSceneData
{
    FlyCamera Camera;
    Rendering::CameraData CameraData;
    DirectionalLight lightsData{};
};

// TODO move to scene, referencing and building manually scene graph is bad
struct GPuSceneBuffers
{
    GPuSceneBuffers(uint32_t width, uint32_t height, GLint MaxSupportedMSAASamples):
        CurrentWidth(width), CurrentHeight(height),
        MSAASampleCount(std::min(static_cast<uint8_t>(std::clamp(MaxSupportedMSAASamples, 0, (int)(UINT8_MAX))), kBaseSampleCount)),
        SceneRadianceRT(CurrentWidth, CurrentHeight, Texture::Packed_R11F_G11F_B10F, Texture::RGB),
        SceneDepthRT(CurrentWidth, CurrentHeight, Texture::UnsignedInt, Texture::D),
        SceneDepthAttachment(SceneDepthRT),
        SceneRadianceFB(FrameBuffer::Attachment(SceneRadianceRT, FrameBuffer::ClearColor(0.f)), &SceneDepthAttachment),
        SceneRadianceMSAART(CurrentWidth, CurrentHeight, Texture::Packed_R11F_G11F_B10F, Texture::RGB, MSAASampleCount),
        SceneDepthMSAART(CurrentWidth, CurrentHeight, Texture::UnsignedInt, Texture::D, MSAASampleCount),
        SceneDepthAttachmentMSAA(SceneDepthMSAART),
        SceneRadianceMSAAFB(FrameBuffer::Attachment(SceneRadianceMSAART, FrameBuffer::ClearColor(0.f)), &SceneDepthAttachmentMSAA)
    {}

    // States
    uint32_t CurrentWidth, CurrentHeight;
    uint32_t SkylightMethod = 1;
    uint32_t AntiAliasingMethod = kBaseAntiAliasingMethod;
    uint8_t MSAASampleCount;
    
    GLTF::GPUScene Scene;
    UniformBuffer Lights;
    UniformBuffer Camera;

    TextureCube SkylightCube{0, 0, Texture::Byte, Texture::R};
    Texture2D SkylightHDRI{0, 0, Texture::Byte, Texture::R};

    Texture2D SceneRadianceRT;
    Texture2D SceneDepthRT;
    FrameBuffer::DepthAttachment SceneDepthAttachment;
        
    FrameBuffer SceneRadianceFB;
        
    Texture2D SceneRadianceMSAART;
    WriteOnlyTexture2D SceneDepthMSAART;
    FrameBuffer::DepthAttachment SceneDepthAttachmentMSAA;
        
    FrameBuffer SceneRadianceMSAAFB;
        
    
    Sampler BaseSampler{{}};
};

/* ____________________________________ Baking Passes ____________________________________ */

/* ____________________________________ Real time Passes ____________________________________ */

class DrawSky
{
public:
    DrawSky() : 
        m_PipelineCubemap(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineCubemapDefines)),
        m_PipelineHDRI(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineHDRIDefines))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const GPuSceneBuffers& SceneObjects)
    {
        DebugScopeMarker scope("Draw Sky");

        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Cubemap Sampling
            Bind(m_PipelineCubemap);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.SkylightCube, SceneObjects.BaseSampler);
        
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(m_PipelineCubemap);
            break;
            
        case 1: // HDRI Sampling
            Bind(m_PipelineHDRI);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineHDRI, "SkyLightHDRi", 0, SceneObjects.SkylightHDRI, SceneObjects.BaseSampler);
        
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(m_PipelineHDRI);
            break;
        }
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_PipelineCubemap, "SkylightToRadiance.glsl", m_PipelineCubemapDefines);
        PipelineUpdateFromFile(m_PipelineHDRI, "SkylightToRadiance.glsl", m_PipelineHDRIDefines);
    }

private:
    Shader::DefineArray<1> m_PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
    Shader::DefineArray<1> m_PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
    
    Pipeline m_PipelineCubemap;
    Pipeline m_PipelineHDRI;
};

class DrawScene
{
public:    
    DrawScene() : 
        m_PipelineCubemap(PipelineFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToRadiance.glsl", m_PipelineCubemapDefines)),
        m_PipelineHDRI(PipelineFromFile("Mesh To Radiance", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToRadiance.glsl", m_PipelineHDRIDefines))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const GPuSceneBuffers& SceneObjects, const Camera& Camera)
    {
        DebugScopeMarker scope("Draw Scene");
        
        const Pipeline* pipeline = nullptr;
        Matrix4f ViewProj = Camera.Projection() * Camera.View();
        
        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Cubemap Sampling
            Bind(m_PipelineCubemap);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.SkylightCube, SceneObjects.BaseSampler);
            SetUniform(m_PipelineCubemap, "SkyLightMipCount",  SceneObjects.SkylightCube.MipCount());
            
            SetUniform(m_PipelineCubemap, "IndirectLightingSampleCount", m_SkyLightSampleCount);
            
            pipeline = &m_PipelineCubemap;
            break;
            
        case 1: // HDRI Sampling
            Bind(m_PipelineHDRI);
                
            // Scene buffers
            SetUniform(0, SceneObjects.Camera);
            SetUniform(1, SceneObjects.Lights);
            SetUniform(m_PipelineHDRI, "SkyLightHDRi", 0, SceneObjects.SkylightHDRI, SceneObjects.BaseSampler);
            SetUniform(m_PipelineHDRI, "SkyLightMipCount",  SceneObjects.SkylightHDRI.MipCount());
            
            SetUniform(m_PipelineCubemap, "IndirectLightingSampleCount", m_SkyLightSampleCount);
            
            pipeline = &m_PipelineHDRI;
            break;
                        
        default:
            return;
        }
        
        for (const GLTF::MeshInstance& Instance : SceneObjects.Scene.instances)
        {
            const MeshObject& Mesh = SceneObjects.Scene.meshes[Instance.mesh];
            const Mesh::VertexGroup& Group = Mesh.GetGroups()[Instance.vertexGroup];
            const GLTF::Transform& Transform = SceneObjects.Scene.transforms[Instance.transform];
            const GLTF::Material& Material = SceneObjects.Scene.materials[Instance.material];
            
            switch (Transform.Type)
            {
            case GLTF::Transform::Properties:
                {
                    Transform4f TransformMatrix = Transform.Value.asProperties.GetTransform();
            
                    if (UseFrustumCulling && !Rendering::frustumCullingTest(ViewProj, TransformMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
            
                    SetUniform(*pipeline, "Model", TransformMatrix);
                }
                break;
                
            case GLTF::Transform::Matrix:
                {
                    if (UseFrustumCulling && !Rendering::frustumCullingTest(ViewProj, Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
            
                    SetUniform(*pipeline, "Model", Transform.Value.asMatrix);
                }
                break;
                
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }

            
            // Material
            SetUniform(*pipeline, "BaseColor", Material.color.XYZ());
            SetUniform(*pipeline, "Roughness", Material.roughness);
            SetUniform(*pipeline, "Metalness", Material.metallic);
            SetUniform(*pipeline, "UseColorTexture", Material.colorTexture != UINT64_MAX);
            SetUniform(*pipeline, "UseNormalTexture", Material.normalTexture != UINT64_MAX);
            SetUniform(*pipeline, "UseMRTexture", Material.metallicRoughnessTexture != UINT64_MAX);
            SetUniform(*pipeline, "UseAOTexture", Material.occlusionTexture != UINT64_MAX);
            if (Material.colorTexture != UINT64_MAX) SetUniform(*pipeline, "texColor", 1, SceneObjects.Scene.textures[Material.colorTexture], SceneObjects.BaseSampler);
            if (Material.normalTexture != UINT64_MAX) SetUniform(*pipeline, "texNormal", 2, SceneObjects.Scene.textures[Material.normalTexture], SceneObjects.BaseSampler);
            if (Material.metallicRoughnessTexture != UINT64_MAX) SetUniform(*pipeline, "texMR", 3, SceneObjects.Scene.textures[Material.metallicRoughnessTexture], SceneObjects.BaseSampler);
            if (Material.occlusionTexture != UINT64_MAX) SetUniform(*pipeline, "texAO", 4, SceneObjects.Scene.textures[Material.occlusionTexture], SceneObjects.BaseSampler);
            
            Bind(Mesh.GetVAO());
            if (Mesh.GetIndexBuffer().has_value())
            {
                const IndexBuffer& indexBuffer = Mesh.GetIndexBuffer().value();
                Bind(indexBuffer);
            
                glDrawElements(ToGLGeometryType(Mesh.GetVertexType()), Group.VertexCount, ToGLIndexType(indexBuffer.GetIndexType()), (void*)(Group.FirstVertex * ToGLIndexSize(indexBuffer.GetIndexType())));
            
                UnBind(indexBuffer);
            }
            else
            {
                glDrawArrays(ToGLGeometryType(Mesh.GetVertexType()), Group.FirstVertex, Group.VertexCount);
            }
            
            UnBind(Mesh.GetVAO());
        }
        
        UnBind(*pipeline);
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_PipelineCubemap, "MeshToRadiance.glsl", m_PipelineCubemapDefines);
        PipelineUpdateFromFile(m_PipelineHDRI, "MeshToRadiance.glsl", m_PipelineHDRIDefines);
    }
    
    uint32_t& SkyLightSampleCount() {return m_SkyLightSampleCount;}
    
private:
    Shader::DefineArray<1> m_PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
    Shader::DefineArray<1> m_PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
    
    Pipeline m_PipelineCubemap;
    Pipeline m_PipelineHDRI;
    
    uint32_t m_SkyLightSampleCount = 32;
};

class PostProcess
{
public:

    PostProcess() : 
        m_Pipeline(PipelineFromFile("Post Process", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "PostProcess.glsl")),
        m_Sampler({
            .Magnification = Sampler::F_Nearest,
            .Minification = Sampler::F_Nearest,
        })
    {}
    
    // void Update(double DeltaTime);
    void Draw(const GPuSceneBuffers& SceneObjects, const Texture2D& SceneRadiance)
    {
        DebugScopeMarker scope("Apply Tone Mapping");
        
        Bind(m_Pipeline);
        
        // Scene buffers
        SetUniform(0, SceneObjects.Camera);
        SetUniform(1, SceneObjects.Lights);
        
        SetUniform(m_Pipeline, "SceneRadiance", 0, SceneRadiance, m_Sampler);
        
        // Draw screen quad
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        UnBind(m_Pipeline);
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_Pipeline, "PostProcess.glsl");
    }

private:
    Pipeline m_Pipeline;
    Sampler m_Sampler;
};

/* ____________________________________ Process ____________________________________ */

struct GPUDrawCalls
{
    GPUDrawCalls() = default;
    ~GPUDrawCalls() = default;
    
    DrawSky DrawSkyPass;
    DrawScene DrawScenePass;
    PostProcess PostProcessPass;
};

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
        
        m_CPUSceneData.emplace();
        m_GPUScene.emplace(InitialWidth, InitialHeight, MaxSupportedMSAASamples);
        m_GPUDrawCalls.emplace();

        // Load scene data
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "ABeautifulGame" / "glTF-Binary" /"ABeautifulGame.glb" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "MetalRoughSpheres" / "glTF-Binary" /"MetalRoughSpheres.glb" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("glTF-Sample-Assets") / "Models" / "MetalRoughSpheres" / "glTF" /"MetalRoughSpheres.gltf" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("Willy") / "Splash" /"splash.gltf" ,path))
            // if (GetAbsoluteFilePath(std::filesystem::path("Willy") / "BistroGLTF" /"exterior.glb" ,path))
            {
                AssertOrError( GLTF::LoadGPUScene(path, m_GPUScene->Scene), "Failed to load scene")
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

            m_GPUScene->SkylightCube.Data(faces);

            // Load HDRi
            {
                std::filesystem::path path;
                if (GetAbsoluteFilePath(std::filesystem::path("Textures") / "HDRi" / "san_giuseppe_bridge_4k.hdr" ,path))
                {
                    m_GPUScene->SkylightHDRI.Data(ImageLoad(path, Image::Float));
                }
            }
        }
    }

    void Tick(double deltaTime) override
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        // Handle Window resize
        uint32_t NextWidth, NextHeight;
        if (Window->GetFrameBufferSize(NextWidth, NextHeight))
        {
            m_CPUSceneData->Camera.SetProjection(NextWidth, NextHeight, Math::Radians(45.0f), kZNear, kZFar);
                
            m_GPUScene->SceneRadianceRT.Data(NextWidth, NextHeight);
            m_GPUScene->SceneDepthRT.Data(NextWidth, NextHeight);
            m_GPUScene->SceneRadianceFB.Resize(NextWidth, NextHeight);
                
            m_GPUScene->SceneRadianceMSAART.Data(NextWidth, NextHeight);
            m_GPUScene->SceneDepthMSAART.Data(NextWidth, NextHeight);
            m_GPUScene->SceneRadianceMSAAFB.Resize(NextWidth, NextHeight);

            m_GPUScene->CurrentWidth = NextWidth;
            m_GPUScene->CurrentHeight = NextHeight;
        }

        // Handle Shader Reload
        if (Window->ShouldRecompileShaders())
        {
            m_GPUDrawCalls->DrawSkyPass.Reload();
            m_GPUDrawCalls->DrawScenePass.Reload();
            m_GPUDrawCalls->PostProcessPass.Reload();
        }

        // Update scene
        {                
            UpdateCamera(*Window, deltaTime, m_CPUSceneData->Camera);
            Rendering::UpdateCameraData(m_CPUSceneData->CameraData, m_CPUSceneData->Camera);
                
            m_GPUScene->Camera.Data(&(m_CPUSceneData->CameraData), sizeof(m_CPUSceneData->CameraData));
            m_GPUScene->Lights.Data(&(m_CPUSceneData->lightsData), sizeof(m_CPUSceneData->lightsData));
        }

        // Draw scene
        {
            switch (m_GPUScene->AntiAliasingMethod)
            {
            case 0: // NONE
                Bind(m_GPUScene->SceneRadianceFB);
                m_GPUScene->SceneRadianceFB.Clear();
                
                m_GPUDrawCalls->DrawSkyPass.Draw(*m_GPUScene);
                
                glEnable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);
                glClear(GL_DEPTH_BUFFER_BIT);
                
                m_GPUDrawCalls->DrawScenePass.Draw(*m_GPUScene, m_CPUSceneData->Camera);
                glDisable(GL_CULL_FACE);
                glDisable(GL_DEPTH_TEST);
                
                UnBind(m_GPUScene->SceneRadianceFB);
                
                m_GPUDrawCalls->PostProcessPass.Draw(*m_GPUScene, m_GPUScene->SceneRadianceRT);
                break;
                    
            case 1: // MSAA
                Bind(m_GPUScene->SceneRadianceMSAAFB);
                m_GPUScene->SceneRadianceMSAAFB.Clear();
                
                m_GPUDrawCalls->DrawSkyPass.Draw(*m_GPUScene);
                
                glEnable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);
                glClear(GL_DEPTH_BUFFER_BIT);
                
                m_GPUDrawCalls->DrawScenePass.Draw(*m_GPUScene, m_CPUSceneData->Camera);
                glDisable(GL_CULL_FACE);
                glDisable(GL_DEPTH_TEST);
                    
                Bind(m_GPUScene->SceneRadianceFB, m_GPUScene->SceneRadianceMSAAFB);
                    
                glBlitFramebuffer(
                    0, 0, m_GPUScene->CurrentWidth, m_GPUScene->CurrentHeight,
                    0, 0, m_GPUScene->CurrentWidth, m_GPUScene->CurrentHeight,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST );
                
                UnBind(m_GPUScene->SceneRadianceFB);
                
                m_GPUDrawCalls->PostProcessPass.Draw(*m_GPUScene, m_GPUScene->SceneRadianceRT);
                break;
                    
                SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported anti aliasing method")
                }
        }
            
    }

    void Shutdown() override
    {
        m_GPUScene.reset();
        m_GPUDrawCalls.reset();
        m_CPUSceneData.reset();
    }

    void EditorUI() override
    {
        // Directional Light
        ImGui::DragFloat3("Light Direction", m_CPUSceneData->lightsData.LightDir.vector.data(), 0.1f);
        ImGui::ColorEdit3("Light Color", m_CPUSceneData->lightsData.LightColor.data());
        ImGui::SliderFloat("Light Intensity", &m_CPUSceneData->lightsData.LightIntensity, 0.1f, 10.0f);

        ImGui::Separator();

        static const char* SkyLightMethodNames[] =
        {
            "Cubemap", "HDRi"
        };

        ImGui::ListBox("Sky Light Method", (int*)&m_GPUScene->SkylightMethod, SkyLightMethodNames, 2);
        m_GPUScene->SkylightMethod = Math::Clamp(m_GPUScene->SkylightMethod, 0u, 1u);

        ImGui::Separator();

        ImGui::SliderFloat("Camera Speed", &CameraSpeed, 0.1f, 2.0f);
        ImGui::Checkbox("Use Frustum Culling", &UseFrustumCulling);

        // TODO make a profiling window
        // ImGui::Text("Frame time (CPU): %f ms", frameTimeCPU);
        // ImGui::Text("Frame time (GPU): %f ms", frameTimeGPU);
    }
    
private:
    GLint MaxSupportedMSAASamples;

    std::optional<CPUSceneData> m_CPUSceneData;
    std::optional<GPuSceneBuffers> m_GPUScene;
    std::optional<GPUDrawCalls> m_GPUDrawCalls;
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
