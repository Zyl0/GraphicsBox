#version 450

// Vertex Shader
#include "Passes/ScreenTriangle.glsl"

#ifdef FRAGMENT_SHADER

#include "Include/Camera.glsl"

layout (binding = 0, std140) uniform CameraBuffer{
    CameraData Camera;
};

// Skylight method switch
#ifdef USE_PROCEDURAL_SKYLIGHT
#include "ProceduralSkylight.glsl"
#endif // USE_PROCEDURAL_SKYLIGHT
#ifdef USE_CUBEMAP_SKYLIGHT
#include "CubemapSkylight.glsl"
#endif // USE_CUBEMAP_SKYLIGHT
#ifdef USE_HDRI_SKYLIGHT
#include "HDRISkylight.glsl"
#endif // USE_HDRI_SKYLIGHT

layout(location = 0) in vec2 UV;
layout(location = 1) in vec2 UVProj;

out vec4 OutColor;

void main() 
{
    vec4 ViewportDirection = vec4(UVProj, 1.0, 0.0);
    ViewportDirection.x *= -1;
    ViewportDirection.z *= -1;
    vec4 Direction = ProjToWorld(Camera, ViewportToProj(Camera, ViewportDirection));

    OutColor.xyz = SampleSkylightColor(normalize(Direction.xyz));
    OutColor.w = 1.0;
}

#endif // FRAGMENT_SHADER