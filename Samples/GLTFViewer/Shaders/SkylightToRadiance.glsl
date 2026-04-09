#version 450

// Vertex Shader
#include "Passes/ScreenTriangle.glsl"

#ifdef FRAGMENT_SHADER

#include "Include/Camera.glsl"

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
    vec4 Direction = ProjToWorld(ViewportToProj(vec4(UVProj, 1.0, 0.0)));

    OutColor.xyz = SampleSkylightColor(normalize(Direction.xyz));
    OutColor.w = 1.0;
}

#endif // FRAGMENT_SHADER