#include <imgui.h>

#include "App.h"
#include "Core/Spec.h"

// Modules

#include <random>

#include "Math/RMath.h"
#include "Camera/FlyCamera.h"
#include "Modules/Window/Module.h"
#include "Modules/Rendering/Module.h"
#include "Modules/ImGui/Module.h"

#include "Files/Files.h"
#include "Importers/GLTF/SceneLoader.h"
#include "RayTracing/RayTrace.h"
#include "Rendering/Rendering.h"

#include <GLFW/glfw3.h>

#include "Image/ImageOps.h"
#include "Modules/Rendering/Shaders/FresnelSchlick.h"
#include "Modules/Rendering/Shaders/GGX.h"
#include "Modules/Rendering/Tools/DebugModule.h"
#include "RayTracers/RayTracers.h"

using namespace Math;

float CameraSpeed = 1.00f;

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
        PositionDir.z -= 1;
    if (Window.GLFWGetKey(GLFW_KEY_D))
        PositionDir.z += 1;
    
    PositionDir = PositionDir * static_cast<float>(deltaTime) * (CameraSpeed * 100) * 4.0f;
    camera.Translate(Transpose(camera.GetWorldRotation().GetRotationMatrix()) * PositionDir);
        
    if (Window.GLFWGetKey(GLFW_KEY_Q))
        rotateDir -= 1;
    if (Window.GLFWGetKey(GLFW_KEY_E))
        rotateDir += 1;
    
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
        m_ZFar = 1000.0f;
        m_RayTracingCamera.SetProjection(Width, Height, Radians(m_FOV), m_ZNear, m_ZFar);
        m_RayTracingCamera.SetTranslation(0,1,4);
        m_RayTracingCamera.SetRotationDegrees(0,-90);
        m_ViewportCamera.SetProjection(Width, Height, Radians(m_FOV), m_ZNear, m_ZFar);
        m_ViewportCamera.SetTranslation(0,1,4);
        m_ViewportCamera.SetRotationDegrees(0,-90);
        m_DrawDebugRays = false;
        m_DrawDebugTLAS = false;
        m_DrawDebugBLASes = false;
        m_DrawDebugRayTraversal = false;
        m_DebugFreezeRTCamera = false;
        m_DebugRayCoordinates = {0, 0};
        m_MaterialSampler.emplace(Sampler::Params{});
        m_RayTracingParameters = {};
        m_PathTracerParameters = {};
        m_ShadingParameters = {};
        m_RTDebugParameters = {};

        // Load scene data
        {
            // std::filesystem::path relativePath = std::filesystem::path("Meshes") / "CornellBox-Original.glb";
            // std::filesystem::path relativePath = std::filesystem::path("glTF-Sample-Assets") / "Models" / "ABeautifulGame" / "glTF-Binary" /"ABeautifulGame.glb";
            std::filesystem::path relativePath = std::filesystem::path("RTXDI-Assets") / "bistro" / "bistro.gltf";
            std::filesystem::path path;
            // if (GetAbsoluteFilePath(std::filesystem::path("Meshes") / "CornellBox-Original.glb" ,path))
            if (GetAbsoluteFilePath(relativePath ,path))
            {
                m_Scene.emplace();
                AssertOrError( GLTF::LoadCPUScene(path, *m_Scene), "Failed to load scene")
                m_MeshObjects.resize(m_Scene->meshes.size());
                for (size_t i = 0; i < m_Scene->meshes.size(); i++)
                    m_MeshObjects[i].Data(m_Scene->meshes[i]);
                
                m_Textures.reserve(m_Scene->textures.size());
                for (size_t i = 0; i < m_Scene->textures.size(); i++)
                {
                    m_Textures.emplace_back(m_Scene->textures[i]);
                    
                    m_Scene->textures[i].EnableMips();
                    GenerateMips(m_Scene->textures[i]);
                }
            }
            else
            {
                EngineRuntimeCrashF("Failed to load scene. \"%s\" no such file or directory", relativePath.generic_string().c_str())
            }
            
            // Build Acceleration Structures
            if (m_Scene)
            {
                // Build BLASes
                auto start= std::chrono::high_resolution_clock::now();
                std::map<std::pair<size_t, uint8_t>, size_t> BLASTable{};
                for (size_t meshIndex = 0; meshIndex < m_Scene->meshes.size(); meshIndex++)
                {
                    const Mesh& mesh = m_Scene->meshes[meshIndex];
                    for (uint8_t group = 0, end = mesh.GetVertexGroups().size(); group < end; group++)
                    {
                        BLASTable[std::pair(meshIndex, group)] = m_CPUMeshesBLASs.size();
                        m_CPUMeshesBLASs.emplace_back(BuildBLAS(mesh, group, 96));
                    }
                }
                auto stop= std::chrono::high_resolution_clock::now();
                long cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
                EngineLoggerErrorF("Scene meshes BLASes build took %ld ms", cpu);
                
                // Build TLAS
                start= std::chrono::high_resolution_clock::now();
                m_CPUSceneTLAS.emplace();
                for (const auto & instance : m_Scene->instances)
                {
                    Transform4f TransformMatrix;
                    const GLTF::Transform& Transform = m_Scene->transforms[instance.transform];
                    
                    // Transform
                    switch (Transform.Type)
                    {
                    case GLTF::Transform::Properties:
                        {
                            TransformMatrix = Transform.Value.asProperties.GetTransform();
                        }
                        break;
                
                    case GLTF::Transform::Matrix:
                        {
                            // if (UseFrustumCulling && !Rendering::frustumCullingTest(Resources.GetMainCameraData().Camera_WorldToProj(), Transform.Value.asMatrix, Group.BoundsMin, Group.BoundsMax)) continue;
            
                            TransformMatrix = Transform.Value.asMatrix;
                        }
                        break;
                
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
                    }
                    
                    TLASAddInstance(*m_CPUSceneTLAS, m_CPUMeshesBLASs[BLASTable[std::pair(instance.mesh, (uint8_t)instance.vertexGroup)]], instance.material, TransformMatrix);
                }
                TLASRebuild(*m_CPUSceneTLAS);
                stop= std::chrono::high_resolution_clock::now();
                cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
                EngineLoggerErrorF("Scene TLAS build took %ld ms", cpu);
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
        
        m_WriteImage.emplace(Width, Height, Image::UnsignedByte, Image::RGB, Image::sRGB, nullptr);
        m_ColorBufferImage.emplace(Width, Height, Image::Float, Image::RGB, Image::Linear, nullptr);
        m_DepthBufferImage.emplace(Width, Height, Image::Float, Image::R, Image::Linear, nullptr);
    }

    void Tick(double deltaTime) override
    {
        DebugScopeMarker scope("Draw Raster");
        
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());

        Rendering::Debug::Module* DebugRendering = Engine::GetModule<Rendering::Debug::Module>(Context());

        // Handle shader reload
        if (Window->ShouldRecompileShaders())
        {
            PipelineUpdateFromFile(*m_SamplePipeline, "MeshToFrame.glsl");
        }

        // Handle window resize
        uint32_t Width, Height;
        if (Window->GetFrameBufferSize(Width, Height))
        {
            m_ViewportCamera.SetProjection(Width, Height, Radians(m_FOV), m_ZNear, m_ZFar);
            m_ViewportCamera.SetProjection(Width, Height, Radians(m_FOV), m_ZNear, m_ZFar);
            m_WriteImage.reset();
            m_ColorBufferImage.reset();
            m_DepthBufferImage.reset();
            m_WriteImage.emplace(Width, Height, Image::UnsignedByte, Image::RGB, Image::sRGB, nullptr);
            m_ColorBufferImage.emplace(Width, Height, Image::Float, Image::RGB, Image::Linear, nullptr);
            m_DepthBufferImage.emplace(Width, Height, Image::Float, Image::R, Image::Linear, nullptr);
        }
        
        UpdateCamera(*Window, deltaTime, m_ViewportCamera);
        if (!m_DebugFreezeRTCamera)
        {
            m_RayTracingCamera = m_ViewportCamera;
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Bind(*m_SamplePipeline);
        
        SetUniform(*m_SamplePipeline, "ViewProjection", m_ViewportCamera.Projection() * m_ViewportCamera.View());
        
        SetUniform(*m_SamplePipeline, "lightColor", m_ShadingParameters.SunLightColor * m_ShadingParameters.SunLightIntensity);
        SetUniform(*m_SamplePipeline, "lightDirection", m_ShadingParameters.SunLightDirection);
        SetUniform(*m_SamplePipeline, "ambientColor",m_ShadingParameters.AmbientColor * m_ShadingParameters.AmbientIntensity);
        SetUniform(*m_SamplePipeline, "cameraPosition", m_ViewportCamera.GetWorldPosition());
        
        for (const auto & instance : m_Scene->instances)
        {            
            const MeshObject& Mesh = m_MeshObjects[instance.mesh];
            Bind(Mesh.GetVAO());
            
            const Mesh::VertexGroup& Group = Mesh.GetGroups()[instance.vertexGroup];
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
            SetUniform(*m_SamplePipeline, "BaseColor", Material.color.xyz());
            SetUniform(*m_SamplePipeline, "Emissive", Material.emissive.xyz());
            SetUniform(*m_SamplePipeline, "Roughness", Material.roughness);
            SetUniform(*m_SamplePipeline, "Metalness", Material.metallic);
            if (Material.colorTexture != UINT64_MAX)                SetUniform(*m_SamplePipeline, "texColor", 4, m_Textures[Material.colorTexture], *m_MaterialSampler);
            if (Material.normalTexture != UINT64_MAX)               SetUniform(*m_SamplePipeline, "texNormal", 5, m_Textures[Material.normalTexture], *m_MaterialSampler);
            if (Material.metallicRoughnessTexture != UINT64_MAX)    SetUniform(*m_SamplePipeline, "texMR", 6, m_Textures[Material.metallicRoughnessTexture], *m_MaterialSampler);
            if (Material.occlusionTexture != UINT64_MAX)            SetUniform(*m_SamplePipeline, "texAO", 7, m_Textures[Material.occlusionTexture], *m_MaterialSampler);
            SetUniform(*m_SamplePipeline, "UseColorTexture", Material.colorTexture != UINT64_MAX);
            SetUniform(*m_SamplePipeline, "UseNormalTexture", Material.normalTexture != UINT64_MAX);
            SetUniform(*m_SamplePipeline, "UseMRTexture", Material.metallicRoughnessTexture != UINT64_MAX);
            SetUniform(*m_SamplePipeline, "UseAOTexture", Material.occlusionTexture != UINT64_MAX);
            
            
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
        
        UnBind(*m_SamplePipeline);
        
        if (m_DebugFreezeRTCamera)
        {
            DebugRendering->DrawFrustum(Inverse(m_RayTracingCamera.Projection() * m_RayTracingCamera.View()), (m_ViewportCamera.Projection() * m_ViewportCamera.View()));
        }

        if (m_DrawDebugRays)
        {
            Width = m_WriteImage->Width();
            Height = m_WriteImage->Height();
            const Matrix4f ProjToWorld = Inverse(m_RayTracingCamera.Projection() * m_RayTracingCamera.View());
            const Matrix4f WorldToProj = m_ViewportCamera.Projection() * m_ViewportCamera.View();

            for (uint32_t y = 4; y < Height; y+=8)
            for (uint32_t x = 4; x < Width; x+=8)
            {
                Vector4f Start = ProjToWorld * Vector4f(
                    ((static_cast<float>(x) + .5f) * 2.f / static_cast<float>(Width)) - 1.f,
                    ((static_cast<float>(y) + .5f) * 2.f / static_cast<float>(Height)) - 1.f,
                    0.f, 1.f);
                Start.xyz() /= Start.w;
                Vector4f End = ProjToWorld * Vector4f(
                    ((static_cast<float>(x) + .5f) * 2.f / static_cast<float>(Width)) - 1.f,
                    ((static_cast<float>(y) + .5f) * 2.f / static_cast<float>(Height)) - 1.f,
                    1.f, 1.f);
                End.xyz() /= End.w;

                DebugRendering->DrawRay(WorldToProj,
                    Start.xyz(),
                    Magnitude(End.xyz() - Start.xyz()),
                    Normalize(End.xyz() - Start.xyz()),
                    Magnitude(End.xyz() - Start.xyz())
                );
            }
        }
        
        if (m_DrawDebugTLAS)
        {
            DebugScopeMarker scope2("Draw Debug Scene TLAS");
            
            Matrix4f ViewProj = m_ViewportCamera.Projection() * m_ViewportCamera.View();
            std::stack<uint32_t> stack;
            
            stack.push(m_CPUSceneTLAS->Head);

            while (stack.empty() == false)
            {
                uint32_t NodeIndex = stack.top();
                stack.pop();
                
                if (m_CPUSceneTLAS->Tree[NodeIndex].IsNode())
                {
                    stack.push(m_CPUSceneTLAS->Tree[NodeIndex].LeftIndex());
                    stack.push(m_CPUSceneTLAS->Tree[NodeIndex].RightIndex());
                    DebugRendering->DrawBox(ViewProj, m_CPUSceneTLAS->Tree[NodeIndex].Bounds, Vector3f(.5f,.5f,.5f));
                }
                else // if (m_Blas->Tree[NodeIndex].IsLeaf())
                {
                    const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[NodeIndex].LeftIndex()];
                    DebugRendering->DrawBox(ViewProj, m_CPUSceneTLAS->Tree[NodeIndex].Bounds, Vector3f(1.f,.7f,.3f));
                }
            }
        }
        
        if (m_DrawDebugBLASes)
        {
            DebugScopeMarker scope2("Draw Debug Scene BLASes");
            
            Matrix4f ViewProj = m_ViewportCamera.Projection() * m_ViewportCamera.View();
            for (uint32_t TLASElementIndex = 0; TLASElementIndex < m_CPUSceneTLAS->Elements.size(); TLASElementIndex++)
            {
                const TLASElement& TLASElt = m_CPUSceneTLAS->Elements[TLASElementIndex];
                
                std::stack<uint32_t> stack;
                stack.push(TLASElt.BLAS->Head);

                while (stack.empty() == false)
                {
                    uint32_t NodeIndex = stack.top();
                    stack.pop();
                
                    if (TLASElt.BLAS->Tree[NodeIndex].IsNode())
                    {
                        stack.push(TLASElt.BLAS->Tree[NodeIndex].LeftIndex());
                        stack.push(TLASElt.BLAS->Tree[NodeIndex].RightIndex());
                        DebugRendering->DrawBox(ViewProj, TLASElt.BLAS->Tree[NodeIndex].Bounds, Vector3f(.7f,1.f,.3f), TLASElt.ModelToWorld);
                    }
                    else // if (m_Blas->Tree[NodeIndex].IsLeaf())
                    {
                        DebugRendering->DrawBox(ViewProj, TLASElt.BLAS->Tree[NodeIndex].Bounds, Vector3f(.3f,.7f,1.f), TLASElt.ModelToWorld);
                    }
                }
            }
        }
    
        if (m_DrawDebugRayTraversal)
        {
            DebugScopeMarker scope2("Draw Debug Ray Traversal");
            
            m_RayTracingParameters.Shading = m_ShadingParameters;
            m_RayTracingParameters.AccelerationStructure = &(*m_CPUSceneTLAS);
            m_RayTracingParameters.BLASes = m_CPUMeshesBLASs;
            m_RayTracingParameters.Scene = &(*m_Scene);
            
            m_RTDebugParameters.EngineContext = &Context();
            
            Vector2f ScreenSpaceCoordinates = {
                ((static_cast<float>(m_DebugRayCoordinates.x) + .5f) * 2.f / static_cast<float>(Width)) - 1.f,
                ((static_cast<float>(m_DebugRayCoordinates.y) + .5f) * 2.f / static_cast<float>(Height)) - 1.f,
            };
            
            RayTracerBVH_DrawTraversal(m_RayTracingParameters, m_RTDebugParameters, m_RayTracingCamera, ScreenSpaceCoordinates);
        }
    }

    void Shutdown() override
    {
        m_SamplePipeline.reset();
        m_CPUSceneTLAS.reset();
        m_CPUMeshesBLASs.clear();
        m_Scene.reset();
        m_SkyboxHDRiCpu.reset();
        m_WriteImage.reset();
        m_SkyboxHDRiGpu.reset();
        m_MeshObjects.clear();
        m_Textures.clear();
        m_MaterialSampler.reset();
    }

    void EditorUI() override
    {
        Window::Module* Window = Engine::GetModule<Window::Module>(Context());
        uint32_t Width, Height;
        Window->GetFrameBufferSize(Width, Height);
        
        {
            ImGui::Text("Camera");
            
            ImGui::Checkbox("Freeze RT Camera", &m_DebugFreezeRTCamera);
            if (m_DebugFreezeRTCamera)
            {
                if (ImGui::Button("Reset Viewport Camera"))
                {
                    m_ViewportCamera = m_RayTracingCamera;
                }
            }
            ImGui::Checkbox("Debug draw primary rays", &m_DrawDebugRays);
            
            {
                Vector3f CamPos = m_ViewportCamera.GetWorldPosition();
                if (ImGui::InputFloat3("Camera Position", CamPos.data()))
                {
                    m_ViewportCamera.SetTranslation(CamPos);
                }
            }
            {
                float Pitch = m_ViewportCamera.GetPitchDegrees();
                float Yaw = m_ViewportCamera.GetYawDegrees();
                bool changePitch = ImGui::DragFloat("Camera Pitch", &Pitch, .1f);
                bool changeYaw = ImGui::DragFloat("Camera Yaw", &Yaw, .1f);
                if (changePitch || changeYaw)
                {
                    m_ViewportCamera.SetRotationDegrees(Pitch, Yaw);
                }
            }
        }
        
        ImGui::Separator();
            
        if (ImGui::Button("Take screenshot"))
        {
            std::filesystem::path exportPath = std::filesystem::path(TEMP_DIR) / "screenshot.png";

            auto start= std::chrono::high_resolution_clock::now();
            RayTracedScreenshot();
            auto stop= std::chrono::high_resolution_clock::now();
            long cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            EngineLoggerLogF("Ray traced screenshot took %ld ms", cpu);

            ImageStore(exportPath, *m_WriteImage, Image::JPG);
        }
        
        if (ImGui::Button("Take screenshot BVH"))
        {
            std::filesystem::path exportPath = std::filesystem::path(TEMP_DIR) / "screenshotBVH.png";

            auto start= std::chrono::high_resolution_clock::now();
            RayTracedScreenshotBVH();
            auto stop= std::chrono::high_resolution_clock::now();
            long cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            EngineLoggerLogF("Ray traced screenshot with BVH took %ld ms", cpu);

            ImageStore(exportPath, *m_WriteImage, Image::JPG);
        }
        
        ImGui::Checkbox("Debug draw TLAS", &m_DrawDebugTLAS);
        ImGui::Checkbox("Debug draw all BLASes", &m_DrawDebugBLASes);
        
        ImGui::Checkbox("Debug draw Ray traversal", &m_DrawDebugRayTraversal);
        ImGui::Checkbox("Debug draw traversal shadow rays", &m_RTDebugParameters.DebugSunlight);
        ImGui::Checkbox("Debug draw traversal indirect rays", &m_RTDebugParameters.DebugIndirectLight);
        ImGui::Checkbox("Debug freeze indirect ray gen", &m_RTDebugParameters.FreezeRandomSequence);
        
        if (m_DrawDebugRayTraversal)
        {
            ImGui::DragInt2("Debug Ray Coordinates", m_DebugRayCoordinates.data());
            m_DebugRayCoordinates.x = std::clamp(m_DebugRayCoordinates.x, 0, (int)Width - 1);
            m_DebugRayCoordinates.y = std::clamp(m_DebugRayCoordinates.y, 0, (int)Height - 1);
        }
        
        if (ImGui::Button("Take shaded screenshot BVH"))
        {
            std::filesystem::path exportPath = std::filesystem::path(TEMP_DIR) / "shadedScreenshotBVH.bmp";

            auto start= std::chrono::high_resolution_clock::now();
            RayTracedScreenshotBVHShaded();
            auto stop= std::chrono::high_resolution_clock::now();
            long cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            EngineLoggerLogF("Ray traced screenshot with BVH took %ld ms", cpu);
            
            ImageStore(exportPath, *m_WriteImage, Image::BMP);
        }
        
        ImGui::Separator();
        
        {
            ImGui::Text("Shading");
            
            ImGui::DragFloat3("Sun Light Direction", m_ShadingParameters.SunLightDirection.data(), 0.01f);
            m_ShadingParameters.SunLightDirection = Normalize(m_ShadingParameters.SunLightDirection);
            
            ImGui::ColorEdit3("LightColor", m_ShadingParameters.SunLightColor.data());
            ImGui::DragFloat("Light intensity", &m_ShadingParameters.SunLightIntensity);
            ImGui::SliderFloat("Sunlight Soft Shadows Angle", &m_ShadingParameters.SunLightSoftShadowHalfAngleDegrees, 0.f, 4.f);
            
            ImGui::ColorEdit3("SkyLightColor", m_ShadingParameters.AmbientColor.data());
            ImGui::DragFloat("SkyLightIntensity", &m_ShadingParameters.AmbientIntensity, 0.01f);
        }
        
        ImGui::Separator();
        
        {
            ImGui::Text("Ray Tracing");
            
            // TODO reintroduce AA
            // ImGui::SliderInt("Multi Sample Count", &m_RayTracingParameters.m_MultiSampleCount, 1, 32);
            
            ImGui::Checkbox("Render Direct Lights", &m_RayTracingParameters.RenderSunlight);
            ImGui::SliderInt("Sunlight ray count", &m_RayTracingParameters.SunlightSampleCount, 1, 16);
            
            ImGui::Checkbox("Render Indirect Lighting", &m_RayTracingParameters.RenderIndirectLight);
            ImGui::SliderInt("Indirect Sample Count", &m_RayTracingParameters.IndirectSampleCount, 1, 1024);
            ImGui::SliderInt("Indirect Bounce Count", &m_RayTracingParameters.IndirectSampleBounceCount, 1, 8);
            
            ImGui::Checkbox("Render Emissive Lighting", &m_RayTracingParameters.RenderEmissiveIndirectLights);
            ImGui::SliderInt("Emissive Sample Count", &m_RayTracingParameters.EmissiveSurfaceSampleCount, 1, 16);
        }
        
        ImGui::Separator();
        
        {
            ImGui::Text("Path Tracing");
            ImGui::Text("TODO");
        }
    }
    
    void RayTracedScreenshot()
    {
        ImageBuffer<Vector3t<uint8_t>> TargetImage(*m_WriteImage);
        ImageBuffer<Vector3f> RTColorImage(*m_ColorBufferImage);
        ImageBuffer<float> RTDepthImage(*m_DepthBufferImage);
        
        m_RayTracingParameters.Shading = m_ShadingParameters;
        m_RayTracingParameters.AccelerationStructure = &(*m_CPUSceneTLAS);
        m_RayTracingParameters.BLASes = m_CPUMeshesBLASs;
        m_RayTracingParameters.Scene = &(*m_Scene);
        RayTracerDummy_RenderShaded(RTColorImage, RTDepthImage, m_RayTracingParameters, m_RayTracingCamera);
        
        // TODO tonemapping
        
        const uint32_t Width = m_WriteImage->Width(); 
        const uint32_t Height = m_WriteImage->Height();
        
#pragma omp parallel for collapse(2)
        for (int y = 0; y < Height; y++)
        for (int x = 0; x < Width; x++)
        {
            WriteBuffer(TargetImage, x, y, ReadBuffer(RTColorImage, x, y));
        }
    }
    
    void RayTracedScreenshotBVH()
    {
        ImageBuffer<Vector3t<uint8_t>> TargetImage(*m_WriteImage);
        ImageBuffer<Vector3f> RTColorImage(*m_ColorBufferImage);
        ImageBuffer<float> RTDepthImage(*m_DepthBufferImage);
        
        m_RayTracingParameters.Shading = m_ShadingParameters;
        m_RayTracingParameters.AccelerationStructure = &(*m_CPUSceneTLAS);
        m_RayTracingParameters.BLASes = m_CPUMeshesBLASs;
        m_RayTracingParameters.Scene = &(*m_Scene);
        RayTracerBVH_RenderPrimaryRay(RTColorImage, m_RayTracingParameters, m_RayTracingCamera);
        
        // TODO tonemapping
        
        const uint32_t Width = m_WriteImage->Width(); 
        const uint32_t Height = m_WriteImage->Height();
        
#pragma omp parallel for collapse(2)
        for (int y = 0; y < Height; y++)
        for (int x = 0; x < Width; x++)
        {
            WriteBuffer(TargetImage, x, y, ReadBuffer(RTColorImage, x, y));
        }
    }

    void RayTracedScreenshotBVHShaded()
    {
        ImageBuffer<Vector3t<uint8_t>> TargetImage(*m_WriteImage);
        ImageBuffer<Vector3f> RTColorImage(*m_ColorBufferImage);
        ImageBuffer<float> RTDepthImage(*m_DepthBufferImage);
        
        m_RayTracingParameters.Shading = m_ShadingParameters;
        m_RayTracingParameters.AccelerationStructure = &(*m_CPUSceneTLAS);
        m_RayTracingParameters.BLASes = m_CPUMeshesBLASs;
        m_RayTracingParameters.Scene = &(*m_Scene);
        RayTracerBVH_RenderShaded(RTColorImage, RTDepthImage, m_RayTracingParameters, m_RayTracingCamera);
        
        // TODO tonemapping
        
        const uint32_t Width = m_WriteImage->Width(); 
        const uint32_t Height = m_WriteImage->Height();
        
#pragma omp parallel for collapse(2)
        for (int y = 0; y < Height; y++)
        for (int x = 0; x < Width; x++)
        {
            WriteBuffer(TargetImage, x, y, ReadBuffer(RTColorImage, x, y));
        }
    }
    
private:
    float m_FOV;
    float m_ZNear;
    float m_ZFar;
    RayTracerFrameParameters m_RayTracingParameters;
    PathTracerFrameParameters m_PathTracerParameters;
    RayShadingParameters m_ShadingParameters;
    TraversalDebuggerParameters m_RTDebugParameters;

    bool m_DrawDebugRays;
    bool m_DrawDebugTLAS;
    bool m_DrawDebugBLASes;
    bool m_DrawDebugRayTraversal;
    bool m_DebugFreezeRTCamera;
    Vector2t<int> m_DebugRayCoordinates;
    
    std::optional<GLTF::CPUScene> m_Scene;
    std::optional<Image> m_SkyboxHDRiCpu;
    std::optional<Image> m_WriteImage;
    std::optional<Image> m_ColorBufferImage;
    std::optional<Image> m_DepthBufferImage;
    
    std::optional<Pipeline> m_SamplePipeline;
    std::optional<Texture2D> m_SkyboxHDRiGpu;
    std::vector<MeshObject> m_MeshObjects;
    std::vector<Texture2D> m_Textures;
    std::optional<Sampler> m_MaterialSampler;
    
    std::vector<BLAS> m_CPUMeshesBLASs;
    std::optional<TLAS> m_CPUSceneTLAS;
    
    FlyCamera m_RayTracingCamera;
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
    Specification.Register<Rendering::Debug::Module>();
    Specification.Register<ImGui::Module>();
    Specification.Register<AppModule>();
    
    Engine::App App(std::move(Specification));
    
    App.Run();
}
