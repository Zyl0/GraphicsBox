#version 430

#include "Camera.glsl"

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec3 normal;
layout(location= 2) in vec3 tangent;
layout(location= 3) in vec2 coordinates;

layout(location= 0) out vec3 FragWorldPosition;
layout(location= 1) out vec3 FragNormal;
layout(location= 2) out vec3 FragTangent;
layout(location= 3) out vec3 FragBiTangent;
layout(location= 4) out vec2 UV0;

uniform mat4 Model, InverseModel;

vec4 M(vec4 position)
{
    return Model * position;
}

vec4 InverseM(vec4 position)
{
    return position * InverseModel;
}

void main( )
{
    gl_Position = WorldToProj(M(vec4(position, 1))); 

    vec4 FragWorldPositionH  = M(vec4(position, 1));
    FragWorldPosition = FragWorldPositionH.xyz / FragWorldPositionH.w;

    FragNormal = normalize(vec3(M(vec4(normal,     0.0))));
    FragTangent = normalize(vec3(M(vec4(tangent,   0.0))));
    FragTangent = normalize(FragTangent - dot(FragTangent, FragNormal) * FragNormal);
    FragBiTangent = cross(FragNormal, FragTangent);

    UV0 = coordinates;
}
#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

#include "Include/Math.glsl"
#include "Include/FresnelSchlick.glsl"
#include "Include/GGX.glsl"
#include "LightSources.glsl"
#include "LightingModel.glsl"

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

// Derived from the sample GGX function assuming the surface is rough to get the most diffused samples
// vec3 SampleHemisphere(vec3 n, float U1, float U2)
// {
//     float phi = 2.0 * M_PI * U2;
//     float cosTheta = sqrt( 1.0 - U1 );
//     float sinTheta = sqrt( 1.0 - cosTheta * cosTheta );
// 
//     // from spherical coordinates to cartesian coordinates
//     vec3 H;
//     H.x = cos(phi) * sinTheta;
//     H.y = sin(phi) * sinTheta;
//     H.z = cosTheta;
// 
//     // from tangent-space vector to world-space sample vector
//     vec3 up        = abs(n.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
//     vec3 tangent   = normalize(cross(up, n));
//     vec3 bitangent = cross(n, tangent);
// 
//     vec3 sampleVec = tangent * H.x + bitangent * H.y + n * H.z;
//     return normalize(sampleVec);
// }

layout(location= 0) in vec3 FragWorldPosition;
layout(location= 1) in vec3 FragNormal;
layout(location= 2) in vec3 FragTangent;
layout(location= 3) in vec3 FragBiTangent;
layout(location= 4) in vec2 UV0;

// Material
uniform vec3 BaseColor;
uniform float Roughness;
uniform float Metalness;

// Uniform
uniform uint IndirectLightingSampleCount;

out vec4 OutColor;

void main( )
{
    mat3 TBN = mat3(FragTangent, FragBiTangent, FragNormal);

    // Hit point Material settings
    vec3 DiffuseColor = mix(BaseColor, vec3(0), Metalness);
    vec3 F0 = mix(vec3(0.04), BaseColor, Metalness);
    float Alpha = Roughness * Roughness;

    // TODO base color / normal / etc... textures
    // vec3 Normal = normalize(TBN * MaterialGetNormal());
    vec3 Normal =  FragNormal;

    vec3 finalColor = vec3(0);

    // Direct lighting
    {
        vec3 n = Normal;
        vec3 v = normalize(CameraWorldPosition() - FragWorldPosition);
        vec3 l = normalize(-LightSources.SunLight.LightDir);
        vec3 h = normalize(v + l);
        
        float CosThetaL = dot(n, l);
        float CosThetaV = dot(n, v);

        if(CosThetaV > 0 && CosThetaL > 0)
        {
            float VdotH = dot(h, v);
            vec3 F = Fresnel(VdotH, F0);

            float D = D_GGX_Heitz2014_EQ71(h, n, Alpha);

            float G = G2_Heitz2014_EQ99(h, n, v, l, Alpha);

            float DGNormalized = (D * G) / max(4 * CosThetaL * CosThetaV , 0.0001);

            vec3 ReflectanceDielectrical = fDielectrical(DiffuseColor, DGNormalized, F);
            vec3 ReflectanceMetallic = fMetallic(DGNormalized, F);

            vec3 Reflectance = mix(ReflectanceDielectrical, ReflectanceMetallic, Metalness);

            vec3 Light = LightSources.SunLight.LightColor * CosThetaL * LightSources.SunLight.LightIntensity;

            finalColor += Reflectance * Light;
        }
    }
    
    // Indirect lighting
    {
        // For now we only do a manual integration
        {
            vec3 v = normalize(CameraWorldPosition() - FragWorldPosition);
            
            vec3 sum = vec3(0);
            for (uint i = 0u; i < IndirectLightingSampleCount; i++ )
            {
                vec2 u = Hammersley(i, IndirectLightingSampleCount);

                // Ne
                vec3 SampledDirection = SampleGGXVNDF_Intel2023(Normal, Alpha, Alpha, u.x, u.y);

                vec3 n = SampledDirection;
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

                vec3 Reflectance = mix(ReflectanceDielectrical, ReflectanceMetallic, Metalness);
                
                vec3 SkyLight = SampleSkylightColor(l);

                sum += Reflectance * SkyLight;
            }

            finalColor += sum / float(IndirectLightingSampleCount);
        }
    }

    OutColor.xyz = finalColor;
    OutColor.w = 1.0;
}
#endif // FRAGMENT_SHADER