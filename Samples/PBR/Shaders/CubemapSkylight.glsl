#ifndef INCLUDE_GUARD_GLSL_CUBEMAP_SKYLIGHT
#define INCLUDE_GUARD_GLSL_CUBEMAP_SKYLIGHT

// Cubemap Skylight
uniform samplerCube SkyLightCubeMap;

vec3 SampleSkylightColor(vec3 Direction)
{
    return texture(SkyLightCubeMap, Direction).xyz;
}

#endif // INCLUDE_GUARD_GLSL_CUBEMAP_SKYLIGHT