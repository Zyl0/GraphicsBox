#include <random>

#include "RayTracers.h"
#include "RayTracersCommon.h"
#include "Modules/Rendering/Shaders/FresnelSchlick.h"
#include "Modules/Rendering/Shaders/GGX.h"
#include "Modules/Rendering/Shaders/LightinModel.h"
#include "Modules/Rendering/Tools/DebugModule.h"

using namespace Math;

void RayTracerBVH_RenderPrimaryRay(ImageBuffer<Math::Vector3f>& RenderTarget, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera)
{
    AssertOrErrorCallF(RayTracingParameters.AccelerationStructure != nullptr, return;, "RayTracerBVH requires a valid acceleration structure")
    AssertOrErrorCallF(RayTracingParameters.Scene != nullptr, return;, "RayTracerBVH requires a valid scene")
    
    ClearBuffer(RenderTarget);
    
    const uint32_t Width = RenderTarget.Width(); 
    const uint32_t Height = RenderTarget.Height();
    const Matrix4f ProjToWorld = Camera.InverseView() * Camera.InverseProjection();
    
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
            
            TraceRayTLAS PrimaryRayTracerTLAS(*RayTracingParameters.AccelerationStructure, PrimaryRay);
            for (BVHHit TLASHit : PrimaryRayTracerTLAS)
            {
                const TLASElement& Elt = *PrimaryRayTracerTLAS.Element();
                TraceRayBLAS PrimaryRayTracerBLAS(*(Elt.BLAS), PrimaryRay, PrimaryRayTracerTLAS.ClosestHit(), Elt.WorldToModel);
                
                for (Hit Hit : PrimaryRayTracerBLAS)
                {                    
                    // BLAS Any Hit   
                }
                
                PrimaryRayTracerTLAS.AcceptHit(PrimaryRayTracerBLAS.ClosestHit());
            }
            
            // Skip background
            Hit ClosestHit = PrimaryRayTracerTLAS.ClosestHit();

            if (!ClosestHit) continue;
            const Mesh* Mesh = PrimaryRayTracerTLAS.ClosestHitElement()->BLAS->Meta.MeshRef;
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
                Color.x = ClosestHit.u;
                Color.y = ClosestHit.v;
                Color.z = Saturate(1 - (ClosestHit.u + ClosestHit.v));
            }

            WriteBuffer(RenderTarget, x, y, Color);
        }
    }
}

struct ShadeContext
{
    Vector2t<uint32_t> Viewport;
    std::uniform_real_distribution<float>& Uniform;
    std::default_random_engine& RNG;
    const std::span<const std::pair<const TLASElement*, Box3f>> EmissiveElements;
};

static const Matrix3f RotationX = Matrix3f::RotationY(M_PI / 2.0);

