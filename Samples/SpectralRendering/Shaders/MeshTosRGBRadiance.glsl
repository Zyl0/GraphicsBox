#version 450

#include "FesnelSchlick.glsl"
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
uniform vec3 BaseColorsRGB;
uniform float Roughness;
uniform float Metalness;

// Directionnal Light
uniform vec3 LightDir;
uniform vec3 LightColor;
uniform float LightIntensity;

void main( )
{
    mat3 TBN = mat3(FragTangent, FragBiTangent, FragNormal);

    // Hit point Material settings
    vec3 DiffuseColor = mix(BaseColorsRGB, vec3(0), Metalness);
    vec3 F0 = mix(vec3(0.04), BaseColorsRGB, Metalness);
    float Alpha = Roughness * Roughness;

    // TODO base color / normal / etc... textures
    // vec3 Normal = normalize(TBN * MaterialGetNormal());
    vec3 Normal =  FragNormal;

    vec3 finalColor = vec4(0);

    // Direct lighting
    {
        vec3 n = Normal;
        vec3 v = normalize(CameraPosition - position);
        vec3 l = normalize(LightDir);
        vec3 h = normalize(V + H);
        
        float CosThetaL = dot(n, l);
        float CosThetaV = dot(n, v);

        if(CosThetaV > 0 && CosThetaL > 0)
        {
            float VdotH = dot(h, v);
            vec3 F = Fresnel(VdotH, v, F0);

            float D = D_GGX_Heitz2014_EQ71(h, n, alpha);

            float G = G2_GeometrySmith(n, v, l, roughness);

            float DGNormalized = (D * G) / max(4 * CosThetaL * CosThetaV , 0.0001);

            vec3 ReflectanceDielectrical = fDielectrical(DiffuseColor, DGNormalized, F);
            vec3 ReflectanceMetallic = fMetallic(DGNormalized, F);

            vec3 Reflectance = clamp(mix(ReflectanceDielectrical, ReflectanceMetallic, metallic), 0, 1);

            vec3 Light = LightColor * CosThetaL * RecievedIntensity;

            finalColor += vec4(Reflectance * Light, 1.0);
        }
    }

    gl_FragColor = finalColor;
}
#endif // FRAGMENT_SHADER