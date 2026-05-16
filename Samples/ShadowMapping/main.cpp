#include "Shared/Assertion.h"
#include "Math/RMath.h"
#include "Files/Files.h"
#include "Modeling/Mesh.h"
#include "Rendering/Rendering.h"
#include "Importers/GLTF/SceneLoader.h"

#include <imgui.h>

#include "Camera/FlyCamera.h"
#include "Camera/OrthographicCamera.h"

#include "App.h"
#include "Core/Engine.h"

// Modules
#include "Modules/ImGui/Module.h"
#include "Modules/Rendering/Module.h"
#include "Modules/Rendering/Tools/FrustumCulling.h"
#include "Modules/Window/Module.h"

// for macro keys, TODO maybe abstract into an input system or module
#include <GLFW/glfw3.h>

#include "Modules/Rendering/Shaders/Camera.h"

using namespace Math;

/* ____________________________________ Constants ____________________________________ */

constexpr size_t kBaseWidth = 1280;
constexpr size_t kBaseHeight = 720;
constexpr float kZNear = 0.01f;
constexpr float kZFar = 1000.0f;
constexpr uint16_t kBaseShadowMapResolution = 1024;
constexpr uint16_t kBaseShadowCount = 8;
constexpr uint16_t kBaseCameraCount = 8;

/* ____________________________________ States ____________________________________ */

bool RequestShaderReload = false;
bool RequestRebake = true;
bool RebakeEveryFrame = false;

float CameraSpeed = 1.0f;

bool UseFrustumCulling = false;

bool DebugDrawLightObserverFrustum = false;

/* ____________________________________ Window ____________________________________ */

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

/* ____________________________________ Render Data ____________________________________ */

struct LightColor_t
{
    Vector3f LightColor = {1.0f, 1.0f, 1.0f};
    float LightIntensity = 3.2f;
};

struct DirectionalLight_t
{
    LightColor_t Color;
    
    uint32_t Camera;
    uint32_t pad[3];
};

struct SceneBuffers
{
    GLTF::GPUScene Scene;

    UniformBuffer DirectionalLight{};
    // StorageBuffer PointLights{};
    StorageBuffer Cameras{kBaseCameraCount * sizeof(Rendering::CameraData), nullptr};

    std::vector<Rendering::CameraData> CamerasData{kBaseCameraCount};
    
    TextureCube SkylightCube{0, 0, Texture::Byte, Texture::R};
    Texture2D SkylightHDRI{0, 0, Texture::Byte, Texture::R};
    Texture2D ShadowMap{kBaseShadowMapResolution, kBaseShadowMapResolution, Texture::UnsignedInt, Texture::D};
    
    Sampler BaseSampler{{}};
    Sampler ShadowMapSampler{{.WarpModeU = Sampler::W_ClampToEdge, .WarpModeV = Sampler::W_ClampToEdge, .WarpModeW = Sampler::W_ClampToEdge}};
    
    // States
    uint32_t SkylightMethod = 1;
    uint32_t MainCameraIndex = 0;
    uint32_t CameraCount = 0;
    float ShadowMapBias = 0.01f;
    
    SceneBuffers() = default;
};

void UpdateGPUCamera(SceneBuffers& Scene, size_t Index, const Camera& Camera)
{
    UpdateCameraData(Scene.CamerasData[Index], Camera);

    Scene.Cameras.SubData(&(Scene.CamerasData[Index]), sizeof(Rendering::CameraData), sizeof(Rendering::CameraData) * Index);
}

size_t AddGPUCamera(SceneBuffers& Scene)
{
    // Resize if needed
    if (Scene.CameraCount == Scene.CamerasData.size())
    {
        Scene.CamerasData.resize(Scene.CameraCount * 2);
        Scene.Cameras.Data(Scene.CamerasData.data(), sizeof(Rendering::CameraData) * Scene.CameraCount * 2);
    }

    return Scene.CameraCount++;
}

size_t AddGPUCamera(SceneBuffers& Scene, const Camera& camera)
{
    size_t CameraID = AddGPUCamera(Scene);

    UpdateGPUCamera(Scene, CameraID, camera);

    return CameraID;
}

