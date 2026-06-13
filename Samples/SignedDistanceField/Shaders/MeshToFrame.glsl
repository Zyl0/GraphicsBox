#version 430


#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec3 normal;

uniform mat4 Model, ViewProjection;

layout(location= 0) out vec3 FragWorldPosition;
layout(location= 1) out vec3 FragWorldNormal;

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

}
#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER
#include "Include/ToneMapping.glsl"

layout(location= 0) in vec3 FragWorldPosition;
layout(location= 1) in vec3 FragWorldNormal;

uniform vec3 lightColor;
uniform vec3 lightDirection;
uniform vec3 ambientColor;

uniform uint useGrid;
uniform float gridSize;
uniform float gridLineWidth;

out vec4 OutColor;

void main( )
{
    vec3 BaseColor = FragWorldNormal;

    if (useGrid != 0)
    {
        vec3 mod = abs(mod(FragWorldPosition, gridSize));

        bool IsGridLine = any(lessThan(mod, vec3(min(gridSize, gridLineWidth))));
        BaseColor = IsGridLine ? vec3(1) - BaseColor : BaseColor;
    }

    OutColor.xyz += max(dot(FragWorldNormal, lightDirection), 0.0) * lightColor * BaseColor;
    OutColor.xyz += ambientColor * BaseColor;

    // sRGB OETF encoding
    OutColor.xyz = clamp(OutColor.xyz, 0, 1);
    OutColor.xyz = SRGB_OETF(OutColor.xyz, 2.2 /* PC (Or PC monitor) Gamma, TV is 2.4*/);

    OutColor.w = 1.0;
}
#endif // FRAGMENT_SHADER