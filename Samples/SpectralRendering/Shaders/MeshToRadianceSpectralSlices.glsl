#version 430

uniform mat4 Model, InverseModel;
uniform mat4 ViewProjection, InverseViewProjection;

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

vec4 VP(vec4 position)
{
    return ViewProjection * position;
}

vec4 InverseVP(vec4 position)
{
    return position * InverseViewProjection;
}

vec4 MVP(vec4 position)
{
    return VP(Model * position);
}

vec4 InverseMVP(vec4 position)
{
    return InverseVP(position) * InverseModel;
}

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
    gl_Position = MVP(vec4(position, 1)); 

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

#include "Include/ToneMapping.glsl"
#include "FresnelSchlick.glsl"
#include "GGX.glsl"
#include "SpectralRendering.glsl"

layout(location= 0) in vec3 FragWorldPosition;
layout(location= 1) in vec3 FragNormal;
layout(location= 2) in vec3 FragTangent;
layout(location= 3) in vec3 FragBiTangent;
layout(location= 4) in vec2 UV0;

// Material
uniform vec4 BaseColorPack0;
uniform vec4 BaseColorPack1;
uniform vec4 BaseColorPack2;
uniform vec4 BaseColorPack3;
uniform float Roughness;
uniform float Metalness;

// Directional Light
uniform vec3 LightDir;
uniform float LightIntensity;
uniform vec4 LightColorPack0;
uniform vec4 LightColorPack1;
uniform vec4 LightColorPack2;
uniform vec4 LightColorPack3;

// Camera position
uniform vec3 CameraPosition;

// Sliced spectral rendering
uniform uint SampleCount; // Can support up to 16 samples (4 x 4 samples)

// Color Data
uniform mat4 XYZToRec709sRGB;

out vec4 OutColor;

vec3 CSXYZToCSRec709(vec3 color)
{
    return (XYZToRec709sRGB * vec4(color, 1.0)).xyz;
}

vec4 fDielectrical( vec4 DiffuseColor, float DGNormalized, vec4 F )
{
    return ((1 - F) * DiffuseColor / M_PI) + (F * DGNormalized);
}

vec4 fMetallic(float DGNormalized, vec4 F )
{
    return ((1 - F) * vec4(0)) + (F * DGNormalized);
}

vec4 f(vec4 Diffuse, vec4 F, float DGNormalized, float Metalness)
{
    vec4 ReflectanceDielectrical = fDielectrical(Diffuse, DGNormalized, F);
    vec4 ReflectanceMetallic = fMetallic(DGNormalized, F);
    
    return clamp(mix(ReflectanceDielectrical, ReflectanceMetallic, Metalness), 0, 1);
}

void main( )
{
    if (SampleCount == 0 || SampleCount > 16)
    {
        OutColor.xyz = vec3(1,0,0);
        OutColor.w = 1.;
        return;
    }

    // TODO base color / normal / etc... textures
    vec3 Normal =  FragNormal;
    float Alpha = Roughness * Roughness;
    
    vec3 n = Normal;
    vec3 v = normalize(CameraPosition - FragWorldPosition);
    vec3 l = normalize(-LightDir);
    vec3 h = normalize(v + l);

    float CosThetaL = dot(n, l);
    float CosThetaV = dot(n, v);

    vec3 finalColor = vec3(0);

    // Direct lighting
    if(CosThetaV > 0 && CosThetaL > 0)
    {
        float D = D_GGX_Heitz2014_EQ71(h, n, Alpha);

        float G = G2_Heitz2014_EQ99(h, n, v, l, Alpha);

        float DGNormalized = (D * G) / max(4 * CosThetaL * CosThetaV , 0.0001);
        
        vec4 Reactions[4] = vec4[4](vec4(0), vec4(0), vec4(0), vec4(0));
        // Compute reaction (luminence distribution over the spectrum) for each wavelenght
        {
            float VdotH = dot(h, v);
            
            if (SampleCount > 12)
            {
                vec4 Diffuse = mix(BaseColorPack3, vec4(0), Metalness);
                vec4 F0 = mix(vec4(0.04), BaseColorPack3, Metalness);
                
                Reactions[3] = f(Diffuse, Fresnel(VdotH, F0), DGNormalized, Metalness);
                Reactions[3] *= LightColorPack3 * CosThetaL * LightIntensity;
            }
            if (SampleCount > 8)
            {
                vec4 Diffuse = mix(BaseColorPack2, vec4(0), Metalness);
                vec4 F0 = mix(vec4(0.04), BaseColorPack2, Metalness);

                Reactions[2] = f(Diffuse, Fresnel(VdotH, F0), DGNormalized, Metalness);
                Reactions[2] *= LightColorPack2 * CosThetaL * LightIntensity;
            }
            if (SampleCount > 4)
            {
                vec4 Diffuse = mix(BaseColorPack1, vec4(0), Metalness);
                vec4 F0 = mix(vec4(0.04), BaseColorPack1, Metalness);

                Reactions[1] = f(Diffuse, Fresnel(VdotH, F0), DGNormalized, Metalness);
                Reactions[1] *= LightColorPack1 * CosThetaL * LightIntensity;
            }
            vec4 Diffuse = mix(BaseColorPack0, vec4(0), Metalness);
            vec4 F0 = mix(vec4(0.04), BaseColorPack0, Metalness);

            Reactions[0] = f(Diffuse, Fresnel(VdotH, F0), DGNormalized, Metalness);
            Reactions[0] *= LightColorPack0 * CosThetaL * LightIntensity;
        }
        
        // Integrate
        {
            float Range = 760. - 400.;
            float Offset = 400.;
            
            // Simple inverse distribution
            // TODO customizable or better wavelenght sample distribution
            for (uint WaveID = 0; WaveID < SampleCount; WaveID+=4)
            {
                float WavePosition; vec3 XYZ;
                
                WavePosition = float(WaveID + 0) / float(SampleCount);
                finalColor += WavelengthToXYZ(WavePosition * Range + Offset) * Reactions[WaveID / 4].x;
                
                WavePosition = float(WaveID + 1) / float(SampleCount);
                finalColor += WavelengthToXYZ(WavePosition * Range + Offset) * Reactions[WaveID / 4].y;
                
                WavePosition = float(WaveID + 2) / float(SampleCount);
                finalColor += WavelengthToXYZ(WavePosition * Range + Offset) * Reactions[WaveID / 4].z;
                
                WavePosition = float(WaveID + 3) / float(SampleCount);
                finalColor += WavelengthToXYZ(WavePosition * Range + Offset) * Reactions[WaveID / 4].w;
                
            }
            finalColor = CSXYZToCSRec709(finalColor / float(SampleCount));
        }
    }
    
    OutColor.xyz = SRGB_OETF(finalColor, 2.2);
    OutColor.w = 1.0;
}
#endif // FRAGMENT_SHADER