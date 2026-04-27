#version 450

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 UV;
layout(location = 1) out vec2 UVProj;

layout(location = 0) out vec3 ColorWeights;

void main()
{
    const vec2 pos[3] = vec2[](
        vec2(-0.5f, -0.5f),
        vec2( 0.5f, -0.5f),
        vec2( 0.0f,  0.5f)
    );

    vec3 weights[3] = vec3[3](
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    );

    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);

    ColorWeights = weights[gl_VertexID];
}

#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

layout(location = 1) in vec3 ColorWeights;

out vec4 OutColor;

uniform vec3 ColorA;
uniform vec3 ColorB;
uniform vec3 ColorC;

void main()
{
    vec3 finalColor = vec3(0);

    finalColor += ColorA * ColorWeights.x;
    finalColor += ColorB * ColorWeights.y;
    finalColor += ColorC * ColorWeights.z;
    
    OutColor.xyz = finalColor;
    OutColor.w = 1.;
}

#endif // FRAGMENT_SHADER