static Vector4f ShadeHit(const TraceRayTLAS& CurrentHit, const RayTracerFrameParameters& RayTracingParameters, const ShadeContext& ShadingContext, uint32_t RemainingEvaluationCount)
{
    if (RemainingEvaluationCount == 0) return Vector4f(0,0,0,1);
    
    const RayShadingParameters& Shading = RayTracingParameters.Shading;
    const TLAS* AccelerationStructure = RayTracingParameters.AccelerationStructure;
    const GLTF::CPUScene* Scene = RayTracingParameters.Scene;
    
    // Hit mesh instance properties
    const Mesh* Mesh = CurrentHit.ClosestHitElement()->BLAS->Meta.MeshRef;
    const Matrix4f* ModelMatrix = &(CurrentHit.ClosestHitElement()->ModelToWorld);
    size_t MaterialID = CurrentHit.ClosestHitElement()->MaterialIndex;
    const GLTF::Material& Material = Scene->materials[MaterialID];
    
    // Hit surface properties
    Ray ShadeRay = CurrentHit.GetRay();
    Hit ClosestHit = CurrentHit.ClosestHit();
    SurfaceHit surface = HitInterpolateProperties(*Mesh, ClosestHit, ModelMatrix);
    SurfaceSampler surfaceSampler = CalcSurfaceSample(*Mesh, ShadeRay, ClosestHit, ShadingContext.Viewport, ModelMatrix);
    Vector3f HitPosition = ShadeRay.origin + ShadeRay.direction * ClosestHit.t;
    
    // Hit material properties
    Vector3f PixBaseColor = Material.color.xyz();
    Vector3f PixEmissive = Material.emissive.xyz();
    float PixMetalness = Material.metallic;
    float PixRoughness = Material.roughness;
    float PixAmbiantOcclusion = 1.f;
    if (Material.colorTexture != UINT64_MAX)
    {
        PixBaseColor = SampleImage(Scene->textures[Material.colorTexture], surfaceSampler, Shading.Sampler, surface.TextureCoordinates).xyz() * PixBaseColor;
    }
    if (Material.metallicRoughnessTexture != UINT64_MAX)
    {
        Vector4f mr = SampleImage(Scene->textures[Material.metallicRoughnessTexture], surfaceSampler, Shading.Sampler, surface.TextureCoordinates);
        PixMetalness = mr.z;
        PixRoughness = mr.y;
    }
    if (Material.occlusionTexture != UINT64_MAX)
    {
        PixAmbiantOcclusion = SampleImage(Scene->textures[Material.occlusionTexture], surfaceSampler, Shading.Sampler, surface.TextureCoordinates).x * PixAmbiantOcclusion;
    }
    if (Material.emissiveTexture != UINT64_MAX)
    {
        PixEmissive = SampleImage(Scene->textures[Material.emissiveTexture], surfaceSampler, Shading.Sampler, surface.TextureCoordinates).xyz();
    }
    
    // Normal mapping
    Vector3f LocalNormal = Vector3f(0,0,1);
    Vector3f FragBiTangent = Cross(surface.Normal, surface.Tangent); // always re-derive B from N×T
    Matrix3f FragTBN = Matrix3f(surface.Tangent, FragBiTangent, surface.Normal);
    if (Material.normalTexture != UINT64_MAX)
    {
        LocalNormal = SampleImage(Scene->textures[Material.normalTexture], surfaceSampler, Shading.Sampler, surface.TextureCoordinates).xyz();
        LocalNormal = LocalNormal * 2.f - 1.f; // 0:1 normalized to normalized space (-1, 1)
        surface.Normal = Normalize(FragTBN * LocalNormal);
    }
    
    // Microfacets properties
    Vector3f DiffuseColor = LinearInterpolate(PixBaseColor, Vector3f(0), PixBaseColor);
    Vector3f F0 = LinearInterpolate(Vector3f(0.04f), PixBaseColor, PixMetalness);
    float Alpha = PixRoughness * PixRoughness;
    if (Material.flags & GLTF::Material::UseSpecularGlossinessPBRExt) // Specular glossiness material
    {
        Vector3f PixSpecularColor = Material.specularColor.xyz();
                        
        if (Material.specularColorTexture == 1)
        {
            Vector4f SpecularGlossiness = SampleImage(Scene->textures[Material.specularColorTexture], surfaceSampler, Shading.Sampler, surface.TextureCoordinates);
            PixSpecularColor = SpecularGlossiness.xyz() * PixSpecularColor;
            PixRoughness = SpecularGlossiness.w;
        }
                        
        DiffuseColor = LinearInterpolate(PixBaseColor * (1 - std::max(PixSpecularColor.x, std::max(PixSpecularColor.y, PixSpecularColor.z))), Vector3f(0), PixMetalness);
        F0 = LinearInterpolate(Vector3f(0.04), PixSpecularColor, PixMetalness);
        PixRoughness = 1.f - PixRoughness;
        Alpha = PixRoughness * PixRoughness;
    }
    
    Vector3f finalColor = 0.f;
    
    // Emissive
    finalColor += PixEmissive;
    
    // Direct lighting
    {
        Vector3f n = surface.Normal;
        Vector3f v = -Normalize(ShadeRay.direction);
        Vector3f l = Normalize(Shading.SunLightDirection);
        Vector3f h = Normalize(v + l);
                
        float CosThetaL = Dot(n, l);
        float CosThetaV = Dot(n, v);

        if(CosThetaV > 0 && CosThetaL > 0)
        {
            // Shadowmap test
            float SunlightVisibility = 1.0f;
            if (RayTracingParameters.RenderSunlight)
            {
                Ray SunLightRay = {
                    .origin = ShadeRay.origin + ShadeRay.direction * ClosestHit.t + Shading.SunLightDirection * 0.01f, 
                    .direction = Shading.SunLightDirection, 
                    .distance = 300.f
                };
                
                uint32_t HitCount = 0;
                for (uint32_t SunSample = 0; SunSample < RayTracingParameters.SunlightSampleCount; SunSample++)
                {
                    // Halton version
                    // const float rx = Halton<2>(SunSample);
                    // const float ry = Halton<3>(SunSample);
                    
                    // Random version
                    float rx = ShadingContext.Uniform( ShadingContext.RNG ) * 0.5f;
                    float ry = ShadingContext.Uniform( ShadingContext.RNG ) * 0.5f;
                    
                    Vector3f right;
                    if (Dot(Shading.SunLightDirection, Vector3f(0,1,0)) < 1e-6f)
                    {
                        right = Vector3f(1,0,0);
                    }
                    else
                    {
                        right = Normalize(Cross(Shading.SunLightDirection,  Vector3f(0,1,0))); 
                    }
                    Vector3f up = Cross(right, Shading.SunLightDirection);
                    
                    QuaternionF qPitch = QuaternionF::FromAxisAngle(right, rx * Radians(Shading.SunLightSoftShadowHalfAngleDegrees * 2.f));
                    QuaternionF qYaw = QuaternionF::FromAxisAngle(up, ry * Radians(Shading.SunLightSoftShadowHalfAngleDegrees * 2.f));
                    
                    SunLightRay.direction = qPitch(qYaw(Shading.SunLightDirection));
                    
                    TraceRayTLAS SunlightRayTracerTLAS(*AccelerationStructure, SunLightRay);
    
                    bool HasHit = false;
                    for (BVHHit TLASHit : SunlightRayTracerTLAS)
                    {
                        const TLASElement& Elt = *SunlightRayTracerTLAS.Element();
        
                        TraceRayBLAS SunlightRayTracerBLAS(*(Elt.BLAS), SunLightRay, SunlightRayTracerTLAS.ClosestHit(), Elt.WorldToModel);
        
                        for (Hit Hit : SunlightRayTracerBLAS)
                        {                    
                            // BLAS Any Hit   
                            HasHit = true;
                        }
                        if (HasHit) break;
                    }
                    
                    if (HasHit) HitCount++;
                }
    
                SunlightVisibility = 1.f - ((float)(HitCount) / (float)(RayTracingParameters.SunlightSampleCount));
            }
            
            float VdotH = Dot(h, v);
            Vector3f F = Rendering::FresnelSchlick(VdotH, F0);

            float D = Rendering::D_GGX_Heitz2014_EQ71(h, n, Alpha);

            float G = Rendering::G2_Heitz2014_EQ99(h, n, v, l, Alpha);

            float DGNormalized = (D * G) / std::max(4.f * CosThetaL * CosThetaV , 0.0001f);

            Vector3f ReflectanceDielectrical = Rendering::fDielectrical(DiffuseColor, DGNormalized, F);
            Vector3f ReflectanceMetallic = Rendering::fMetallic(DGNormalized, F);

            Vector3f Reflectance = LinearInterpolate(ReflectanceDielectrical, ReflectanceMetallic, PixMetalness);

            Vector3f Light = Shading.SunLightColor * CosThetaL * Shading.SunLightIntensity * SunlightVisibility;

            finalColor += Reflectance * Light;
        }
    }
    
    // Emissive lighting
    if (RayTracingParameters.RenderEmissiveIndirectLights && RayTracingParameters.EmissiveSurfaceSampleCount > 0)
    {
        for (const auto& emissiveElement : ShadingContext.EmissiveElements)
        {
            size_t emissiveMatId = emissiveElement.first->MaterialIndex;
                    
            Vector3f n = surface.Normal;
            Vector3f v = -Normalize(ShadeRay.direction);
            Vector3f l = Normalize(emissiveElement.first->Center() - surface.Position);
            Vector3f h = Normalize(v + l);
                        
            float pqlen = Magnitude(emissiveElement.first->Center() - surface.Position);
                
            float CosThetaL = Dot(n, l);
            float CosThetaV = Dot(n, v);

            if(CosThetaV > 0 && CosThetaL > 0)
            {
                float Visibility = 0.f;
                for (uint32_t EmissiveSample = 0; EmissiveSample < RayTracingParameters.EmissiveSurfaceSampleCount; ++EmissiveSample)
                {
                    Vector3f samplePosition {};
                    samplePosition.x = LinearInterpolate(emissiveElement.second.a.x, emissiveElement.second.b.x,ShadingContext.Uniform( ShadingContext.RNG ));
                    samplePosition.y = LinearInterpolate(emissiveElement.second.a.y, emissiveElement.second.b.y,ShadingContext.Uniform( ShadingContext.RNG ));
                    samplePosition.z = LinearInterpolate(emissiveElement.second.a.z, emissiveElement.second.b.z,ShadingContext.Uniform( ShadingContext.RNG ));
                    
                    Vector3f origin = HitPosition;
                    Vector3f direction = Normalize(samplePosition - origin);
                    Ray EmissiveRay = {
                        .origin = origin + direction * 0.01f, 
                        .direction = direction, 
                        .distance = Magnitude(samplePosition - origin),
                    };
                    
                    TraceRayTLAS EmissiveLightTracerTLAS(*AccelerationStructure, EmissiveRay);
                    for (BVHHit bvhHit : EmissiveLightTracerTLAS) 
                    {
                        const TLASElement& Elt = *EmissiveLightTracerTLAS.Element();
                        if (Elt.MaterialIndex == emissiveMatId) continue;
                        
                        bool HasHit = false;
                        TraceRayBLAS EmissiveLightTracerBLAS(*(Elt.BLAS), EmissiveRay, EmissiveLightTracerTLAS.ClosestHit(), Elt.WorldToModel);
                        for (Hit Hit : EmissiveLightTracerBLAS)
                        {                    
                            // BLAS Any Hit   
                            HasHit = true;
                        }
                        if (HasHit) 
                        {
                            Visibility += 1.f;
                            break;
                        }
                    }
                }
                Visibility /= (float)(RayTracingParameters.EmissiveSurfaceSampleCount);
            
                float VdotH = Dot(h, v);
                Vector3f F = Rendering::FresnelSchlick(VdotH, F0);

                float D = Rendering::D_GGX_Heitz2014_EQ71(h, n, Alpha);

                float G = Rendering::G2_Heitz2014_EQ99(h, n, v, l, Alpha);

                float DGNormalized = (D * G) / std::max(4.f * CosThetaL * CosThetaV , 0.0001f);

                Vector3f ReflectanceDielectrical = Rendering::fDielectrical(DiffuseColor, DGNormalized, F);
                Vector3f ReflectanceMetallic = Rendering::fMetallic(DGNormalized, F);

                Vector3f Reflectance = LinearInterpolate(ReflectanceDielectrical, ReflectanceMetallic, PixMetalness);

                Vector3f Light = Scene->materials[emissiveElement.first->MaterialIndex].emissive.xyz() * CosThetaL * Visibility * 1.f / (pqlen * pqlen);

                finalColor += Reflectance * Light;
            }
        }
    }
    
    // Indirect lighting
    if (RayTracingParameters.RenderIndirectLight && RayTracingParameters.IndirectSampleCount > 0)
    {
        Vector3f LocalTangent = (RotationX * LocalNormal);
        Vector3f LocalBiTangent = Cross(LocalNormal, LocalTangent);
        Matrix3f LocalTBN = Matrix3f(LocalTangent, LocalBiTangent, LocalNormal);
        LocalTBN = Transpose(LocalTBN);


        Vector3f v = -Normalize(ShadeRay.direction);
        Matrix3f TBN =  FragTBN * LocalTBN;
        Matrix3f InvTBN = Transpose(TBN);

        Vector3f vNormalSpace = InvTBN * v;

        Vector3f sum = Vector3f(0);
        for (size_t i = 0; i < RayTracingParameters.IndirectSampleCount; ++i)
        {
            float u1 = ShadingContext.Uniform( ShadingContext.RNG );
            float u2 = ShadingContext.Uniform( ShadingContext.RNG );
        
            Vector3f SampledFace = Rendering::SampleGGX(Vector3f(0, 0, 1), Alpha, Alpha, u1, u2);
            Vector3f ne = Normalize(TBN * SampledFace);
            Vector3f l = Reflect(-v, ne);
            Vector3f n = surface.Normal;
            Vector3f h = Normalize(v + l);
        
            float VdotH = Dot(h, v);
            Vector3f F = Rendering::FresnelSchlick(VdotH, F0);
            float G1 = Rendering::G1_Heitz2014_EQ98(h, n, v, Alpha);
            float G2 = Rendering::G2_Heitz2014_EQ99(h, n, v, l, Alpha);
        
            Vector3f ReflectanceDielectrical = Rendering::fDielectricalIndirect(DiffuseColor, G2 / G1, F);
            Vector3f ReflectanceMetallic = Rendering::fMetallicIndirect(DiffuseColor, G2 / G1, F);

            Vector3f Reflectance = LinearInterpolate(ReflectanceDielectrical, ReflectanceMetallic, PixMetalness);
        
            Ray IndirectLightRay = {
                .origin = HitPosition + l * 0.01f, 
                .direction = l, 
                .distance = 50.f
            };

            TraceRayTLAS IndirectLightRayTracerTLAS(*AccelerationStructure, IndirectLightRay);
            for (BVHHit TLASHit : IndirectLightRayTracerTLAS)
            {
                const TLASElement& Elt = *(IndirectLightRayTracerTLAS.Element());
    
                TraceRayBLAS IndirectLightRayTracerBLAS(*(Elt.BLAS), IndirectLightRay, IndirectLightRayTracerTLAS.ClosestHit(), Elt.WorldToModel);
                for (Hit Hit : IndirectLightRayTracerBLAS)
                {                    
                    // BLAS Any Hit   
                }
                
                IndirectLightRayTracerTLAS.AcceptHit(IndirectLightRayTracerBLAS.ClosestHit());
            }

            Vector3f IndirectLight(0);
            if (IndirectLightRayTracerTLAS.ClosestHit())
            {
                if (Scene->materials[IndirectLightRayTracerTLAS.ClosestHitElement()->MaterialIndex].flags & GLTF::Material::Emissive)
                {
                    IndirectLight += Scene->materials[IndirectLightRayTracerTLAS.ClosestHitElement()->MaterialIndex].emissive.xyz();
                }
                
                IndirectLight += ShadeHit(IndirectLightRayTracerTLAS, RayTracingParameters, ShadingContext, RemainingEvaluationCount - 1).xyz();
            }
            else
            {
                IndirectLight += Shading.AmbientColor * Shading.AmbientIntensity;
            }
    
            sum += (G1 > 0.0f && G2 > 0.0f) ? Reflectance * IndirectLight : Vector3f(0.0f);
        }

        finalColor += (sum * PixAmbiantOcclusion) / static_cast<float>(RayTracingParameters.IndirectSampleCount);
    }
                
    // Transparent
    {
        // TODO transparent shading
    }
    
    return Vector4f(finalColor, 1.0f);
}

