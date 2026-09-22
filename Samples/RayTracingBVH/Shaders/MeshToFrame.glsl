#version 430


#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec3 normal;
layout(location= 2) in vec3 tangents;
layout(location= 3) in vec2 coordinates;

uniform mat4 Model, ViewProjection;

layout(location= 0) out vec3 FragWorldPosition;
layout(location= 1) out vec3 FragWorldNormal;
layout(location= 2) out vec3 FragWorldTangent;
layout(location= 3) out vec3 FragWorldBiTangent;
layout(location= 4) out vec2 UV0;
layout(location= 5) out mat3 FragWorldTBN;

vec4 VP(vec4 position)
{
    return ViewProjection * position;
}

vec4 MVP(vec4 position)
{
    return VP(Model * position);
}

vec4 M(vec4 position)
{
    return Model * position;
}


void main( )
{
    gl_Position = MVP(vec4(position, 1));

    vec4 FragWorldPositionH  = M(vec4(position, 1));
    FragWorldPosition = FragWorldPositionH.xyz / FragWorldPositionH.w;

    FragWorldNormal = M(vec4(normal, 0)).xyz;
    FragWorldTangent = M(vec4(tangents, 0)).xyz;
    FragWorldBiTangent = cross(FragWorldNormal, FragWorldTangent); // always re-derive B from N×T
    FragWorldTBN = mat3(FragWorldTangent, FragWorldBiTangent, FragWorldNormal);

    UV0 = coordinates;
}
#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER
#include "Include/Math.glsl"
#include "Include/FresnelSchlick.glsl"
#include "Include/GGX.glsl"
#include "Include/PBRLightingModel.glsl"
#include "Include/ToneMapping.glsl"

layout(location= 0) in vec3 FragWorldPosition;
layout(location= 1) in vec3 FragNormal;
layout(location= 2) in vec3 FragTangent;
layout(location= 3) in vec3 FragBiTangent;
layout(location= 4) in vec2 UV0;
layout(location= 5) in mat3 FragTBN;

uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 ambientColor;
uniform vec3 cameraPosition;

uniform vec3 Emissive;
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
    vec3 LocalNormal = vec3(0,0,1);
    if (UseNormalTexture == 1)
    {
        LocalNormal = (texture(texNormal, UV0).xyz * 2.f - 1.f);
        // LocalNormal.x *= -1;
        // LocalNormal.y *= -1;
        Normal = normalize(FragTBN * LocalNormal);
    }

    vec3 finalColor = Emissive;

    // Direct lighting
    {
        vec3 n = Normal;
        vec3 v = normalize(cameraPosition - FragWorldPosition);
        vec3 l = normalize(lightDirection);
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

            finalColor += Reflectance * lightColor * CosThetaL;
        }
    }
    
    finalColor += PixAmbiantOcclusion * ambientColor;
    
    // OutColor.xyz = BaseColor * dot(normalize(FragWorldNormal), lightDirection) * lightColor + ambientColor * BaseColor + emissive;

    OutColor.xyz = finalColor;
    
    // sRGB OETF encoding
    OutColor.xyz = clamp(OutColor.xyz, 0, 1);
    OutColor.xyz = SRGB_OETF(OutColor.xyz, 2.2 /* PC (Or PC monitor) Gamma, TV is 2.4*/);

    OutColor.w = 1.0;
}
#endif // FRAGMENT_SHADER