#version 430

// Scene cameras
#include "Include/Camera.glsl"

layout(binding = 0, std430) readonly buffer Cameras
{
    CameraData cameras[];
};

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
    gl_Position = WorldToProj(cameras[0], M(vec4(position, 1)));

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

layout(location= 0) out vec3 OutColor;
layout(location= 1) out vec3 OutNormal;
layout(location= 2) out vec3 OutProperties;

void main()
{
    OutColor = BaseColor;
    float PixMetalness = Metalness;
    float PixRoughness = Roughness;
    float PixAmbiantOcclusion = 1.f;

    if (UseColorTexture == 1)
    {
        OutColor = texture(texColor, UV0).xyz;
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

    OutNormal =  FragNormal;
    if (UseNormalTexture == 1)
    {
        vec3 LocalNormal = (texture(texNormal, UV0).xyz * 2.f - 1.f);
        LocalNormal.x *= -1;
        LocalNormal.y *= -1;
        OutNormal = normalize(FragTBN * LocalNormal);
    }

    OutProperties.x = PixRoughness;
    OutProperties.y = PixMetalness;
    OutProperties.z = PixAmbiantOcclusion;
}
#endif // FRAGMENT_SHADER