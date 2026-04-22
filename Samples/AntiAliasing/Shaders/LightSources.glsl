#ifndef INCLUDE_GUARD_GLSL_LIGHT_SOURCES
#define INCLUDE_GUARD_GLSL_LIGHT_SOURCES

struct DirectionalLight_t
{
    vec3 LightDir;
    vec3 LightColor;
    float LightIntensity;
};

// Scene lights
layout (binding = 1, std140)  uniform LightSources_t
{
    DirectionalLight_t SunLight;
} LightSources;

#endif // INCLUDE_GUARD_GLSL_LIGHT_SOURCES