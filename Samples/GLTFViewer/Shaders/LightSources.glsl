#ifndef INCLUDE_GUARD_GLSL_LIGHT_SOURCES
#define INCLUDE_GUARD_GLSL_LIGHT_SOURCES

struct DirectionalLight_t
{
    vec3 LightDir;
    vec3 LightColor;
    float LightIntensity;
};

#endif // INCLUDE_GUARD_GLSL_LIGHT_SOURCES