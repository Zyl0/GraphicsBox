#version 450

#include "FresnelSchlick.glsl"
#include "GGX.glsl"

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec3 normal;
layout(location= 2) in vec3 tangent;
layout(location= 3) in vec2 coordinates;

out vec3 FragWorldPosition;
out vec3 FragNormal;
out vec3 FragTangent;
out vec3 FragBiTangent;
out vec2 UV0;

uniform mat4 Model, InverseModel;
uniform mat4 ViewProjection, InverseViewProjection;

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
in vec3 FragWorldPosition;
in vec3 FragNormal;
in vec3 FragTangent;
in vec3 FragBiTangent;
in vec2 UV0;

// Material
uniform vec3 BaseColorRGB;
uniform float Roughness;
uniform float Metalness;

// Directional Light
uniform vec3 LightDir;
uniform vec3 LightColorRGB;
uniform float LightIntensity;

uniform vec3 CameraPosition;

out vec4 fragColor;

vec3 fDielectrical( vec3 DiffuseColor, float DGNormalized, vec3 F )
{
    return ((1 - F) * DiffuseColor / M_PI) + (F * DGNormalized);
}

vec3 fMetallic(float DGNormalized, vec3 F )
{
    return ((1 - F) * vec3(0)) + (F * DGNormalized);
}

void main( )
{
    mat3 TBN = mat3(FragTangent, FragBiTangent, FragNormal);

    // Hit point Material settings
    vec3 DiffuseColor = mix(BaseColorRGB, vec3(0), Metalness);
    vec3 F0 = mix(vec3(0.04), BaseColorRGB, Metalness);
    float Alpha = Roughness * Roughness;

    // TODO base color / normal / etc... textures
    // vec3 Normal = normalize(TBN * MaterialGetNormal());
    vec3 Normal =  FragNormal;

    vec3 finalColor = vec3(0);

    // Direct lighting
    {
        vec3 n = Normal;
        vec3 v = normalize(CameraPosition - FragWorldPosition);
        vec3 l = normalize(-LightDir);
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

            vec3 Reflectance = clamp(mix(ReflectanceDielectrical, ReflectanceMetallic, Metalness), 0, 1);

            vec3 Light = LightColorRGB * CosThetaL * LightIntensity;

            finalColor += vec4(Reflectance * Light, 1.0);
        }
    }

    fragColor.xyz = finalColor;
    fragColor.w = 1.0;
}
#endif // FRAGMENT_SHADER