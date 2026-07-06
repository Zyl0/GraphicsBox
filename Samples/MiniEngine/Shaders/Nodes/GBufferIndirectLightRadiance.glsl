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

// Skylight method switch
#ifdef USE_CUBEMAP_SKYLIGHT
#include "Skylight/CubemapSkylight.glsl"
#endif // USE_CUBEMAP_SKYLIGHT
#ifdef USE_HDRI_SKYLIGHT
#include "Skylight/HDRISkylight.glsl"
#endif // USE_HDRI_SKYLIGHT


float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

layout(location = 0) in vec2 UV;
layout(location = 1) in vec2 UVProj;

uniform sampler2D GBufferAlbedo;
uniform sampler2D GPackedNormal;
uniform sampler2D GPackedTangent;
uniform sampler2D GBufferProperties;
uniform sampler2D GBufferDepth;

// Globals
uniform uint IndirectLightingSampleCount;

layout(binding = 0, std430) readonly buffer Cameras
{
    CameraData cameras[];
};

out vec4 OutColor;

void main()
{
    vec3 PixBaseColor = texture(GBufferAlbedo, UV).xyz;
    // vec4 PackedNormalTangent = texture(GPackedNormalTangent, UV);
    // vec3 Normal = DecodeOctahedron(PackedNormalTangent.xy);
    // vec3 Tangent = DecodeOctahedron(PackedNormalTangent.zw);
    vec3 Normal = texture(GPackedNormal, UV).xyz;
    vec3 Tangent = texture(GPackedTangent, UV).xyz;
    vec3 SurfaceProperties = texture(GBufferProperties, UV).xyz;
    float PixRoughness = SurfaceProperties.x;
    float PixMetalness = SurfaceProperties.y;
    float PixAmbiantOcclusion = SurfaceProperties.z;
    float Depth = texture(GBufferDepth, UV).x;

    if (Depth >= 1.0f) discard;

    // Deproject position
    vec4 ViewPosition = ProjToView(cameras[0], vec4(UVProj, Depth, 1));
    ViewPosition.xyz /= ViewPosition.w;


    // Hit point Material settings
    vec3 DiffuseColor = mix(PixBaseColor, vec3(0), PixMetalness);
    vec3 F0 = mix(vec3(0.04), PixBaseColor, PixMetalness);
    float Alpha = PixRoughness * PixRoughness;

    vec3 finalColor = vec3(0);

    vec3 v = ViewToWorld(cameras[0], -normalize(ViewPosition.xyz));

    vec3 BiTangent = cross(Normal, Tangent);
    mat3 TBN =  mat3(BiTangent, Tangent, Normal);
    mat3 InvTBN = transpose(TBN);

    vec3 vNormalSpace = InvTBN * v;

    vec3 sum = vec3(0);
    // For now we only do a manual integration
    for (uint i = 0u; i < IndirectLightingSampleCount; i++ )
    {
        vec2 u = Hammersley(i, IndirectLightingSampleCount);

        // Ne
        vec3 SampledDirection = SampleGGXVNDF_Intel2023(vNormalSpace, Alpha, Alpha, u.x, u.y);

        vec3 n = normalize(TBN * SampledDirection);
        vec3 l = reflect(v, n);
        vec3 h = normalize(v + l);

        // PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
        // float D = D_GGX_Heitz2014_EQ71(h, n, Alpha);
        // float G1 = G1_Heitz2014_EQ98(h, n, v, Alpha);
        // float pdf = G1 * max(0, dot(v, n)) * D / ???; // Ve.z probably not in the right coordinate space, should work better when baking a texture on flat

        float VdotH = dot(h, v);
        vec3 F = Fresnel(VdotH, F0);
        float G1 = G1_Heitz2014_EQ98(h, n, v, Alpha);
        float G2 = G2_Heitz2014_EQ99(h, n, v, l, Alpha);

        vec3 ReflectanceDielectrical = fDielectricalIndirect(DiffuseColor, G2 / G1, F);
        vec3 ReflectanceMetallic = fMetallicIndirect(DiffuseColor, G2 / G1, F);

        vec3 Reflectance = mix(ReflectanceDielectrical, ReflectanceMetallic, PixMetalness);

        vec3 SkyLight = SampleSkylightColor(l, PixRoughness);

        sum += (G1 > 0.0f && G2 > 0.0f) ? Reflectance * SkyLight : vec3(0.0f);
    }
    finalColor += (sum * PixAmbiantOcclusion) / float(IndirectLightingSampleCount);


    OutColor.xyz = finalColor;
    OutColor.w = 1.0;
}

#endif // FRAGMENT_SHADER