#version 450

#ifdef VERTEX_SHADER

layout(location = 0) out vec3 Direction;

uniform mat4 CameraProjToWorld;

vec3 ProjToWorld(vec3 Vector)
{
    Vector.y *= -1;
    
    return (CameraProjToWorld * vec4(Vector, 0.0f)).xyz;
}

void main()
{
    const vec2 pos[3] = vec2[]
    (
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );
    
    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);

    Direction = ProjToWorld(vec3(pos[gl_VertexID], 0.5));
}

#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

#include "HDRISkylight.glsl"

layout(location = 0) in vec3 Direction;

out vec4 OutColor;

void main()
{ 
    OutColor.xyz = SampleSkylightColor(normalize(Direction));
    OutColor.w = 1.;
}

#endif // FRAGMENT_SHADER