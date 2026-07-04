#version 450

// Ray settings
uniform vec3 Position;
uniform vec3 Direction;
uniform float Distance;
uniform float HitDistance;

#ifdef VERTEX_SHADER

#include "Include/Camera.glsl"

uniform bool useCameraBuffer;
layout(binding = 0, std430) readonly buffer Cameras
{
    CameraData cameras[];
};
uniform uint TargetCamera;
uniform mat4 CameraWorldToProj;

out float FragDist;

void main( )
{
    vec3 position = Position + ((gl_VertexID > 0 )? Direction * Distance : vec3(0));
    if (useCameraBuffer == true)
    {
        gl_Position = WorldToProj(cameras[TargetCamera], vec4(position, 1));
    }
    else
    {
        gl_Position = CameraWorldToProj * vec4(position, 1);
    }

    FragDist = (gl_VertexID > 0) ? Distance : 0;
}
#endif

#ifdef FRAGMENT_SHADER

in float FragDist;
out vec4 OutColor;

void main( )
{
    OutColor = FragDist < HitDistance ? vec4(1.0,0.1,0.1,1.0) : vec4(0.1,0.1,1.0,1.0);
}
#endif