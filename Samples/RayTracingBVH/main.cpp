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

#include "Image/ImageOps.h"
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
        m_ZFar = 10.0f;
        m_LightColor = {1.0f, 0.97f, 0.9f};
        m_LightDirection = Normalize(Math::Vector3f(.1, .9, .2));
        m_LightIntensity = 3.0f;
        m_AmbientColor = {0.7f, 0.78f, 1.0f};
        m_AmbientIntensity = 1.2f;
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

        // Load scene data
        {
            std::filesystem::path relativePath = std::filesystem::path("Meshes") / "CornellBox-Original.glb";
            // std::filesystem::path relativePath = std::filesystem::path("glTF-Sample-Assets") / "Models" / "ABeautifulGame" / "glTF-Binary" /"ABeautifulGame.glb";
            std::filesystem::path path;
            // if (GetAbsoluteFilePath(std::filesystem::path("Meshes") / "CornellBox-Original.glb" ,path))
            if (GetAbsoluteFilePath(relativePath ,path))
            {
                m_Scene.emplace();
                AssertOrError( GLTF::LoadCPUScene(path, *m_Scene), "Failed to load scene")
                m_MeshObjects.resize(m_Scene->meshes.size());
                for (size_t i = 0; i < m_Scene->meshes.size(); i++)
                    m_MeshObjects[i].Data(m_Scene->meshes[i]);
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
                        m_CPUMeshesBLASs.emplace_back(BuildBLAS(mesh, group, 2));
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
        
        m_WriteImage.emplace(Width, Height, Image::UnsignedByte, Image::RGB, Image::Linear, nullptr);
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
            m_WriteImage.emplace(Width, Height, Image::UnsignedByte, Image::RGB, Image::Linear, nullptr);
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
            SetUniform(*m_SamplePipeline, "baseColor", Material.color.XYZ());
            // SetUniform(*m_SamplePipeline, "emissive", Material.emissive.XYZ());
            // SetUniform(*m_SamplePipeline, "roughness", Material.roughness);
            // SetUniform(*m_SamplePipeline, "metallic", Material.metallic);
            
            
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
            TraceRayTLAS DebugRayTracerTLAS(*m_CPUSceneTLAS, PrimaryRay);
            
            for (BVHHit TLASHit : DebugRayTracerTLAS)
            {
                const TLASElement& Elt = m_CPUSceneTLAS->Elements[m_CPUSceneTLAS->Tree[TLASHit.NodeIndex].LeftIndex()];
            
                BVHHit CurrentBVHHit{};
                uint32_t CurrentElementIndex{};
                std::stack<uint32_t> IterationStack;
                IterationStack.push(Elt.BLAS->Head);
                
                Ray LocalSpaceRay;
                Vector3f end =  PrimaryRay.origin + PrimaryRay.distance * PrimaryRay.direction;

                Vector4f t = Elt.WorldToModel * Vector4f(PrimaryRay.origin, 1.0f);
                LocalSpaceRay.origin = Vector3f(t.XYZ()) / t.w;

                t = Elt.WorldToModel * Vector4f(end, 1.0f);
                end = Vector3f(t.XYZ()) / t.w;

                LocalSpaceRay.distance = Magnitude(end - LocalSpaceRay.origin);
                LocalSpaceRay.direction = Normalize(end - LocalSpaceRay.origin);
                
            explore_bvh:
                if (!CurrentBVHHit)
                while (!IterationStack.empty())
                {
                    uint32_t NodeIndex = IterationStack.top();
                    IterationStack.pop();
                    
                    if (BoxHit hit = IntersectBox(Elt.BLAS->Tree[NodeIndex].Bounds, LocalSpaceRay); hit)
                    {
                        DebugRendering->DrawBox(WorldToViewportProj, Elt.BLAS->Tree[NodeIndex].Bounds, Vector3f(1.f,1.f,0.f), Elt.ModelToWorld );
                        if (Elt.BLAS->Tree[NodeIndex].IsNode())
                        {
                            IterationStack.push(Elt.BLAS->Tree[NodeIndex].LeftIndex());
                            IterationStack.push(Elt.BLAS->Tree[NodeIndex].RightIndex());
                        }
                        else // if (m_Blas->Tree[NodeIndex].IsLeaf())
                        {
                            CurrentBVHHit = BVHHit(NodeIndex, hit);
                            CurrentElementIndex = Elt.BLAS->Tree[NodeIndex].LeftIndex();
                            goto trace_leaf;
                        }
                    }
                }
                
            trace_leaf:
                if (CurrentBVHHit)
                {
                    uint32_t BLASFaceEnd = Elt.BLAS->Tree[CurrentBVHHit.NodeIndex].RightIndex();
                    Mesh::ConstFaces Faces(*(Elt.BLAS->Meta.MeshRef));
                    for (uint32_t ElementIndex = CurrentElementIndex; ElementIndex < BLASFaceEnd; ElementIndex++)
                    {
                        Mesh::ConstFace Face = Faces[Elt.BLAS->Elements[ElementIndex]];
                        
                        switch (Elt.BLAS->Meta.VertexType)
                        {            
                        case Mesh::TRIANGLE_STRIP_ADJACENCY:
                        case Mesh::TRIANGLES_ADJACENCY:
                        case Mesh::TRIANGLE_STRIP:
                        case Mesh::TRIANGLE_FAN:
                        case Mesh::TRIANGLES:
                            if (Hit hit = IntersectTriangle(Face, LocalSpaceRay); hit)
                            {
                                if (!ClosestHit || ClosestHit.t > hit.t)
                                {
                                    ClosestHit = hit;
                                }
                            }
                            break;
                        
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
                        SWITCH_ENUM_DEFAULT_AS_OUT_OF_RANGE("Unsupported face type for ray tracing")
                        }
                    }
                    
                    CurrentBVHHit = {};
                    CurrentElementIndex = std::numeric_limits<uint32_t>::max();
                    goto explore_bvh;
                }
            }
            
            if (ClosestHit)
            {
                DebugRendering->DrawRay(WorldToViewportProj, PrimaryRay, ClosestHit.t);
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
        
        if (m_DrawDebugRayTraversal)
        {
            ImGui::DragInt2("Debug Ray Coordinates", m_DebugRayCoordinates.data());
            m_DebugRayCoordinates.x = std::clamp(m_DebugRayCoordinates.x, 0, (int)Width - 1);
            m_DebugRayCoordinates.y = std::clamp(m_DebugRayCoordinates.y, 0, (int)Height - 1);
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
                .distance = m_ZFar - m_ZNear
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
                .distance = m_ZFar - m_ZNear
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
    
private:
    float m_FOV;
    float m_ZNear;
    float m_ZFar;
    Math::Vector3f m_LightColor;
    Math::Vector3f m_LightDirection;
    float m_LightIntensity;
    Math::Vector3f m_AmbientColor;
    float m_AmbientIntensity;

    bool m_DrawDebugRays;
    bool m_DrawDebugTLAS;
    bool m_DrawDebugBLASes;
    bool m_DrawDebugRayTraversal;
    bool m_DebugFreezeRTCamera;
    Vector2t<int> m_DebugRayCoordinates;
    
    std::optional<GLTF::CPUScene> m_Scene;
    std::optional<Image> m_SkyboxHDRiCpu;
    std::optional<Image> m_WriteImage;
    
    std::optional<Pipeline> m_SamplePipeline;
    std::optional<Texture2D> m_SkyboxHDRiGpu;
    std::vector<MeshObject> m_MeshObjects;
    
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
