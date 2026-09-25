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
#include "Modules/Rendering/Shaders/LightinModel.h"
#include "Modules/Rendering/Tools/DebugModule.h"

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
        m_ZFar = 1000.0f;
        m_LightColor = {1.0f, 0.97f, 0.9f};
        m_LightDirection = Normalize(Math::Vector3f(.1, .9, .2));
        m_LightIntensity = 3.0f;
        m_AmbientColor = {0.7f, 0.78f, 1.0f};
        m_AmbientIntensity = 0.08f;
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
        m_DrawDebugRayTraversalIndirectLight = false;
        m_DrawDebugRayTraversalShadowMap = false;
        m_DebugFreezeRTCamera = false;
        m_DebugRayCoordinates = {0, 0};
        m_MaterialSampler.emplace(Sampler::Params{});
        m_IndirectSampleCount = 128;

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
                    m_Textures.emplace_back(m_Scene->textures[i]);
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
                        m_CPUMeshesBLASs.emplace_back(BuildBLAS(mesh, group));
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
            m_WriteImage.emplace(Width, Height, Image::UnsignedByte, Image::RGB, Image::sRGB, nullptr);
        }
        
        UpdateCamera(*Window, deltaTime, m_ViewportCamera);
        if (!m_DebugFreezeRTCamera)
        {
            m_RayTracingCamera = m_ViewportCamera;
        }
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        Bind(*m_SamplePipeline);
        
        SetUniform(*m_SamplePipeline, "ViewProjection", m_ViewportCamera.Projection() * m_ViewportCamera.View());
        
        SetUniform(*m_SamplePipeline, "lightColor", m_LightColor * m_LightIntensity);
        SetUniform(*m_SamplePipeline, "lightDirection", m_LightDirection);
        SetUniform(*m_SamplePipeline, "ambientColor",m_AmbientColor * m_AmbientIntensity);
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
            SetUniform(*m_SamplePipeline, "BaseColor", Material.color.XYZ());
            SetUniform(*m_SamplePipeline, "Emissive", Material.emissive.XYZ());
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
            
            Width = m_WriteImage->Width();
            Height = m_WriteImage->Height();
            const Matrix4f ProjToWorld = Inverse(m_RayTracingCamera.Projection() * m_RayTracingCamera.View());
            const Matrix4f WorldToViewportProj = m_ViewportCamera.Projection() * m_ViewportCamera.View();
            
            Vector4f Start = ProjToWorld * Vector4f(
                ((static_cast<float>(m_DebugRayCoordinates.x) + .5f) * 2.f / static_cast<float>(Width)) - 1.f,
                ((static_cast<float>(m_DebugRayCoordinates.y) + .5f) * 2.f / static_cast<float>(Height)) - 1.f,
                0.f, 1.f);
            Start.xyz() /= Start.w;
            Vector4f End = ProjToWorld * Vector4f(
                ((static_cast<float>(m_DebugRayCoordinates.x) + .5f) * 2.f / static_cast<float>(Width)) - 1.f,
                ((static_cast<float>(m_DebugRayCoordinates.y) + .5f) * 2.f / static_cast<float>(Height)) - 1.f,
                1.f, 1.f);
            End.xyz() /= End.w;
            
            Ray PrimaryRay = {
                .origin = Start.xyz(), 
                .direction = Normalize(End.xyz() - Start.xyz()), 
                .distance = m_ZFar - m_ZNear
            };
            
            Hit ClosestHit = Hit(); 
            const Mesh* Mesh = nullptr;
            const Matrix4f* ModelMatrix = nullptr;
            size_t Material = 0;        
            TraceRayTLAS DebugRayTracerTLAS(*m_CPUSceneTLAS, PrimaryRay);    
            for (BVHHit TLASHit : DebugRayTracerTLAS)
            {
                const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[TLASHit.NodeIndex].LeftIndex()];
                
                TraceRayBLAS PrimaryRayTracerBLAS(*(Elt.BLAS), PrimaryRay, Elt.WorldToModel);
                
                for (Hit Hit : PrimaryRayTracerBLAS)
                {                    
                    // BLAS Any Hit   
                }
                
                if (Hit Closest = PrimaryRayTracerBLAS.ClosestHit(); Closest)
                {
                    // BLAS Closest Hit
                    if (ClosestHit && ClosestHit.t < Closest.t) continue;

                    ClosestHit = Closest;
                    Mesh = Elt.BLAS->Meta.MeshRef;
                    ModelMatrix = &Elt.ModelToWorld;
                    Material = Elt.MaterialIndex;
                }
                else
                {
                    // Miss
                }
            }
            
            if (ClosestHit)
            {
                static const Matrix3f RotationX = Matrix3f::RotationY(M_PI / 2.0);
                
                DebugRendering->DrawRay(WorldToViewportProj, PrimaryRay, ClosestHit.t);
                
                Point3f Position;
                Vector3f Normal;
                Vector3f Tangent = Vector3f(0);
                switch (Mesh->GetMeshType())
                {
                case Mesh::POINTS:
                case Mesh::LINE_STRIP:
                case Mesh::LINE_LOOP:
                case Mesh::LINES:
                case Mesh::LINE_STRIP_ADJACENCY:
                case Mesh::LINES_ADJACENCY:
                case Mesh::PATCHES:
                case Mesh::QUAD_STRIP:
                case Mesh::QUADS:
                case Mesh::_Count:
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported vertex type. Expected triangles")

                case Mesh::TRIANGLE_STRIP_ADJACENCY:
                case Mesh::TRIANGLES_ADJACENCY:
                case Mesh::TRIANGLE_STRIP:
                case Mesh::TRIANGLE_FAN:
                case Mesh::TRIANGLES:
                    Mesh::ConstFace face(*Mesh, ClosestHit.face);
                    Mesh::ConstVertex a = face[0];
                    Mesh::ConstVertex b = face[1];
                    Mesh::ConstVertex c = face[2];
            
                    Vector4f PositionH = ((*ModelMatrix) * Vector4f(VertexInterpolateTriangle(ClosestHit, a.Position(), b.Position(), c.Position()), 1.0f));
                    Position = PositionH.xyz() / PositionH.w;
                    Normal = Normalize(((*ModelMatrix) * Vector4f(VertexInterpolateTriangle(ClosestHit, a.Normal(), b.Normal(), c.Normal()), 0.0f)).xyz());
                    break;
                }
                
                
                const GLTF::Material& material = m_Scene->materials[Material];
                
                float PixRoughness = material.roughness;
                float Alpha = PixRoughness * PixRoughness;
                
                if (m_DrawDebugRayTraversalShadowMap)
                {
                    Ray SunLightRay = {
                        .origin = PrimaryRay.origin + PrimaryRay.direction * ClosestHit.t + m_LightDirection * 0.01f, 
                        .direction = m_LightDirection, 
                        .distance = 300.f
                    };
                    TraceRayTLAS SunlightRayTracerTLAS(*m_CPUSceneTLAS, SunLightRay);
                
                    bool HasHit = false;
                    float t;
                    for (BVHHit TLASHit : SunlightRayTracerTLAS)
                    {
                        const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[TLASHit.NodeIndex].LeftIndex()];
                    
                        TraceRayBLAS SunlightRayTracerBLAS(*(Elt.BLAS), SunLightRay, Elt.WorldToModel);
                    
                        for (Hit Hit : SunlightRayTracerBLAS)
                        {                    
                            // BLAS Any Hit   
                            HasHit = true;
                            t = Hit.t;
                        }
                        if (HasHit) break;
                    }
                    if (HasHit)
                    {
                        DebugRendering->DrawRay(WorldToViewportProj, SunLightRay, PrimaryRay.distance);
                    }
                    else
                    {
                        DebugRendering->DrawRay(WorldToViewportProj, SunLightRay, PrimaryRay.distance);
                    }
                }
                
                if (m_DrawDebugRayTraversalIndirectLight)
                {                    
                    Tangent = RotationX * Normal;
                    Vector3f FragBiTangent = Cross(Normal, Tangent); // always re-derive B from N×T
                    Matrix3f InvTBN = Matrix3f(Tangent, FragBiTangent, Normal);
                    Matrix3f FragTBN = Transpose(InvTBN);
                    
                    Vector3f v = Normalize(m_RayTracingCamera.GetWorldPosition() - Position);

                    Vector3f vNormalSpace = InvTBN * v;
                    
                    std::random_device hwseed;
                    std::default_random_engine rng( hwseed() );
                    std::uniform_real_distribution<float> uniform(0, 1);
                    
                    for (size_t i = 0; i < m_IndirectSampleCount; ++i)
                    {
                        float u1 = uniform( rng );
                        float u2 = uniform( rng );
                            
                        Vector3f SampledFace = Rendering::SampleGGXVNDF_Intel2023(vNormalSpace, Alpha, Alpha, u1, u2);
                        Vector3f ne = Normalize(FragTBN * SampledFace);
                        Vector3f l = Reflect(v, ne);
                        
                        Ray IndirectLightRay = {
                            .origin = PrimaryRay.origin + PrimaryRay.direction * ClosestHit.t + l * 0.01f, 
                            .direction = l, 
                            .distance = 50.f
                        };
                        
                        Hit ClosestIndirectHit{};
                        TraceRayTLAS IndirectLightRayTracerTLAS(*m_CPUSceneTLAS, IndirectLightRay);
                        for (BVHHit TLASHit : IndirectLightRayTracerTLAS)
                        {
                            const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[TLASHit.NodeIndex].LeftIndex()];
                        
                            TraceRayBLAS IndirectLightRayTracerBLAS(*(Elt.BLAS), IndirectLightRay, Elt.WorldToModel);
                        
                            for (Hit Hit : IndirectLightRayTracerBLAS)
                            {                    
                                // BLAS Any Hit   
                            }
                
                            if (Hit Closest = IndirectLightRayTracerBLAS.ClosestHit(); Closest)
                            {
                                // BLAS Closest Hit
                                ClosestIndirectHit = Closest;
                            }
                            else
                            {
                                // Miss
                            }
                        }
                        
                        if (ClosestIndirectHit)
                        {
                            DebugRendering->DrawRay(WorldToViewportProj, IndirectLightRay, ClosestIndirectHit.t);
                        }
                        else
                        {
                            DebugRendering->DrawRay(WorldToViewportProj, IndirectLightRay, IndirectLightRay.distance);
                        }
                    }
                }
            }
            else
            {
                DebugRendering->DrawRay(WorldToViewportProj, PrimaryRay, PrimaryRay.distance);
            }
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
        
        ImGui::Checkbox("Freeze RT Camera", &m_DebugFreezeRTCamera);
        if (m_DebugFreezeRTCamera)
        {
            if (ImGui::Button("Reset Viewport Camera"))
            {
                m_ViewportCamera = m_RayTracingCamera;
            }
        }
            
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

        ImGui::Checkbox("Debug draw primary rays", &m_DrawDebugRays);
        
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
        ImGui::Checkbox("Debug draw traversal shadow rays", &m_DrawDebugRayTraversalShadowMap);
        ImGui::Checkbox("Debug draw traversal indirect rays", &m_DrawDebugRayTraversalIndirectLight);
        
        if (m_DrawDebugRayTraversal)
        {
            ImGui::DragInt2("Debug Ray Coordinates", m_DebugRayCoordinates.data());
            m_DebugRayCoordinates.x = std::clamp(m_DebugRayCoordinates.x, 0, (int)Width - 1);
            m_DebugRayCoordinates.y = std::clamp(m_DebugRayCoordinates.y, 0, (int)Height - 1);
        }
        
        if (ImGui::Button("Take shaded screenshot BVH"))
        {
            std::filesystem::path exportPath = std::filesystem::path(TEMP_DIR) / "shadedScreenshotBVH.png";

            auto start= std::chrono::high_resolution_clock::now();
            RayTracedScreenshotBVHShaded();
            auto stop= std::chrono::high_resolution_clock::now();
            long cpu= std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
            EngineLoggerLogF("Ray traced screenshot with BVH took %ld ms", cpu);
            
            ImageStore(exportPath, *m_WriteImage, Image::JPG);
        }
        
        ImGui::Separator();
        
        {
            ImGui::ColorEdit3("LightColor", m_LightColor.data());
            ImGui::DragFloat("LightIntensity", &m_LightIntensity, 0.01f);
            
            ImGui::ColorEdit3("SkyLightColor", m_AmbientColor.data());
            ImGui::DragFloat("SkyLightIntensity", &m_AmbientIntensity, 0.01f);
            
            ImGui::SliderInt("Indirect Sample Count", &m_IndirectSampleCount, 1, 1024);
        }
    }
    
    void RayTracedScreenshot()
    {
        ImageBuffer<Vector3t<uint8_t>> TargetImage(*m_WriteImage);
        ClearBuffer(TargetImage);

        const uint32_t Width = m_WriteImage->Width(); 
        const uint32_t Height = m_WriteImage->Height();
        const Matrix4f ProjToWorld = m_RayTracingCamera.InverseView() * m_RayTracingCamera.InverseProjection();
        // const Matrix4f ProjToWorld = Inverse(m_Camera.Projection() * m_Camera.View());

        std::vector<Matrix4f> WorldToModelMatrix(m_Scene->transforms.size());
        for (size_t i = 0; i < m_Scene->transforms.size(); ++i)
        {
            const GLTF::Transform& Transform = m_Scene->transforms[i];
            switch (Transform.Type)
            {
            case GLTF::Transform::Properties:
                {
                    WorldToModelMatrix[i] = Inverse(Transform.Value.asProperties.GetTransform());
                }
                break;

            case GLTF::Transform::Matrix:
                {
                    WorldToModelMatrix[i] = Inverse(Transform.Value.asMatrix);
                }
                break;

            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
        }

        #pragma omp parallel for collapse(2)
        for (int y = 0; y < Height; y++)
        for (int x = 0; x < Width; x++)
        {
            Vector4f Start = ProjToWorld * Vector4f(
                ((static_cast<float>(x) + .5f) / static_cast<float>(Width)) * 2.f - 1.f,
                -(((static_cast<float>(y) + .5f) / static_cast<float>(Height)) * 2.f - 1.f),
                0.f, 1.f);
            Start.xyz() /= Start.w;
            Vector4f End = ProjToWorld * Vector4f(
                ((static_cast<float>(x) + .5f) / static_cast<float>(Width)) * 2.f - 1.f,
                -(((static_cast<float>(y) + .5f) / static_cast<float>(Height)) * 2.f - 1.f),
                1.f, 1.f);
            End.xyz() /= End.w;
            
            Ray PrimaryRay = {
                .origin = Start.xyz(), 
                .direction = Normalize(End.xyz() - Start.xyz()), 
                .distance = Magnitude(End.xyz() - Start.xyz())
            };

            Hit ClosestHit = Hit();
            GLTF::MeshInstance ClosestHitInstance;

            // Traversal without BVH
            for (const auto & instance : m_Scene->instances)
            {
                const Mesh& mesh = m_Scene->meshes[instance.mesh];
                const GLTF::Material& material = m_Scene->materials[instance.material];
                const Mesh::VertexGroup& group = mesh.GetVertexGroups()[instance.vertexGroup];
                const Matrix4f& transform = WorldToModelMatrix[instance.transform];

                TraceRay PrimaryRayTracer(mesh, group.FirstVertex, group.VertexCount, PrimaryRay, transform);

                for (Hit hit : PrimaryRayTracer)
                {
                    // Any hit
                }

                if (Hit Closest = PrimaryRayTracer.ClosestHit(); Closest)
                {
                    // Closest Hit
                    if (ClosestHit && ClosestHit.t < Closest.t) continue;

                    ClosestHit = Closest;
                    ClosestHitInstance = instance;
                }
                else
                {
                    // Miss
                }
            }

            // Skip background
            if (!ClosestHit) continue;

            // Evaluate material
            Mesh& mesh = m_Scene->meshes[ClosestHitInstance.mesh];
            const GLTF::Material& material = m_Scene->materials[ClosestHitInstance.material];
            Vector3f Color{};
            switch (mesh.GetMeshType())
            {
            case Mesh::POINTS:
            case Mesh::LINE_STRIP:
            case Mesh::LINE_LOOP:
            case Mesh::LINES:
            case Mesh::LINE_STRIP_ADJACENCY:
            case Mesh::LINES_ADJACENCY:
            case Mesh::PATCHES:
            case Mesh::QUAD_STRIP:
            case Mesh::QUADS:
            case Mesh::_Count:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported vertex type. Expected triangles")

            case Mesh::TRIANGLE_STRIP_ADJACENCY:
            case Mesh::TRIANGLES_ADJACENCY:
            case Mesh::TRIANGLE_STRIP:
            case Mesh::TRIANGLE_FAN:
            case Mesh::TRIANGLES:
                //Mesh::Face face(mesh, ClosestHit.face);
                //Mesh::Vertex a = face[0];
                //Mesh::Vertex b = face[1];
                //Mesh::Vertex c = face[2];
                //
                // Color = material.color.XYZ();
                Color.x = ClosestHit.u;
                Color.y = ClosestHit.v;
                Color.z = Saturate(1 - (ClosestHit.u + ClosestHit.v));
            }

            WriteBuffer(TargetImage, x, y, Color);
        }
    }
    
    void RayTracedScreenshotBVH()
    {
        ImageBuffer<Vector3t<uint8_t>> TargetImage(*m_WriteImage);
        ClearBuffer(TargetImage);

        const uint32_t Width = m_WriteImage->Width(); 
        const uint32_t Height = m_WriteImage->Height();
        const Matrix4f ProjToWorld = m_RayTracingCamera.InverseView() * m_RayTracingCamera.InverseProjection();
        // const Matrix4f ProjToWorld = Inverse(m_Camera.Projection() * m_Camera.View());

        std::vector<Matrix4f> WorldToModelMatrix(m_Scene->transforms.size());
        for (size_t i = 0; i < m_Scene->transforms.size(); ++i)
        {
            const GLTF::Transform& Transform = m_Scene->transforms[i];
            switch (Transform.Type)
            {
            case GLTF::Transform::Properties:
                {
                    WorldToModelMatrix[i] = Inverse(Transform.Value.asProperties.GetTransform());
                }
                break;

            case GLTF::Transform::Matrix:
                {
                    WorldToModelMatrix[i] = Inverse(Transform.Value.asMatrix);
                }
                break;

            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
        }

        #pragma omp parallel for collapse(2)
        for (int y = 0; y < Height; y++)
        for (int x = 0; x < Width; x++)
        {
            Vector4f Start = ProjToWorld * Vector4f(
                ((static_cast<float>(x) + .5f) / static_cast<float>(Width)) * 2.f - 1.f,
                -(((static_cast<float>(y) + .5f) / static_cast<float>(Height)) * 2.f - 1.f),
                0.f, 1.f);
            Start.xyz() /= Start.w;
            Vector4f End = ProjToWorld * Vector4f(
                ((static_cast<float>(x) + .5f) / static_cast<float>(Width)) * 2.f - 1.f,
                -(((static_cast<float>(y) + .5f) / static_cast<float>(Height)) * 2.f - 1.f),
                1.f, 1.f);
            End.xyz() /= End.w;
            
            Ray PrimaryRay = {
                .origin = Start.xyz(), 
                .direction = Normalize(End.xyz() - Start.xyz()), 
                .distance = Magnitude(End.xyz() - Start.xyz())
            };
            
            // if (x == (Width / 2) && y == (Height / 2)) {EngineRuntimeBREAKPOINT}

            Hit ClosestHit = Hit();
            const Mesh* Mesh = nullptr;
            size_t Material = 0;
            
            TraceRayTLAS PrimaryRayTracerTLAS(*m_CPUSceneTLAS, PrimaryRay);
            
            for (BVHHit TLASHit : PrimaryRayTracerTLAS)
            {
                const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[TLASHit.NodeIndex].LeftIndex()];
                
                TraceRayBLAS PrimaryRayTracerBLAS(*(Elt.BLAS), PrimaryRay, Elt.WorldToModel);
                
                for (Hit Hit : PrimaryRayTracerBLAS)
                {                    
                    // BLAS Any Hit   
                }
                
                if (Hit Closest = PrimaryRayTracerBLAS.ClosestHit(); Closest)
                {
                    // BLAS Closest Hit
                    if (ClosestHit && ClosestHit.t < Closest.t) continue;

                    ClosestHit = Closest;
                    Mesh = Elt.BLAS->Meta.MeshRef;
                    Material = Elt.MaterialIndex;
                }
                else
                {
                    // Miss
                }
            }
            
            // Skip background
            if (!ClosestHit) continue;

            // Evaluate material
            const GLTF::Material& material = m_Scene->materials[Material];
            Vector3f Color{};
            switch (Mesh->GetMeshType())
            {
            case Mesh::POINTS:
            case Mesh::LINE_STRIP:
            case Mesh::LINE_LOOP:
            case Mesh::LINES:
            case Mesh::LINE_STRIP_ADJACENCY:
            case Mesh::LINES_ADJACENCY:
            case Mesh::PATCHES:
            case Mesh::QUAD_STRIP:
            case Mesh::QUADS:
            case Mesh::_Count:
            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported vertex type. Expected triangles")

            case Mesh::TRIANGLE_STRIP_ADJACENCY:
            case Mesh::TRIANGLES_ADJACENCY:
            case Mesh::TRIANGLE_STRIP:
            case Mesh::TRIANGLE_FAN:
            case Mesh::TRIANGLES:
                //Mesh::Face face(mesh, ClosestHit.face);
                //Mesh::Vertex a = face[0];
                //Mesh::Vertex b = face[1];
                //Mesh::Vertex c = face[2];
                //
                // Color = material.color.XYZ();
                Color.x = ClosestHit.u;
                Color.y = ClosestHit.v;
                Color.z = Saturate(1 - (ClosestHit.u + ClosestHit.v));
            }

            WriteBuffer(TargetImage, x, y, Color);
        }
    }

    void RayTracedScreenshotBVHShaded()
    {
        static const Matrix3f RotationX = Matrix3f::RotationY(M_PI / 2.0);
        /*
         (
            1,0,0,  // 1,0,0,
            0,0,-1, // 0,cos (M_PI / 2.0), -sin(M_PI / 2.0),
            0,1,0   // 0,sin(M_PI / 2.0), cos (M_PI / 2.0)
        );
        */
        
        ImageBuffer<Vector3t<uint8_t>> TargetImage(*m_WriteImage);
        ClearBuffer(TargetImage);

        const uint32_t Width = m_WriteImage->Width(); 
        const uint32_t Height = m_WriteImage->Height();
        const Matrix4f ProjToWorld = m_RayTracingCamera.InverseView() * m_RayTracingCamera.InverseProjection();
        // const Matrix4f ProjToWorld = Inverse(m_Camera.Projection() * m_Camera.View());

        std::vector<Matrix4f> WorldToModelMatrix(m_Scene->transforms.size());
        for (size_t i = 0; i < m_Scene->transforms.size(); ++i)
        {
            const GLTF::Transform& Transform = m_Scene->transforms[i];
            switch (Transform.Type)
            {
            case GLTF::Transform::Properties:
                {
                    WorldToModelMatrix[i] = Inverse(Transform.Value.asProperties.GetTransform());
                }
                break;

            case GLTF::Transform::Matrix:
                {
                    WorldToModelMatrix[i] = Inverse(Transform.Value.asMatrix);
                }
                break;

            SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported transform type")
            }
        }

        #pragma omp parallel
        {
            // TODO replace with a sobol sequence
            std::random_device hwseed;
            std::default_random_engine rng( hwseed() );
            std::uniform_real_distribution<float> uniform(0, 1);
            
            #pragma omp for collapse(2)
            for (int y = 0; y < Height; y++)
            for (int x = 0; x < Width; x++)
            {
                Vector4f Start = ProjToWorld * Vector4f(
                    ((static_cast<float>(x) + .5f) / static_cast<float>(Width)) * 2.f - 1.f,
                    -(((static_cast<float>(y) + .5f) / static_cast<float>(Height)) * 2.f - 1.f),
                    0.f, 1.f);
                Start.xyz() /= Start.w;
                Vector4f End = ProjToWorld * Vector4f(
                    ((static_cast<float>(x) + .5f) / static_cast<float>(Width)) * 2.f - 1.f,
                    -(((static_cast<float>(y) + .5f) / static_cast<float>(Height)) * 2.f - 1.f),
                    1.f, 1.f);
                End.xyz() /= End.w;
        
                Ray PrimaryRay = {
                    .origin = Start.xyz(), 
                    .direction = Normalize(End.xyz() - Start.xyz()), 
                    .distance = Magnitude(End.xyz() - Start.xyz())
                };
        
                // if (x == (Width / 2) && y == (Height / 2)) {EngineRuntimeBREAKPOINT}

                Hit ClosestHit = Hit();
                const Mesh* Mesh = nullptr;
                const Matrix4f* ModelMatrix = nullptr;
                size_t Material = 0;
        
                TraceRayTLAS PrimaryRayTracerTLAS(*m_CPUSceneTLAS, PrimaryRay);
        
                for (BVHHit TLASHit : PrimaryRayTracerTLAS)
                {
                    const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[TLASHit.NodeIndex].LeftIndex()];
            
                    TraceRayBLAS PrimaryRayTracerBLAS(*(Elt.BLAS), PrimaryRay, Elt.WorldToModel);
            
                    for (Hit Hit : PrimaryRayTracerBLAS)
                    {                    
                        // BLAS Any Hit   
                    }
            
                    if (Hit Closest = PrimaryRayTracerBLAS.ClosestHit(); Closest)
                    {
                        // BLAS Closest Hit
                        if (ClosestHit && ClosestHit.t < Closest.t) continue;

                        ClosestHit = Closest;
                        Mesh = Elt.BLAS->Meta.MeshRef;
                        Material = Elt.MaterialIndex;
                        ModelMatrix = &Elt.ModelToWorld;
                    }
                    else
                    {
                        // Miss
                    }
                }
        
                // Skip background
                if (!ClosestHit) continue;

                // Evaluate material
                const GLTF::Material& material = m_Scene->materials[Material];
                Vector3f Color{};
                switch (Mesh->GetMeshType())
                {
                case Mesh::POINTS:
                case Mesh::LINE_STRIP:
                case Mesh::LINE_LOOP:
                case Mesh::LINES:
                case Mesh::LINE_STRIP_ADJACENCY:
                case Mesh::LINES_ADJACENCY:
                case Mesh::PATCHES:
                case Mesh::QUAD_STRIP:
                case Mesh::QUADS:
                case Mesh::_Count:
                    SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported vertex type. Expected triangles")

                case Mesh::TRIANGLE_STRIP_ADJACENCY:
                case Mesh::TRIANGLES_ADJACENCY:
                case Mesh::TRIANGLE_STRIP:
                case Mesh::TRIANGLE_FAN:
                case Mesh::TRIANGLES:
                    Mesh::ConstFace face(*Mesh, ClosestHit.face);
                    Mesh::ConstVertex a = face[0];
                    Mesh::ConstVertex b = face[1];
                    Mesh::ConstVertex c = face[2];
            
                    Vector4f PositionH = ((*ModelMatrix) * Vector4f(VertexInterpolateTriangle(ClosestHit, a.Position(), b.Position(), c.Position()), 1.0f));
                    Point3f Position = PositionH.xyz() / PositionH.w;
                    Vector3f Normal = Normalize(((*ModelMatrix) * Vector4f(VertexInterpolateTriangle(ClosestHit, a.Normal(), b.Normal(), c.Normal()), 0.0f)).xyz());
                    Vector3f Tangent = Vector3f(0);
                    if (Mesh->HasTangents()) Normalize(((*ModelMatrix) * Vector4f(VertexInterpolateTriangle(ClosestHit, a.Tangent(), b.Tangent(), c.Tangent()), 0.0f)).xyz());
                    Vector2f UVs = Vector2f(0.5);
                    if (Mesh->HasTextureCoordinates()) VertexInterpolateTriangle(ClosestHit, a.TextureCoordinate(), b.TextureCoordinate(), c.TextureCoordinate());
            
                    // UVs in repeat mode 
                    // TODO rework image API with sampler
                    UVs.x = abs(fmod(UVs.x, 1.f));
                    UVs.y = abs(fmod(UVs.y, 1.f)); 
            
                    // Shadowmap test
                    float SunlightVisibility = 1.0f;
                    {
                        Ray SunLightRay = {
                            .origin = PrimaryRay.origin + PrimaryRay.direction * ClosestHit.t + m_LightDirection * 0.01f, 
                            .direction = m_LightDirection, 
                            .distance = 300.f
                        };
                        TraceRayTLAS SunlightRayTracerTLAS(*m_CPUSceneTLAS, SunLightRay);
                
                        bool HasHit = false;
                        for (BVHHit TLASHit : SunlightRayTracerTLAS)
                        {
                            const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[TLASHit.NodeIndex].LeftIndex()];
                    
                            TraceRayBLAS SunlightRayTracerBLAS(*(Elt.BLAS), SunLightRay, Elt.WorldToModel);
                    
                            for (Hit Hit : SunlightRayTracerBLAS)
                            {                    
                                // BLAS Any Hit   
                                HasHit = true;
                            }
                            if (HasHit) break;
                        }
                
                        SunlightVisibility = HasHit ? 0.f : 1.f;
                    }
            
                    Vector3f PixBaseColor = material.color.xyz();
                    float PixMetalness = material.metallic;
                    float PixRoughness = material.roughness;
                    float PixAmbiantOcclusion = 1.f;
            
                    if (material.colorTexture != UINT64_MAX)
                    {
                        ImageBuffer<Vector3t<uint8_t>> ColorTexBuffer(m_Scene->textures[material.colorTexture]);
                        PixBaseColor = ReadBuffer(
                            ColorTexBuffer,
                            static_cast<uint32_t>(UVs.x * static_cast<float>(ColorTexBuffer.Width())), 
                            static_cast<uint32_t>(UVs.y * static_cast<float>(ColorTexBuffer.Height()))
                            ) * PixBaseColor;
                    }
                    if (material.metallicRoughnessTexture != UINT64_MAX)
                    {
                        ImageBuffer<Vector3t<uint8_t>> MetallicRoughnessTexBuffer(m_Scene->textures[material.metallicRoughnessTexture]);
                        Vector3f mr = ReadBuffer(
                            MetallicRoughnessTexBuffer,
                            static_cast<uint32_t>(UVs.x * static_cast<float>(MetallicRoughnessTexBuffer.Width())), 
                            static_cast<uint32_t>(UVs.y * static_cast<float>(MetallicRoughnessTexBuffer.Height()))
                            );
                        PixMetalness = mr.z;
                        PixRoughness = mr.y;
                    }
                    if (material.occlusionTexture != UINT64_MAX)
                    {
                        ImageBuffer<Vector3t<uint8_t>> AOTexBuffer(m_Scene->textures[material.occlusionTexture]);
                        PixAmbiantOcclusion = ReadBuffer(
                            AOTexBuffer,
                            static_cast<uint32_t>(UVs.x * static_cast<float>(AOTexBuffer.Width())), 
                            static_cast<uint32_t>(UVs.y * static_cast<float>(AOTexBuffer.Height()))
                            ).x;
                    }
            
                    // Clamp roughness
                    PixRoughness = std::max(PixRoughness, 0.004f);
            
                    // Hit point Material settings
                    Vector3f DiffuseColor = LinearInterpolate(PixBaseColor, Vector3f(0), PixBaseColor);
                    Vector3f F0 = LinearInterpolate(Vector3f(0.04f), PixBaseColor, PixMetalness);
                    float Alpha = PixRoughness * PixRoughness;
            
                    Vector3f LocalNormal = Vector3f(0,0,1);
                    Vector3f FragBiTangent;
                    Matrix3f FragTBN;
                    if (Mesh->HasTangents())
                    {
                        FragBiTangent = Cross(Normal, Tangent); // always re-derive B from N×T
                        FragTBN = Matrix3f(Tangent, FragBiTangent, Normal);
                        FragTBN = Transpose(FragTBN);
                    }
                    else
                    {
                        Tangent = RotationX * Normal;
                        FragBiTangent = Cross(Normal, Tangent); // always re-derive B from N×T
                        FragTBN = Matrix3f(Tangent, FragBiTangent, Normal);
                        FragTBN = Transpose(FragTBN);
                    }
                    if (material.normalTexture != UINT64_MAX)
                    {
                        if (m_Scene->textures[material.normalTexture].ComponentCount() == 4 )
                        {
                            ImageBuffer<Vector4t<uint8_t>> NormalTexBuffer(m_Scene->textures[material.normalTexture]);
                            LocalNormal = ReadBuffer(
                                NormalTexBuffer,
                                static_cast<uint32_t>(UVs.x * static_cast<float>(NormalTexBuffer.Width())), 
                                static_cast<uint32_t>(UVs.y * static_cast<float>(NormalTexBuffer.Height()))
                                ).xyz();
                        }
                        else
                        {
                            ImageBuffer<Vector3t<uint8_t>> NormalTexBuffer(m_Scene->textures[material.normalTexture]);
                            LocalNormal = ReadBuffer(
                                NormalTexBuffer,
                                static_cast<uint32_t>(UVs.x * static_cast<float>(NormalTexBuffer.Width())), 
                                static_cast<uint32_t>(UVs.y * static_cast<float>(NormalTexBuffer.Height()))
                                );
                        }
                        LocalNormal = LocalNormal * 2.f - 1.f; // 0:1 normalized to normalized space (-1, 1)
                        Normal = Normalize(FragTBN * LocalNormal);
                    }

                    Vector3f finalColor = Vector3f(0);

                    // Direct lighting
                    {
                    Vector3f n = Normal;
                    Vector3f v = Normalize(m_RayTracingCamera.GetWorldPosition() - Position);
                    Vector3f l = Normalize(m_LightDirection);
                    Vector3f h = Normalize(v + l);
                
                    float CosThetaL = Dot(n, l);
                    float CosThetaV = Dot(n, v);

                    if(CosThetaV > 0 && CosThetaL > 0)
                    {
                        float VdotH = Dot(h, v);
                        Vector3f F = Rendering::FresnelSchlick(VdotH, F0);

                        float D = Rendering::D_GGX_Heitz2014_EQ71(h, n, Alpha);

                        float G = Rendering::G2_Heitz2014_EQ99(h, n, v, l, Alpha);

                        float DGNormalized = (D * G) / std::max(4.f * CosThetaL * CosThetaV , 0.0001f);

                        Vector3f ReflectanceDielectrical = Rendering::fDielectrical(DiffuseColor, DGNormalized, F);
                        Vector3f ReflectanceMetallic = Rendering::fMetallic(DGNormalized, F);

                        Vector3f Reflectance = LinearInterpolate(ReflectanceDielectrical, ReflectanceMetallic, PixMetalness);

                        Vector3f Light = m_LightColor * CosThetaL * m_LightIntensity * SunlightVisibility;

                        finalColor += Reflectance * Light;
                    }
                    }
            
                    // Indirect lighting
                    {
                        Vector3f LocalTangent = (RotationX * LocalNormal);
                        Vector3f LocalBiTangent = Cross(LocalNormal, LocalTangent);
                        Matrix3f LocalTBN = Matrix3f(LocalTangent, LocalBiTangent, LocalNormal);
                        LocalTBN = Transpose(LocalTBN);
                    
                    
                        Vector3f v = Normalize(m_RayTracingCamera.GetWorldPosition() - Position);
                        Matrix3f TBN =  FragTBN * LocalTBN;
                        Matrix3f InvTBN = Transpose(TBN);

                        Vector3f vNormalSpace = InvTBN * v;
                    
                        Vector3f sum = Vector3f(0);
                        for (size_t i = 0; i < m_IndirectSampleCount; ++i)
                        {
                            float u1 = uniform( rng );
                            float u2 = uniform( rng );
                            
                            Vector3f SampledFace = Rendering::SampleGGXVNDF_Intel2023(vNormalSpace, Alpha, Alpha, u1, u2);
                            Vector3f ne = Normalize(TBN * SampledFace);
                            Vector3f l = Reflect(v, ne);
                            Vector3f n = Normal;
                            Vector3f h = Normalize(v + l);
                            
                            float VdotH = Dot(h, v);
                            Vector3f F = Rendering::FresnelSchlick(VdotH, F0);
                            float G1 = Rendering::G1_Heitz2014_EQ98(h, n, v, Alpha);
                            float G2 = Rendering::G2_Heitz2014_EQ99(h, n, v, l, Alpha);
                            
                            Vector3f ReflectanceDielectrical = Rendering::fDielectricalIndirect(DiffuseColor, G2 / G1, F);
                            Vector3f ReflectanceMetallic = Rendering::fMetallicIndirect(DiffuseColor, G2 / G1, F);

                            Vector3f Reflectance = LinearInterpolate(ReflectanceDielectrical, ReflectanceMetallic, PixMetalness);
                            
                            Ray IndirectLightRay = {
                                .origin = PrimaryRay.origin + PrimaryRay.direction * ClosestHit.t + l * 0.01f, 
                                .direction = l, 
                                .distance = 50.f
                            };
                            TraceRayTLAS IndirectLightRayTracerTLAS(*m_CPUSceneTLAS, IndirectLightRay);
                    
                            Vector3f IndirectLight(0);
                            Hit ClosestIndirectHit{}; size_t IndirectHitMaterialId;
                            for (BVHHit TLASHit : IndirectLightRayTracerTLAS)
                            {
                                const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[TLASHit.NodeIndex].LeftIndex()];
                        
                                TraceRayBLAS IndirectLightRayTracerBLAS(*(Elt.BLAS), IndirectLightRay, Elt.WorldToModel);
                        
                                for (Hit Hit : IndirectLightRayTracerBLAS)
                                {                    
                                    // BLAS Any Hit   
                                }
                
                                if (Hit Closest = IndirectLightRayTracerBLAS.ClosestHit(); Closest)
                                {
                                    // BLAS Closest Hit
                                    ClosestIndirectHit = Closest;
                                    IndirectHitMaterialId = Elt.MaterialIndex;
                                }
                                else
                                {
                                    // Miss
                                }
                            }
                    
                            if (ClosestIndirectHit.IsValid())
                            {
                                if (m_Scene->materials[IndirectHitMaterialId].flags & GLTF::Material::Emissive)
                                {
                                    IndirectLight = m_Scene->materials[IndirectHitMaterialId].emissive.xyz();
                                }
                            }
                            else
                            {
                                IndirectLight = m_AmbientColor * m_AmbientIntensity;
                            }
                        
                            sum += (G1 > 0.0f && G2 > 0.0f) ? Reflectance * IndirectLight : Vector3f(0.0f);
                        }
                    
                        finalColor += sum / static_cast<float>(m_IndirectSampleCount);
                    }
            
                    Color = finalColor;
                }

                WriteBuffer(TargetImage, x, y, Color);
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
    int m_IndirectSampleCount = 128;

    bool m_DrawDebugRays;
    bool m_DrawDebugTLAS;
    bool m_DrawDebugBLASes;
    bool m_DrawDebugRayTraversal;
    bool m_DrawDebugRayTraversalIndirectLight;
    bool m_DrawDebugRayTraversalShadowMap;
    bool m_DebugFreezeRTCamera;
    Vector2t<int> m_DebugRayCoordinates;
    
    std::optional<GLTF::CPUScene> m_Scene;
    std::optional<Image> m_SkyboxHDRiCpu;
    std::optional<Image> m_WriteImage;
    
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
