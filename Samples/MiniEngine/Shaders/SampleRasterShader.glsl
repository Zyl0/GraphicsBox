#version 430


#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;

uniform mat4 Model, ViewProjection;

layout(location= 0) out vec3 FragWorldPosition;

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

}
#endif // VERTEX_SHADER

#ifdef FRAGMENT_SHADER

layout(location= 0) in vec3 FragWorldPosition;

out vec4 OutColor;

void main( )
{
    OutColor.xyz = FragWorldPosition;
    OutColor.w = 1.0;
}
#endif // FRAGMENT_SHADER