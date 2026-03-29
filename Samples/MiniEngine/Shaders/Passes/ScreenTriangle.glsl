#ifndef INCLUDE_GUARD_GLSL_SCREEN_TRIANGLE
#define INCLUDE_GUARD_GLSL_SCREEN_TRIANGLE

#ifdef VERTEX_SHADER

layout(location = 0) out vec2 UV;
layout(location = 1) out vec2 UVProj;

void main()
{
    const vec2 pos[3] = vec2[](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    vec2 texCoords[3] = vec2[3](
        vec2(0.0, 0.0),
        vec2(2.0, 0.0),
        vec2(0.0, 2.0)
    );

    gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);

    UV = texCoords[gl_VertexID];
    UVProj = pos[gl_VertexID];
}

#endif // VERTEX_SHADER

#endif // INCLUDE_GUARD_GLSL_SCREEN_TRIANGLE