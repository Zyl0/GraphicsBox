#version 430

#include "Include/Camera.glsl"

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;

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
}
#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

out vec4 OutColor;

void main()
{
    OutColor = vec4(1.f);
}
#endif // FRAGMENT_SHADER