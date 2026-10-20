#pragma once
#include "Camera/Camera.h"
#include "Image/ImageOps.h"
#include "Importers/GLTF/SceneLoader.h"
#include "RayTracing/RayTrace.h"

struct TraversalDebuggerParameters
{
    
};

struct RayShadingParameters
{
    Math::Vector3f SunLightColor = {1.0f, 0.97f, 0.9f};
    Math::Vector3f SunLightDirection = Normalize(Math::Vector3f(.1, .9, .2));
    float SunLightIntensity = 3.0f;
    float SunLightSoftShadowHalfAngleDegrees = 1.7f;
    
    Math::Vector3f AmbientColor = {0.7f, 0.78f, 1.0f};
    float AmbientIntensity = 1.9f;
    
    ImageSampler Sampler = {
        .BaseFilter = ImageSampler::F_Linear,
        .Magnification = ImageSampler::F_Linear,
        .Minification = ImageSampler::F_Linear,
        .MipMode = ImageSampler::F_Linear
    };
};

struct RayTracerFrameParameters
{
    GLTF::CPUScene* Scene = nullptr;
    const TLAS* AccelerationStructure = nullptr;
    std::span<const BLAS> BLASes = {};
    RayShadingParameters Shading = {};
    
    bool RenderSunlight = true;
    int SunlightSampleCount = 4;
    
    bool RenderIndirectLight = true;
    int IndirectSampleCount = 128;
    int IndirectSampleBounceCount = 1;
    
    bool RenderEmissiveIndirectLights = true;
    int EmissiveSurfaceSampleCount = 1;
};

void RayTracerDummy_RenderPrimaryRay(ImageBuffer<Math::Vector3f>& RenderTarget, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void RayTracerDummy_RenderShaded(ImageBuffer<Math::Vector3f>& RenderTarget, ImageBuffer<float>& Depth, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void RayTracerDummy_DrawTraversal(const RayTracerFrameParameters& RayTracingParameters, const TraversalDebuggerParameters& TraversalDebuggerParameters, const Camera& Camera, Math::Vector2f ProjectiveSpaceCoordinates);

void RayTracerSIMD_RenderPrimaryRay(ImageBuffer<Math::Vector3f>& RenderTarget, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void RayTracerSIMD_RenderShaded(ImageBuffer<Math::Vector3f>& RenderTarget, ImageBuffer<float>& Depth, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void RayTracerSIMD_DrawTraversal(const RayTracerFrameParameters& RayTracingParameters, const TraversalDebuggerParameters& TraversalDebuggerParameters, const Camera& Camera, Math::Vector2f ProjectiveSpaceCoordinates);

void RayTracerBVH_RenderPrimaryRay(ImageBuffer<Math::Vector3f>& RenderTarget, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void RayTracerBVH_RenderShaded(ImageBuffer<Math::Vector3f>& RenderTarget, ImageBuffer<float>& Depth, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void RayTracerBVH_DrawTraversal(const RayTracerFrameParameters& RayTracingParameters, const TraversalDebuggerParameters& TraversalDebuggerParameters, const Camera& Camera, Math::Vector2f ProjectiveSpaceCoordinates);

void RayTracerBVHSIMD_RenderPrimaryRay(ImageBuffer<Math::Vector3f>& RenderTarget, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void RayTracerBVHSIMD_RenderShaded(ImageBuffer<Math::Vector3f>& RenderTarget, ImageBuffer<float>& Depth, const RayTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void RayTracerBVHSIMD_DrawTraversal(const RayTracerFrameParameters& RayTracingParameters, const TraversalDebuggerParameters& TraversalDebuggerParameters, const Camera& Camera, Math::Vector2f ProjectiveSpaceCoordinates);

struct PathTracerFrameParameters
{
    GLTF::CPUScene* Scene = nullptr;
    TLAS* AccelerationStructure = nullptr;
    std::span<const BLAS> BLASes = {};
    RayShadingParameters Shading = {};
};

void PathTracerDummy_RenderPrimaryRay(ImageBuffer<Math::Vector3f>& RenderTarget, const PathTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void PathTracerDummy_RenderShaded(ImageBuffer<Math::Vector3f>& RenderTarget, ImageBuffer<float>& Depth, const PathTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void PathTracerDummy_DrawTraversal(const PathTracerFrameParameters& RayTracingParameters, const TraversalDebuggerParameters& TraversalDebuggerParameters, const Camera& Camera, Math::Vector2f ProjectiveSpaceCoordinates);

void PathTracerBVH_RenderPrimaryRay(ImageBuffer<Math::Vector3f>& RenderTarget, const PathTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void PathTracerBVH_RenderShaded(ImageBuffer<Math::Vector3f>& RenderTarget, ImageBuffer<float>& Depth, const PathTracerFrameParameters& RayTracingParameters, const Camera& Camera);
void PathTracerBVH_DrawTraversal(const PathTracerFrameParameters& RayTracingParameters, const TraversalDebuggerParameters& TraversalDebuggerParameters, const Camera& Camera, Math::Vector2f ProjectiveSpaceCoordinates);