#ifndef INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT
#define INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT

#include "Include/Math.glsl"

// HDRI Skylight
uniform sampler2D SkyLightHDRi;

vec3 SampleSkylightColor(vec3 Direction)
{
    return texture(SkyLightHDRi, SampleSphericalMap(Direction)).xyz;
}

#endif // INCLUDE_GUARD_GLSL_HDRI_SKYLIGHT