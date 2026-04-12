#version 430

#include "Include/Camera.glsl"

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec3 normal;
layout(location= 2) in vec2 coordinates;

// TODO GLTF import to GPU does not provide tangents necessarly, needs fixing
// layout(location= 2) in vec3 tangent;
// layout(location= 3) in vec2 coordinates;

layout(location= 0) out vec3 FragWorldPosition;
layout(location= 1) out vec3 FragNormal;
layout(location= 2) out vec3 FragTangent;
layout(location= 3) out vec3 FragBiTangent;
layout(location= 4) out vec2 UV0;
layout(location= 5) out mat3 FragTBN;

uniform mat4 Model, InverseModel;

vec4 M(vec4 position)
{
    return Model * position;
}

vec4 InverseM(vec4 position)
{
    return position * InverseModel;
}

vec3 gramSchmidt(vec3 T, vec3 N) 
{
    return normalize(T - dot(T, N) * N);
}

void main( )
{
    gl_Position = WorldToProj(M(vec4(position, 1))); 

    vec4 FragWorldPositionH  = M(vec4(position, 1));
    FragWorldPosition = FragWorldPositionH.xyz / FragWorldPositionH.w;

    UV0 = coordinates;

    // TODO GLTF import to GPU does not provide tangents necessarly, needs fixing
    // FragNormal = normalize(vec3(M(vec4(normal,     0.0))));
    // FragTangent = normalize(vec3(M(vec4(tangent,   0.0))));
    // FragTangent = normalize(FragTangent - dot(FragTangent, FragNormal) * FragNormal);
    // FragBiTangent = cross(FragNormal, FragTangent);

    FragNormal = normalize(vec3(M(vec4(normal,     0.0))));

    // ── Tangent derivation ────────────────────────────────────────────────
    // We need a vector that points in the direction of increasing U on the
    // surface. With only per-vertex data we approximate this by choosing an
    // arbitrary "up" reference that is not parallel to N, then projecting it
    // onto the tangent plane.  We pick between two candidates to avoid the
    // singularity when N is nearly parallel to the candidate.

    // Candidate 1: world +X  (good when N is mostly vertical)
    // Candidate 2: world +Y  (good when N is mostly horizontal)
    // Choosing the one that is most perpendicular to N minimises the
    // initial skew before Gram-Schmidt.
    vec3 refAxis   = (abs(FragNormal.y) < 0.9) ? vec3(0.0, 1.0, 0.0) : vec3(0.0, 0.0, 1.0);

    // Initial tangent: perpendicular to N, aimed along refAxis.
    // This gives a consistent "U direction" over the surface that
    // aligns with typical cylindrical / planar UV layouts.
    vec3 T_raw = gramSchmidt(refAxis, FragNormal);

    // Incorporate the actual UV coordinates so that the tangent tracks the
    // UV seams rather than just the geometry.  We rotate T_raw by the
    // per-vertex UV angle — i.e. bias T toward the dU direction implied by
    // the texCoord.  This is a lightweight approximation; for exact results
    // use dFdx/dFdy in the fragment shader or pre-computed tangents.
    float uvAngle  = atan(coordinates.y, coordinates.x);  // U direction hint
    float cosA     = cos(uvAngle);
    float sinA     = sin(uvAngle);
    vec3  B_raw    = cross(FragNormal, T_raw);  // initial bitangent

    // Rotate T_raw in the tangent plane by uvAngle
    FragTangent = normalize(cosA * T_raw + sinA * B_raw);
    FragBiTangent = cross(FragNormal, FragTangent); // always re-derive B from N×T
    FragTBN = mat3(FragTangent, FragBiTangent, FragNormal);
}
#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

#include "Include/Math.glsl"
#include "Include/FresnelSchlick.glsl"
#include "Include/GGX.glsl"
#include "LightSources.glsl"
#include "Include/PBRLightingModel.glsl"

// Skylight method switch
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

layout(location= 0) in vec3 FragWorldPosition;
layout(location= 1) in vec3 FragNormal;
layout(location= 2) in vec3 FragTangent;
layout(location= 3) in vec3 FragBiTangent;
layout(location= 4) in vec2 UV0;
layout(location= 5) in mat3 FragTBN;

// Material
uniform vec3 BaseColor;
uniform float Roughness;
uniform float Metalness;

uniform uint UseColorTexture;
uniform uint UseNormalTexture;
uniform uint UseMRTexture;
uniform uint UseAOTexture;

uniform sampler2D texColor;
uniform sampler2D texNormal;
uniform sampler2D texMR;
uniform sampler2D texAO;

// Globals
uniform uint IndirectLightingSampleCount;

out vec4 OutColor;

void main( )
{
    vec3 PixBaseColor = BaseColor;
    float PixMetalness = Metalness;
    float PixRoughness = Roughness;
    float PixAmbiantOcclusion = 1.f;
    
    if (UseColorTexture == 1)
    {
        PixBaseColor = texture(texColor, UV0).xyz;
    }
    if (UseMRTexture == 1)
    {
        vec3 mr = texture(texMR, UV0).xyz;
        PixMetalness = mr.z;
        PixRoughness = mr.y;
    }
    if (UseAOTexture == 1)
    {
        PixAmbiantOcclusion = texture(texAO, UV0).x;
    }
    
    // Clamp roughness
    PixRoughness = max(PixRoughness, 0.004);
    
    // Hit point Material settings
    vec3 DiffuseColor = mix(BaseColor, vec3(0), PixBaseColor);
    vec3 F0 = mix(vec3(0.04), PixBaseColor, PixMetalness);
    float Alpha = PixRoughness * PixRoughness;
    
    vec3 Normal =  FragNormal;
    if (UseNormalTexture == 1)
    {
        Normal = normalize(FragTBN * (texture(texNormal, UV0).xyz * 2.f - 1.f));
    }

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

            vec3 Reflectance = mix(ReflectanceDielectrical, ReflectanceMetallic, PixMetalness);

            vec3 Light = LightSources.SunLight.LightColor * CosThetaL * LightSources.SunLight.LightIntensity;

            finalColor += Reflectance * Light;
        }
    }
    
    // Indirect lighting
    {
        // For now we only do a manual integration
        {
            vec3 v = normalize(CameraWorldPosition() - FragWorldPosition);
            mat3 InvFragTBN = transpose(FragTBN);
            
            vec3 vNormalSpace = InvFragTBN * v;
            
            vec3 sum = vec3(0);
            for (uint i = 0u; i < IndirectLightingSampleCount; i++ )
            {
                vec2 u = Hammersley(i, IndirectLightingSampleCount);

                // Ne
                vec3 SampledDirection = SampleGGXVNDF_Intel2023(vNormalSpace, Alpha, Alpha, u.x, u.y);

                vec3 n = normalize(FragTBN * SampledDirection);
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
        }
    }

    OutColor.xyz = finalColor;
    OutColor.w = 1.0;
}
#endif // FRAGMENT_SHADER