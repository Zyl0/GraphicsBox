#version 450

// Vertex Shader
#include "Passes/ScreenTriangle.glsl"

#ifdef FRAGMENT_SHADER

layout(location = 0) in vec2 UV;
layout(location = 1) in vec2 UVProj;

out vec4 OutColor;

#ifdef RESAMPLE_PASS
uniform sampler2D InputTex2D;
uniform sampler3D InputTex3D;
uniform uint InputTexType;
uniform uvec2 InputSize;
uniform float LodBias;
uniform float DepthBias;

uniform uvec2 OutputSize;

uniform vec2 RangeRed;
uniform vec2 RangeGreen;
uniform vec2 RangeBlue;
uniform vec2 RangeAlpha;

uniform uint TagOutOfBounds;

void main()
{
    // Perform zoom
    // TODO
    
    vec4 finalColor = vec4(0,0,0,1);
    
    // Sample texture
    if (InputTexType == /* Texture 2D */ 0)
    {
        finalColor = texture(InputTex2D, UV, LodBias);
    }
    if (InputTexType == /* Texture 3D */ 2)
    {
        finalColor = texture(InputTex3D, vec3(UV, DepthBias), LodBias);
    }

    // Handle out of bounds
    if (TagOutOfBounds > 0)
    {
        bool UnderRange = bool(finalColor.x < RangeRed.x) || bool(finalColor.y < RangeGreen.x) || bool(finalColor.z < RangeBlue.x);
        bool OverRange = bool(finalColor.x > RangeRed.y) || bool(finalColor.y > RangeGreen.y) || bool(finalColor.z > RangeBlue.y);
        
        if (TagOutOfBounds == /*Alpha modifier*/ 1)
        {
            finalColor.w *= UnderRange || OverRange ? 0.0f : 1.0;
        }
        if (TagOutOfBounds == /*Blue Green modifier*/ 2)
        {
            finalColor.xyz = UnderRange ? vec3(0,0,1) : OverRange ? vec3(0,1,0) : finalColor.xyz;
        }
    }
    
    // Ranges adjustments
    finalColor.x = (finalColor.x - RangeRed.x) / (finalColor.x - RangeRed.y);
    finalColor.y = (finalColor.y - RangeGreen.x) / (finalColor.y - RangeGreen.y);
    finalColor.z = (finalColor.z - RangeBlue.x) / (finalColor.z - RangeBlue.y);
    finalColor.w = (finalColor.w - RangeAlpha.x) / (finalColor.w - RangeAlpha.y);
    
    OutColor = finalColor;
}
#endif // RESAMPLE_PASS

#ifdef BACKGROUND_GRID_PASS
uniform uint GridSize;

void main()
{
    // uvec2 PixelPosition = uvec2(UV * vec2(OutputSize));
    
    vec2 p = gl_FragCoord.xy;
    uvec2 cell = uvec2(floor(p / float(GridSize)));
    
    float checker = float((cell.x + cell.y) % 2);
    
    OutColor.xyz = mix(vec3(0.2), vec3(0.8), checker);
    OutColor.a = 1.0f;
}
#endif // BACKGROUND_GRID_PASS

#ifdef DISPLAY_PASS
#include "Include/ColorSpaces.glsl"
#include "Include/ToneMapping.glsl"

uniform sampler2D Input;

float InverseLerp(float A, float B, float T)
{
    return clamp((T - A) / (B - A), 0.0f, 1.0f);
}

vec3 InverseLerp(vec3 A, vec3 B, float T)
{
    return clamp((T - A) / (B - A), 0.0f, 1.0f);
}


// Channels
uniform bool ConvertToGreyscale;
uniform bool ShowRed;
uniform bool ShowGreen;
uniform bool ShowBlue;
uniform bool ShowAlpha;

uniform vec2 RedRange;
uniform vec2 GreenRange;
uniform vec2 BlueRange;
uniform vec2 AlphaRange;

uniform bool UseOETF;

void main()
{
    vec4 finalColor = texture(Input, UV);

    // Handle GreyScale converion
    if (ConvertToGreyscale)
    {
        finalColor.xyz = vec3(RGBToY(finalColor.xyz));
    }
    else
    {
        // Handle Ranges
        finalColor.x = InverseLerp(RedRange.x, RedRange.y, finalColor.x);
        finalColor.y = InverseLerp(GreenRange.x, GreenRange.y, finalColor.y);
        finalColor.z = InverseLerp(BlueRange.x, BlueRange.y, finalColor.z);
        finalColor.a = InverseLerp(AlphaRange.x, AlphaRange.y, finalColor.a);
        
        // Channels
        finalColor.x = ShowRed ? finalColor.x : 0.0f;
        finalColor.y = ShowGreen ? finalColor.y : 0.0f;
        finalColor.z = ShowGreen ? finalColor.z : 0.0f;
        finalColor.a = ShowAlpha ? finalColor.a : 1.0f;

        // Handle GreyScale display
        bvec2 RG = bvec2(ShowRed, ShowGreen);
        bvec2 GB = bvec2(ShowGreen, ShowBlue);
        bvec2 RB = bvec2(ShowRed, ShowBlue);
        bvec3 RGB = bvec3(ShowRed, ShowGreen, ShowBlue);

        finalColor.xyz = any(RG) == false && ShowBlue? vec3(finalColor.z) : finalColor.xyz;
        finalColor.xyz = any(GB) == false && ShowRed? vec3(finalColor.x) : finalColor.xyz;
        finalColor.xyz = any(RB) == false && ShowGreen? vec3(finalColor.y) : finalColor.xyz;
        finalColor = any(RGB) == false && ShowAlpha? vec4(vec3(finalColor.a), 1.0) : finalColor;
    }
    
    // EOTF
    if (UseOETF)
    {
        finalColor.xyz = clamp(finalColor.xyz, 0, 1);
        
        finalColor.xyz = SRGB_OETF(finalColor.xyz, 2.2 /* PC (Or PC monitor) Gamma, TV is 2.4*/);
    }
            
    OutColor = finalColor;
}
#endif // DISPLAY_PASS

#endif // FRAGMENT_SHADER