void UpdateLightObserver(OrthographicCamera& Observer, const Camera& SceneCamera, Vector3f DirectionFromLight, float Size)
{
    Vector3f DirectionTowardsLight = -DirectionFromLight;

    Vector3f Center = SceneCamera.GetWorldDirection() + SceneCamera.GetWorldDirection() * Size;
    Vector3f CameraPosition = Center + DirectionTowardsLight * Size / 2.0f;

    Observer.LookAt(CameraPosition, DirectionFromLight, Vector3f(0, 1, 0));
}

/* ____________________________________ Helpers ____________________________________ */

/* ____________________________________ Baking Passes ____________________________________ */

/* ____________________________________ Real time Passes ____________________________________ */

class DrawShadowMap
{
public:
    DrawShadowMap():
        m_Pipeline(PipelineFromFile("Draw Shadow Map", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "MeshToDepthBuffer.glsl"))
    {}
    
    void Draw(const SceneBuffers& SceneObjects, const Camera& LightObserver)
    {
        DebugScopeMarker scope("Draw Shadowmap");

        Matrix4f ViewProj = LightObserver.Projection() * LightObserver.View();
        
        Bind(m_Pipeline);

        // Scene storage buffers
        SetUniform(0, SceneObjects.Cameras);
        SetUniform(m_Pipeline, "CameraIndex", 1u);

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
        
                SetUniform(m_Pipeline, "Model", TransformMatrix);
            }
            break;
                
            case GLTF::Transform::Matrix:
            {
                if (UseFrustumCulling && !Rendering::frustumCullingTest(ViewProj, Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
        
                SetUniform(m_Pipeline, "Model", Transform.Value.asMatrix);
            }
            break;
            
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }

            
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
        
        UnBind(m_Pipeline);
    }
    
    void Reload()
    {
        PipelineUpdateFromFile(m_Pipeline, "MeshToDepthBuffer.glsl");
    }
    
private:
    Pipeline m_Pipeline;
};

class DebugDrawFrustum
{
public:
    DebugDrawFrustum():
        m_Pipeline(PipelineFromFile("Debug Draw Frustum", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "DebugCube.glsl"))
    {}

    void Draw(const SceneBuffers& SceneObjects, uint32_t SourceCamera, uint32_t TargetCamera)
    {
        DebugScopeMarker scope("Debug Draw Frustum");

        Bind(m_Pipeline);

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        // Scene storage buffers
        SetUniform(0, SceneObjects.Cameras);
        
        SetUniform(m_Pipeline, "SourceCamera", SourceCamera);
        SetUniform(m_Pipeline, "TargetCamera", TargetCamera);

        glDrawArrays(GL_QUADS, 0, 24);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        UnBind(m_Pipeline);
    }

    void Reload()
    {
        PipelineUpdateFromFile(m_Pipeline, "DebugCube.glsl");
    }
private:
    Pipeline m_Pipeline;
};

