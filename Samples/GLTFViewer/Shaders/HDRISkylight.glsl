#ifndef INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT
#define INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT

#include "Include/Math.glsl"

// HDRI Skylight
uniform sampler2D SkyLightHDRi;
uniform uint SkyLightMipCount;

vec3 SampleSkylightColor(vec3 Direction)
{
    vec2 uv = SampleSphericalMap(Direction);
    return texture(SkyLightHDRi, uv).xyz;
}

vec3 SampleSkylightColor(vec3 Direction, float Alpha)
{
    vec2 uv = SampleSphericalMap(Direction);
    return texture(SkyLightHDRi, uv, Alpha * SkyLightMipCount).xyz;
}

#endif // INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT