#version 450

// Vertex Shader
#include "Passes/ScreenTriangle.glsl"

#ifdef FRAGMENT_SHADER

// Scene cameras
#include "Include/Camera.glsl"
#include "Include/Math.glsl"
#include "Include/FresnelSchlick.glsl"
#include "Include/GGX.glsl"
#include "LightSources.glsl"
#include "Include/PBRLightingModel.glsl"

layout(location = 0) in vec2 UV;
layout(location = 1) in vec2 UVProj;

uniform sampler2D GBufferAlbedo;
uniform sampler2D GPackedNormalTangent;
uniform sampler2D GBufferProperties;
uniform sampler2D GBufferDepth;

uniform vec3 LightDirection;
uniform vec3 LightColor;
uniform float LightIntensity;

layout(binding = 0, std430) readonly buffer Cameras
{
    CameraData cameras[];
};

out vec4 OutColor;

void main()
{
    vec3 PixBaseColor = texture(GBufferAlbedo, UV).xyz;
    vec4 PackedNormalTangent = texture(GPackedNormalTangent, UV);
    vec3 Normal = DecodeOctahedron(PackedNormalTangent.xy);
    vec3 Tangent = DecodeOctahedron(PackedNormalTangent.zw);
    vec3 SurfaceProperties = texture(GBufferProperties, UV).xyz;
    float PixRoughness = SurfaceProperties.x;
    float PixMetalness = SurfaceProperties.y;
    float PixAmbiantOcclusion = SurfaceProperties.z;
    float Depth = texture(GBufferDepth, UV).x;

    if (Depth >= 1.0f) discard;

    // Deproject position
    vec3 ViewPosition = ProjToView(cameras[0], vec3(UVProj, Depth));

    // Hit point Material settings
    vec3 DiffuseColor = mix(PixBaseColor, vec3(0), PixMetalness);
    vec3 F0 = mix(vec3(0.04), PixBaseColor, PixMetalness);
    float Alpha = PixRoughness * PixRoughness;

    vec3 finalColor = vec3(0);

    vec3 n = Normal;
    vec3 v = ViewToWorld(cameras[0], normalize(ViewPosition));
    vec3 l = normalize(-LightDirection);
    vec3 h = normalize(v + l);

    float CosThetaL = dot(n, l);
    float CosThetaV = dot(n, v);

    // Direct lighting
    if(CosThetaV > 0 && CosThetaL > 0)
    {
        float VdotH = dot(h, v);
        vec3 F = Fresnel(VdotH, F0);

        float D = D_GGX_Heitz2014_EQ71(h, n, Alpha);

        float G = G2_Heitz2014_EQ99(h, n, v, l, Alpha);

        float DGNormalized = (D * G) / max(4 * CosThetaL * CosThetaV , 0.0001);

        vec3 ReflectanceDielectrical = fDielectrical(DiffuseColor, DGNormalized, F);
        vec3 ReflectanceMetallic = fMetallic(DGNormalized, F);

        vec3 Reflectance = mix(ReflectanceDielectrical, ReflectanceMetallic, PixMetalness);

        vec3 Light = LightColor * CosThetaL * LightIntensity;

        finalColor += Reflectance * Light;
    }

    OutColor.xyz = finalColor;
    OutColor.w = 1.0;
}

#endif // FRAGMENT_SHADER