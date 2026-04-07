#ifndef INCLUDE_GUARD_GLSL_CUBEMAP_SKYLIGHT
#define INCLUDE_GUARD_GLSL_CUBEMAP_SKYLIGHT

// Cubemap Skylight
uniform samplerCube SkyLightCubeMap;
uniform uint SkyLightMipCount;

vec3 SampleSkylightColor(vec3 Direction)
{
    return texture(SkyLightCubeMap, Direction).xyz;
}

vec3 SampleSkylightColor(vec3 Direction, float Alpha)
{
    return texture(SkyLightCubeMap, Direction, Alpha * SkyLightMipCount).xyz;
}

#endif // INCLUDE_GUARD_GLSL_CUBEMAP_SKYLIGHT