class DrawSky
{
public:
    DrawSky() : 
        m_PipelineCubemap(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineCubemapDefines)),
        m_PipelineHDRI(PipelineFromFile("Background sky", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "SkylightToRadiance.glsl", m_PipelineHDRIDefines))
    {}
    
    // void Update(double DeltaTime);
    void Draw(const SceneBuffers& SceneObjects)
    {
        DebugScopeMarker scope("Draw Sky");

        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Cubemap Sampling
            Bind(m_PipelineCubemap);
                
            // Scene storage buffers
            SetUniform(0, SceneObjects.Cameras);

            // Scene texture buffers
            SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.SkylightCube, SceneObjects.BaseSampler);
        
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(m_PipelineCubemap);
            break;
            
        case 1: // HDRI Sampling
            Bind(m_PipelineHDRI);
                
            // Scene storage buffers
            SetUniform(0, SceneObjects.Cameras);

            // Scene texture buffers
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
    void Draw(const SceneBuffers& SceneObjects, const Camera& Camera)
    {
        DebugScopeMarker scope("Draw Scene");
        
        const Pipeline* pipeline = nullptr;
        Matrix4f ViewProj = Camera.Projection() * Camera.View();
        
        switch (SceneObjects.SkylightMethod)
        {
        case 0: // Cubemap Sampling
            Bind(m_PipelineCubemap);

            // Scene storage buffers
            SetUniform(0, SceneObjects.Cameras);
        
            // Scene uniform buffers
            SetUniform(0, SceneObjects.DirectionalLight);
                
            // Scene texture buffers
            SetUniform(m_PipelineCubemap, "SkyLightCubeMap", 0, SceneObjects.SkylightCube, SceneObjects.BaseSampler);
            SetUniform(m_PipelineCubemap, "ShadowMap", 1, SceneObjects.ShadowMap, SceneObjects.ShadowMapSampler);
            SetUniform(m_PipelineCubemap, "SkyLightMipCount",  SceneObjects.SkylightCube.MipCount());
            
            pipeline = &m_PipelineCubemap;
            break;
            
        case 1: // HDRI Sampling
            Bind(m_PipelineHDRI);
                
            // Scene storage buffers
            SetUniform(0, SceneObjects.Cameras);
        
            // Scene uniform buffers
            SetUniform(0, SceneObjects.DirectionalLight);
                
            // Scene texture buffers
            SetUniform(m_PipelineHDRI, "SkyLightHDRi", 0, SceneObjects.SkylightHDRI, SceneObjects.BaseSampler);
            SetUniform(m_PipelineHDRI, "ShadowMap", 1, SceneObjects.ShadowMap, SceneObjects.ShadowMapSampler);
            SetUniform(m_PipelineHDRI, "SkyLightMipCount",  SceneObjects.SkylightHDRI.MipCount());
            
            pipeline = &m_PipelineHDRI;
            break;
                        
        default:
            return;
        }

        SetUniform(*pipeline, "ShadowMapBias", SceneObjects.ShadowMapBias);
        SetUniform(*pipeline, "ShadowMapResolution", SceneObjects.ShadowMap.Width());
        SetUniform(*pipeline, "IndirectLightingSampleCount", m_SkyLightSampleCount);
        
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
            if (Material.colorTexture != UINT64_MAX) SetUniform(*pipeline, "texColor", 2, SceneObjects.Scene.textures[Material.colorTexture], SceneObjects.BaseSampler);
            if (Material.normalTexture != UINT64_MAX) SetUniform(*pipeline, "texNormal", 3, SceneObjects.Scene.textures[Material.normalTexture], SceneObjects.BaseSampler);
            if (Material.metallicRoughnessTexture != UINT64_MAX) SetUniform(*pipeline, "texMR", 4, SceneObjects.Scene.textures[Material.metallicRoughnessTexture], SceneObjects.BaseSampler);
            if (Material.occlusionTexture != UINT64_MAX) SetUniform(*pipeline, "texAO", 5, SceneObjects.Scene.textures[Material.occlusionTexture], SceneObjects.BaseSampler);
            
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
    void Draw(const SceneBuffers& SceneObjects, const Texture2D& SceneRadiance)
    {
        DebugScopeMarker scope("Apply Tone Mapping");
        
        Bind(m_Pipeline);
        
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
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")

        Rendering::Module* Rendering = Engine::GetModule<Rendering::Module>(Context());
        AssertOrError(Rendering != nullptr, "Rendering is null")
        
        uint32_t InitialWidth, InitialHeight;
        Window->GetFrameBufferSize(InitialWidth, InitialHeight);
        m_Camera.SetProjection(InitialWidth, InitialHeight, Math::Radians(45.0f), kZNear, kZFar);
        m_Camera.SetTranslation(-4,1,0);
        m_SunlightCamera.SetOrthographicProjection(5,5,0,10);
        
        m_SceneBuffers.emplace();
        m_SceneRadiance.emplace(InitialWidth, InitialHeight, Texture::Packed_R11F_G11F_B10F, Texture::RGB);
        m_SceneDepth.emplace(InitialWidth, InitialHeight, Texture::UnsignedInt, Texture::D);
        FrameBuffer::DepthAttachment SceneDepthAttachment(*m_SceneDepth);
        m_SceneRadianceFB.emplace(FrameBuffer::Attachment(*m_SceneRadiance, FrameBuffer::ClearColor(0.f)), &SceneDepthAttachment);
        FrameBuffer::DepthAttachment ShadowMapDepthAttachment(m_SceneBuffers->ShadowMap);
        m_ShadowMapFB.emplace(kBaseShadowMapResolution, kBaseShadowMapResolution, ShadowMapDepthAttachment);
        m_DrawShadowMapPass.emplace();
        m_DrawSkyPass.emplace();
        m_DrawScenePass.emplace();
        m_DrawPostProcessPass.emplace();
        m_DebugDrawFrustumPass.emplace();
        
        // Load scene data
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Meshes") / "Cubes.glb" ,path))
            {
                AssertOrError( GLTF::LoadGPUScene(path, m_SceneBuffers->Scene), "Failed to load scene")
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

            m_SceneBuffers->SkylightCube.Data(faces);
        }
        
        // Load HDRi
        {
            std::filesystem::path path;
            if (GetAbsoluteFilePath(std::filesystem::path("Textures") / "HDRi" / "san_giuseppe_bridge_4k.hdr" ,path))
            {
                m_SceneBuffers->SkylightHDRI.Data(ImageLoad(path, Image::Float));
            }
        }
        
        m_LightRotation = {0, Radians(28.0f), Radians(-29.5f)};
        MainCameraData = AddGPUCamera(*m_SceneBuffers);
        LightObserverCameraData = AddGPUCamera(*m_SceneBuffers);
        m_LightData = {};
        m_LightData.Camera = LightObserverCameraData;
    }
    
    void Tick(double deltaTime) override
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        AssertOrError(Window != nullptr, "Window is null")
        
        // Handle Window resize
        uint32_t NextWidth, NextHeight;
        if (Window->GetFrameBufferSize(NextWidth, NextHeight))
        {
            m_Camera.SetProjection(NextWidth, NextHeight, Math::Radians(45.0f), kZNear, kZFar);
                
            m_SceneRadiance->Data(NextWidth, NextHeight);
            m_SceneDepth->Data(NextWidth, NextHeight);
            m_SceneRadianceFB->Resize(NextWidth, NextHeight);
        }
        
        // Handle Shader Reload
        if (Window->ShouldRecompileShaders())
        {
            m_DrawSkyPass->Reload();
            m_DrawScenePass->Reload();
            m_DrawPostProcessPass->Reload();
            m_DrawShadowMapPass->Reload();
            
            RequestRebake = true;
        }
        
        // Update scene    
        {
            UpdateCamera(*Window, deltaTime, m_Camera);
            UpdateLightObserver(m_SunlightCamera, m_Camera, m_LightRotation(m_LightBaseDirection), 2.5);

            UpdateGPUCamera(*m_SceneBuffers, MainCameraData, m_Camera);
            UpdateGPUCamera(*m_SceneBuffers, LightObserverCameraData, m_SunlightCamera);

            m_SceneBuffers->DirectionalLight.Data(&m_LightData, sizeof(m_LightData));
        }
        
        // Draw
        {
            Bind(*m_ShadowMapFB);

            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);
            m_DrawShadowMapPass->Draw(*m_SceneBuffers, m_SunlightCamera);

            glDisable(GL_DEPTH_TEST);
                
            Bind(*m_SceneRadianceFB);
            m_SceneRadianceFB->Clear();
                
            m_DrawSkyPass->Draw(*m_SceneBuffers);
                
            glEnable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glClear(GL_DEPTH_BUFFER_BIT);
                
            m_DrawScenePass->Draw(*m_SceneBuffers, m_Camera);
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);

            if (DebugDrawLightObserverFrustum)
            {
                m_DebugDrawFrustumPass->Draw(*m_SceneBuffers, LightObserverCameraData, MainCameraData);
            }
                
            UnBind(*m_SceneRadianceFB);
            
            m_DrawPostProcessPass->Draw(*m_SceneBuffers, *m_SceneRadiance);
        }
    }
    
    void Shutdown() override
    {
        m_DebugDrawFrustumPass.reset();
        m_DrawPostProcessPass.reset();
        m_DrawScenePass.reset();
        m_DrawSkyPass.reset();
        m_DrawShadowMapPass.reset();
        m_ShadowMapFB.reset();
        m_SceneRadianceFB.reset();
        m_SceneDepth.reset();
        m_SceneRadiance.reset();
        m_SceneBuffers.reset();
    }
    
    void EditorUI() override
    {
        // Directional Light
        {
            Vector3f Angles = m_LightRotation.GetAngles();
            Angles.x = Degrees(Angles.x);
            Angles.y = Degrees(Angles.y);
            Angles.z = Degrees(Angles.z);

            ImGui::DragFloat3("Light Rotation", Angles.data(), 0.25, -180, 180);

            Angles.x = Radians(Angles.x);
            Angles.y = Radians(Angles.y);
            Angles.z = Radians(Angles.z);
            
            m_LightRotation = QuaternionF(Angles.x, Angles.y, Angles.z);
        }
        ImGui::ColorEdit3("Light Color", m_LightData.Color.LightColor.data());
        ImGui::SliderFloat("Light Intensity", &(m_LightData.Color.LightIntensity), 0.1f, 10.0f);

        
        ImGui::Separator();

        static const char* SkyLightMethodNames[] =
        {
            "Cubemap", "HDRi"
        };
        ImGui::ListBox("Sky Light Method", (int*)&(m_SceneBuffers->SkylightMethod), SkyLightMethodNames, 2);
        m_SceneBuffers->SkylightMethod = Math::Clamp(m_SceneBuffers->SkylightMethod, 0u, 1u);
        
        ImGui::SliderInt("Sky light Sample Count", (int*)&(m_DrawScenePass->SkyLightSampleCount()), 1, 1024);

        ImGui::Separator();
        ImGui::Checkbox("Light Draw Frustum", &DebugDrawLightObserverFrustum);
        ImGui::SliderFloat("Shadow Map bias", &(m_SceneBuffers->ShadowMapBias), std::numeric_limits<float>::epsilon(), 0.1f);
        
        ImGui::Separator();
        
        if (m_SceneBuffers->SkylightMethod == 3)
        {
            ImGui::Checkbox("Rebake data every frame", &RebakeEveryFrame);
            if (ImGui::Button("Rebake data"))
            {
                RequestRebake = true;
            }
            
            ImGui::Separator();
        }
        
        ImGui::SliderFloat("Camera Speed", &CameraSpeed, 0.1f, 2.0f);
        ImGui::Checkbox("Use Frustum Culling", &UseFrustumCulling);
    }
    