void RayTracerBVH_RenderShaded(ImageBuffer<Math::Vector3f>& RenderTarget, ImageBuffer<float>& Depth, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera)
{
    AssertOrErrorCallF(RayTracingParameters.AccelerationStructure != nullptr, return;, "RayTracerBVH requires a valid acceleration structure")
    AssertOrErrorCallF(RayTracingParameters.Scene != nullptr, return;, "RayTracerBVH requires a valid scene")
    
    const RayShadingParameters& Shading = RayTracingParameters.Shading;
    const TLAS* AccelerationStructure = RayTracingParameters.AccelerationStructure;
    const GLTF::CPUScene* Scene = RayTracingParameters.Scene;
    
    // Collect emissive elements
    std::vector<std::pair<const TLASElement*, Box3f>> emissiveElements;
    for (size_t i = 0; i < AccelerationStructure->Elements.size(); i++)
    {
        if (Scene->materials[AccelerationStructure->Elements[i].MaterialIndex].flags & GLTF::Material::Emissive)
        {
            emissiveElements.emplace_back(&(AccelerationStructure->Elements[i]), TLASElementGetBounds(AccelerationStructure->Elements[i], AccelerationStructure->Meta));
        }
    }
    
    ClearBuffer(RenderTarget, Shading.AmbientColor * Shading.AmbientIntensity);
    
    const uint32_t Width = RenderTarget.Width(); 
    const uint32_t Height = RenderTarget.Height();
    const Matrix4f ProjToWorld = Camera.InverseView() * Camera.InverseProjection();
    
#pragma omp parallel
    {
        std::random_device hwseed;
        std::default_random_engine rng( hwseed() );
        std::uniform_real_distribution<float> uniform(0, 1);
        
        ShadeContext shadingCtx = {RenderTarget.Size() * 2u, uniform, rng, emissiveElements};
        
#pragma omp for collapse(2) schedule(dynamic)
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
            
            TraceRayTLAS PrimaryRayTracerTLAS(*RayTracingParameters.AccelerationStructure, PrimaryRay);
            for (BVHHit TLASHit : PrimaryRayTracerTLAS)
            {
                const TLASElement& Elt = *PrimaryRayTracerTLAS.Element();
                TraceRayBLAS PrimaryRayTracerBLAS(*(Elt.BLAS), PrimaryRay, PrimaryRayTracerTLAS.ClosestHit(), Elt.WorldToModel);
                
                for (Hit Hit : PrimaryRayTracerBLAS)
                {                    
                    // BLAS Any Hit   
                }
                
                PrimaryRayTracerTLAS.AcceptHit(PrimaryRayTracerBLAS.ClosestHit());
            }
        
            // Skip background
            if (!PrimaryRayTracerTLAS.ClosestHit()) continue;

            WriteBuffer(RenderTarget, x, y, ShadeHit(PrimaryRayTracerTLAS, RayTracingParameters, shadingCtx, RayTracingParameters.IndirectSampleBounceCount).xyz());
        }
    }
}

