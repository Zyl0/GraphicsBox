/*
The Fresnel Schlick approximation

TODO see if we need to use N dot L or V dot H
*/

#ifndef INCLUDE_GUARD_GLSL_FRESNEL_SCHLICK
#define INCLUDE_GUARD_GLSL_FRESNEL_SCHLICK

float Fresnel(float NdotL, float F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - clamp(NdotL, 0, 1), 5.0);
}

vec2 Fresnel(float NdotL, vec2 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - clamp(NdotL, 0, 1), 5.0);
}

vec3 Fresnel(float NdotL, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - clamp(NdotL, 0, 1), 5.0);
}

vec4 Fresnel(float NdotL, vec4 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - clamp(NdotL, 0, 1), 5.0);
}

#endif // INCLUDE_GUARD_GLSL_FRESNEL_SCHLICK