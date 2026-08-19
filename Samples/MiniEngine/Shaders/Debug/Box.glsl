#version 450

#ifdef VERTEX_SHADER

#include "Include/Camera.glsl"

uniform bool useCameraBuffer;
layout(binding = 0, std430) readonly buffer Cameras
{
    CameraData cameras[];
};
uniform uint Camera;

uniform mat4 Model;
uniform mat4 CameraWorldToProj;

uniform vec3 BoxMin;
uniform vec3 BoxMax;

void main()
{
    const vec3 frustum[8] = vec3[](
        // vec3( -1, -1, -1 ),
        // vec3( -1, -1, 1 ),
        // vec3( -1, 1, -1 ),
        // vec3( -1, 1, 1 ),
        // vec3( 1, -1, -1 ),
        // vec3( 1, -1, 1 ),
        // vec3( 1, 1, -1 ),
        // vec3( 1, 1, 1 )
    
        vec3( -1, -1,  1 ),
        vec3(  1, -1,  1 ),
        vec3(  1,  1,  1 ),
        vec3( -1,  1,  1 ),
        vec3( -1, -1, -1 ),
        vec3(  1, -1, -1 ),
        vec3(  1,  1, -1 ),
        vec3( -1,  1, -1 )
    );

    const uint indices[24] = {
        0u, 1u, 2u, 3u,
    
        5u, 4u, 7u, 6u,
    
        4u, 0u, 3u, 7u,
    
        1u, 5u, 6u, 2u,
    
        3u, 2u, 6u, 7u,
    
        4u, 5u, 1u, 0u
    };
    
    vec3 SourceVertex = mix(BoxMin,BoxMax,(frustum[indices[gl_VertexID]] + vec3(1.f) ) / 2.f);

    if (useCameraBuffer == true)
    {
        gl_Position = WorldToProj(cameras[Camera], Model * vec4(SourceVertex, 1));
    }
    else
    {
        gl_Position = CameraWorldToProj * Model * vec4(SourceVertex, 1);
    }
}

#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

uniform vec3 BoxColor;

out vec4 OutColor;

void main()
{
    OutColor = vec4(BoxColor, 1.f);
}
#endif // FRAGMENT_SHADER