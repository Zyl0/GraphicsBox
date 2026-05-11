#version 450

// Vertex Shader
#include "Passes/ScreenTriangle.glsl"

#ifdef FRAGMENT_SHADER

#include "Include/ToneMapping.glsl"

layout(location = 0) in vec2 UV;
layout(location = 1) in vec2 UVProj;

uniform sampler2D GBufferAlbedo;
uniform sampler2D GBufferNormal;
uniform sampler2D GBufferProperties;
uniform sampler2D GBufferDepth;

out vec4 OutColor;

uniform sampler2D SceneRadiance;

void main()
{
    vec3 finalColor = texture(SceneRadiance, UV).xyz;

    // Apply bloom
    // TODO

    // Apply tonemapping curve
    // TODO

    // sRGB OETF encoding    
    OutColor.xyz = SRGB_OETF(finalColor, 2.2 /* PC (Or PC monitor) Gamma, TV is 2.4*/);
    OutColor.w = 1.;
}

#endif // FRAGMENT_SHADER