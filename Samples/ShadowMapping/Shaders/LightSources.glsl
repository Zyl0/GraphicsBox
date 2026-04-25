#ifndef INCLUDE_GUARD_GLSL_LIGHT_SOURCES
#define INCLUDE_GUARD_GLSL_LIGHT_SOURCES

struct LightColor_t
{
    vec3 LightColor;
    float LightIntensity;
};

struct DirectionalLight_t
{
    LightColor_t Color;
    
    uint Camera;
    uint ShadowMap;
    vec2 pad;
};

struct PointLight_t
{
    LightColor_t Color;
    
    uint Cameras[6];
    uint ShadowMaps[6];
    float Size;
    float pad;
};

#endif // INCLUDE_GUARD_GLSL_LIGHT_SOURCES