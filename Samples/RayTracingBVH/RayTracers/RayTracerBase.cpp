#include "RayTracers.h"

#include <random>

#include "RayTracersCommon.h"
#include "Modules/Rendering/Shaders/FresnelSchlick.h"
#include "Modules/Rendering/Shaders/GGX.h"
#include "Modules/Rendering/Shaders/LightinModel.h"
#include "Modules/Rendering/Tools/DebugModule.h"

using namespace Math;

void RayTracerDummy_RenderPrimaryRay(ImageBuffer<Math::Vector3f>& RenderTarget, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera)
{
    AssertOrErrorCallF(RayTracingParameters.Scene != nullptr, return;, "RayTracerBVH requires a valid scene")
        
    ClearBuffer(RenderTarget);
    
    const uint32_t Width = RenderTarget.Width(); 
    const uint32_t Height = RenderTarget.Height();
    const Matrix4f ProjToWorld = Camera.InverseView() * Camera.InverseProjection();
    
    const GLTF::CPUScene* Scene = RayTracingParameters.Scene;
    
    std::vector<Matrix4f> WorldToModelMatrix(Scene->transforms.size());
    for (size_t i = 0; i < Scene->transforms.size(); ++i)
    {
        const GLTF::Transform& Transform = Scene->transforms[i];
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
#pragma omp for collapse(2)
        for (int y = 0; y < Height; y++)
        for (int x = 0; x < Width; x++)
        {
            Vector2f RayProjSpaceCoordinates = CalcProjSpaceCoords(x, y, Width, Height);
            Vector3f Start = CalcPrimaryRayPoint(ProjToWorld, RayProjSpaceCoordinates, 0.f, 0);
            Vector3f End = CalcPrimaryRayPoint(ProjToWorld, RayProjSpaceCoordinates, 1.f, 0);
        
            Ray PrimaryRay = {
                .origin = Start, 
                .direction = Normalize(End - Start), 
                .distance = Magnitude(End - Start)
            };
            
            Hit ClosestHit = Hit();
            GLTF::MeshInstance ClosestHitInstance;
            
            // Traversal without BVH
            for (const auto & instance : Scene->instances)
            {
                const Mesh& mesh = Scene->meshes[instance.mesh];
                const GLTF::Material& material = Scene->materials[instance.material];
                const Mesh::VertexGroup& group = mesh.GetVertexGroups()[instance.vertexGroup];
                const Matrix4f& transform = WorldToModelMatrix[instance.transform];
                
                Ray LocalRay = transform * PrimaryRay;
                if (BoxHit hit = IntersectBox(Box3f(group.BoundsMin, group.BoundsMax), LocalRay); !hit)
                {
                    continue;
                }

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
            const Mesh& mesh = Scene->meshes[ClosestHitInstance.mesh];
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
                Color.x = ClosestHit.u;
                Color.y = ClosestHit.v;
                Color.z = Saturate(1 - (ClosestHit.u + ClosestHit.v));
            }

            WriteBuffer(RenderTarget, x, y, Color);
        }
    }
}

void RayTracerDummy_RenderShaded(ImageBuffer<Math::Vector3f>& RenderTarget, ImageBuffer<float>& Depth, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera)
{
    
}

void RayTracerDummy_DrawTraversal(const RayTracerFrameParameters& RayTracingParameters, const TraversalDebuggerParameters& TraversalDebuggerParameters, const Camera& Camera, Math::Vector2f ProjectiveSpaceCoordinates)
{
    
}