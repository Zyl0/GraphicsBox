#ifndef INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT
#define INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT

#include "Include/Math.glsl"

// HDRI Skylight
uniform sampler2D SkyLightHDRi;
uniform uint SkyLightMipCount;

vec3 SampleSkylightColor(vec3 Direction)
{
    return texture(SkyLightHDRi, SampleSphericalMap(Direction)).xyz;
}

vec3 SampleSkylightColor(vec3 Direction, float Alpha)
{
    return texture(SkyLightHDRi, SampleSphericalMap(Direction), Alpha * SkyLightMipCount).xyz;
}

#endif // INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT