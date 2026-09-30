#version 450

// Ray settings
uniform vec3 A;
uniform vec3 B;
uniform vec3 Color;

#ifdef VERTEX_SHADER

#include "Include/Camera.glsl"

uniform bool useCameraBuffer;
layout(binding = 0, std430) readonly buffer Cameras
{
    CameraData cameras[];
};
uniform uint TargetCamera;
uniform mat4 CameraWorldToProj;

void main( )
{
    vec3 position = ((gl_VertexID > 0 )? A : B);
    if (useCameraBuffer == true)
    {
        gl_Position = WorldToProj(cameras[TargetCamera], vec4(position, 1));
    }
    else
    {
        gl_Position = CameraWorldToProj * vec4(position, 1);
    }
}
#endif

#ifdef FRAGMENT_SHADER

in float FragDist;
out vec4 OutColor;

void main( )
{
    OutColor.xyz = Color;
    OutColor.a = 1.f;
}
#endif