void RayTracerBVH_DrawTraversal(const RayTracerFrameParameters& RayTracingParameters, const TraversalDebuggerParameters& TraversalDebuggerParameters, const Camera& Camera, Math::Vector2f ProjectiveSpaceCoordinates)
{
    AssertOrErrorCallF(RayTracingParameters.AccelerationStructure != nullptr, return;, "RayTracerBVH requires a valid acceleration structure")
    AssertOrErrorCallF(RayTracingParameters.Scene != nullptr, return;, "RayTracerBVH requires a valid scene")
    AssertOrErrorCallF(TraversalDebuggerParameters.EngineContext != nullptr, return;, "RayTracerBVH debugger requires a valid engine context")
    
    const RayShadingParameters& Shading = RayTracingParameters.Shading;
    const TLAS* AccelerationStructure = RayTracingParameters.AccelerationStructure;
    const GLTF::CPUScene* Scene = RayTracingParameters.Scene;
    Rendering::Debug::Module* DebugRendering = Engine::GetModule<Rendering::Debug::Module>(*TraversalDebuggerParameters.EngineContext);
    
    const Matrix4f ProjToWorld = Camera.InverseView() * Camera.InverseProjection();
    const Matrix4f WorldToProj = Camera.Projection() * Camera.View();
    
    Vector3f Start = CalcPrimaryRayPoint(ProjToWorld, ProjectiveSpaceCoordinates, 0.f, 0);
    Vector3f End = CalcPrimaryRayPoint(ProjToWorld, ProjectiveSpaceCoordinates, 1.f, 0);
    
    Ray PrimaryRay = {
        .origin = Start.xyz(), 
        .direction = Normalize(End.xyz() - Start.xyz()), 
        .distance = Magnitude(End - Start)
    };
    
    TraceRayTLAS DebugRayTracerTLAS(*AccelerationStructure, PrimaryRay);    
    for (BVHHit TLASHit : DebugRayTracerTLAS)
    {
        const TLASElement& Elt = *DebugRayTracerTLAS.Element();
        
        TraceRayBLAS DebugRayTracerBLAS(*(Elt.BLAS), PrimaryRay, DebugRayTracerTLAS.ClosestHit(), Elt.WorldToModel);
        for (Hit Hit : DebugRayTracerBLAS)
        {                    
            // BLAS Any Hit   
        }
        
        DebugRayTracerTLAS.AcceptHit(DebugRayTracerBLAS.ClosestHit());
    }

    if (Hit ClosestHit = DebugRayTracerTLAS.ClosestHit())
    {
        const Mesh* Mesh = DebugRayTracerTLAS.ClosestHitElement()->BLAS->Meta.MeshRef;
        const Matrix4f* ModelMatrix = &(DebugRayTracerTLAS.ClosestHitElement()->ModelToWorld);
        size_t Material = DebugRayTracerTLAS.ClosestHitElement()->MaterialIndex;
        
        static const Matrix3f RotationX = Matrix3f::RotationY(M_PI / 2.0);
        
        DebugRendering->DrawRay(WorldToProj, PrimaryRay, ClosestHit.t);

        SurfaceHit surface = HitInterpolateProperties(*Mesh, ClosestHit, ModelMatrix);
        
        const GLTF::Material& material = Scene->materials[Material];
        
        float PixRoughness = material.roughness;
        float Alpha = PixRoughness * PixRoughness;
        
        if (TraversalDebuggerParameters.DebugSunlight)
        {
            Ray SunLightRay = {
                .origin = PrimaryRay.origin + PrimaryRay.direction * ClosestHit.t + Shading.SunLightDirection * 0.01f, 
                .direction = Shading.SunLightDirection, 
                .distance = 300.f
            };
            
            for (size_t SunSample = 0; SunSample < RayTracingParameters.SunlightSampleCount; SunSample++)
            {
                const float rx = Halton<2>(SunSample);
                const float ry = Halton<3>(SunSample);
                    
                Vector3f right;
                if (Dot(Shading.SunLightDirection, Vector3f(0,1,0)) < 1e-6f)
                {
                    right = Vector3f(1,0,0);
                }
                else
                {
                    right = Normalize(Cross(Shading.SunLightDirection,  Vector3f(0,1,0))); 
                }
                Vector3f up = Cross(right, Shading.SunLightDirection);
                    
                QuaternionF qPitch = QuaternionF::FromAxisAngle(right, rx * Radians(Shading.SunLightSoftShadowHalfAngleDegrees * 2.f));
                QuaternionF qYaw = QuaternionF::FromAxisAngle(up, ry * Radians(Shading.SunLightSoftShadowHalfAngleDegrees * 2.f));
                    
                SunLightRay.direction = qPitch(qYaw(Shading.SunLightDirection));
                
                TraceRayTLAS SunlightRayTracerTLAS(*AccelerationStructure, SunLightRay);
                bool HasHit = false;
                float t;
                for (BVHHit TLASHit : SunlightRayTracerTLAS)
                {
                    const TLASElement& Elt = *SunlightRayTracerTLAS.Element(); 
            
                    TraceRayBLAS SunlightRayTracerBLAS(*(Elt.BLAS), SunLightRay, {}, Elt.WorldToModel);
                    for (Hit Hit : SunlightRayTracerBLAS)
                    {                    
                        // BLAS Any Hit   
                        HasHit = true;
                        t = Hit.t;
                        break;
                    }
                    if (HasHit) break;
                }
                if (HasHit)
                {
                    DebugRendering->DrawRay(WorldToProj, SunLightRay, PrimaryRay.distance);
                }
                else
                {
                    DebugRendering->DrawRay(WorldToProj, SunLightRay, PrimaryRay.distance);
                }
            }
        }
        
        if (TraversalDebuggerParameters.DebugIndirectLight)
        {
            surface.Tangent = Normalize(surface.Tangent - Dot(surface.Tangent, surface.Normal) * surface.Normal);
            Vector3f FragBiTangent = Cross(surface.Normal, surface.Tangent); // always re-derive B from N×T
            Matrix3f FragTBN = Matrix3f( surface.Tangent, FragBiTangent, surface.Normal);
            // FragTBN = Transpose(FragTBN);
            Matrix3f InvTBN = Transpose(FragTBN);
            
            Vector3f v = Normalize(Camera.GetWorldPosition() - surface.Position);

            Vector3f vNormalSpace = InvTBN * v;
            
            std::random_device hwseed;
            std::default_random_engine rng( TraversalDebuggerParameters.FreezeRandomSequence ? 0 : hwseed() );
            std::uniform_real_distribution<float> uniform(0, 1);
            
            for (size_t i = 0; i < RayTracingParameters.IndirectSampleCount; ++i)
            {
                float u1 = uniform( rng );
                float u2 = uniform( rng );
                    
                Vector3f SampledFace = Rendering::SampleGGX(Vector3f(0, 0, 1), Alpha, Alpha, u1, u2);
                Vector3f ne = Normalize(FragTBN * SampledFace);
                Vector3f l = Reflect(-v, ne);
                
                DebugRendering->DrawLine(
                    WorldToProj,
                    PrimaryRay.origin + PrimaryRay.direction * ClosestHit.t,
                    PrimaryRay.origin + PrimaryRay.direction * ClosestHit.t + ne * 0.2f,
                    Vector3f(0.f, 1.f, 0.f)                            
                );
                
                Ray IndirectLightRay = {
                    .origin = PrimaryRay.origin + PrimaryRay.direction * ClosestHit.t + l * 0.01f, 
                    .direction = l, 
                    .distance = 50.f
                };
                
                TraceRayTLAS IndirectLightRayTracerTLAS(*AccelerationStructure, IndirectLightRay);
                for (BVHHit TLASHit : IndirectLightRayTracerTLAS)
                {
                    const TLASElement& Elt = *IndirectLightRayTracerTLAS.Element();
                    TraceRayBLAS IndirectLightRayTracerBLAS(*(Elt.BLAS), IndirectLightRay, IndirectLightRayTracerTLAS.ClosestHit(), Elt.WorldToModel);
                
                    for (Hit Hit : IndirectLightRayTracerBLAS)
                    {                    
                        // BLAS Any Hit   
                    }

                    IndirectLightRayTracerTLAS.AcceptHit(IndirectLightRayTracerBLAS.ClosestHit());
                }
                
                if (Hit ClosestIndirectHit = IndirectLightRayTracerTLAS.ClosestHit())
                {
                    DebugRendering->DrawRay(WorldToProj, IndirectLightRay, ClosestIndirectHit.t);
                }
                else
                {
                    DebugRendering->DrawRay(WorldToProj, IndirectLightRay, IndirectLightRay.distance);
                }
            }
        }
    }
    else
    {
        DebugRendering->DrawRay(WorldToProj, PrimaryRay, PrimaryRay.distance);
    }
}