private:
    std::optional<SceneBuffers> m_SceneBuffers;
    
    std::optional<Texture2D> m_SceneRadiance;
    std::optional<Texture2D> m_SceneDepth;
    std::optional<FrameBuffer> m_SceneRadianceFB;
    
    std::optional<FrameBuffer> m_ShadowMapFB;
    
    // Passes
    std::optional<DrawShadowMap> m_DrawShadowMapPass;
    std::optional<DrawSky> m_DrawSkyPass;
    std::optional<DrawScene> m_DrawScenePass;
    std::optional<PostProcess> m_DrawPostProcessPass;

    // Debug passes
    std::optional<DebugDrawFrustum> m_DebugDrawFrustumPass;
    
    FlyCamera m_Camera;
    OrthographicCamera m_SunlightCamera;
    size_t MainCameraData;
    size_t LightObserverCameraData;
    
    DirectionalLight_t m_LightData;
    Vector3f m_LightBaseDirection{0, 0, 1};
    QuaternionF m_LightRotation;
};

int main(void)
{    
    AddSearchPath(RESOURCES_GLOBAL);
    AddSearchPath(RESOURCES_PROJECT);

    ShaderAddSearchPath(SHADERS_GLOBAL);
    ShaderAddSearchPath(SHADERS_PROJECT);
    
    Engine::Spec Specification;
    Specification.Register<Window::Module>();
    Specification.Register<Rendering::Module>();
    Specification.Register<AppModule>();
    Specification.Register<ImGui::Module>();
    
#ifndef CONFIG_DEBUG
    UseFrustumCulling = true;
#endif // CONFIG_DEBUG 
    
    Engine::App App(std::move(Specification));
    
    App.Run();
    
    return 0;
}