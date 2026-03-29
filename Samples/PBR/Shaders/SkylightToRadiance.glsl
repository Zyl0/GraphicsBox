#version 450

// Vertex Shader
#include "Passes/ScreenTriangle.glsl"

#ifdef FRAGMENT_SHADER

#include "Camera.glsl"
#include "ProceduralSkylight.glsl"

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