#version 450

#ifdef VERTEX_SHADER

#include "Include/Camera.glsl"

uniform bool useCameraBuffer;
layout(binding = 0, std430) readonly buffer Cameras
{
    CameraData cameras[];
};
uniform uint SourceCamera;
uniform uint TargetCamera;

uniform mat4 SourceMatrixProjToWorld;
uniform mat4 TargetMatrixWorldToProj;

void main()
{
    const vec3 frustum[8] = vec3[](
        vec3( -1, -1,  1 ),
        vec3(  1, -1,  1 ),
        vec3(  1,  1,  1 ),
        vec3( -1,  1,  1 ),
        vec3( -1, -1,  0 ),
        vec3(  1, -1,  0 ),
        vec3(  1,  1,  0 ),
        vec3( -1,  1,  0 )
    );

    const uint indices[24] = {
        0u, 1u, 2u, 3u,

        5u, 4u, 7u, 6u,

        4u, 0u, 3u, 7u,

        1u, 5u, 6u, 2u,

        3u, 2u, 6u, 7u,

        4u, 5u, 1u, 0u
    };

    if (useCameraBuffer == true)
    {
        vec4 SourceWorldPosition = ProjToWorld(cameras[SourceCamera], vec4(frustum[indices[gl_VertexID]], 1));
        SourceWorldPosition.xyz /= SourceWorldPosition.w;
        SourceWorldPosition.w = 1.0;
        gl_Position = WorldToProj(cameras[TargetCamera], SourceWorldPosition);
    }
    else
    {
        vec4 SourceWorldPosition = SourceMatrixProjToWorld * vec4(frustum[indices[gl_VertexID]], 1);
        SourceWorldPosition.xyz /= SourceWorldPosition.w;
        SourceWorldPosition.w = 1.0;
        gl_Position = TargetMatrixWorldToProj * SourceWorldPosition;
    }
}

#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

out vec4 OutColor;

void main()
{
    OutColor = vec4(1.f);
}
#endif // FRAGMENT_SHADER