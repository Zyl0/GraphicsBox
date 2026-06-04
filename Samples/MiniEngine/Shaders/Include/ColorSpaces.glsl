#ifndef INCLUDE_GUARD_GLSL_COLOR_SPACES
#define INCLUDE_GUARD_GLSL_COLOR_SPACES

struct Chromaticities
{
    vec2 red;
    vec2 green;
    vec2 blue;
    vec2 white;
};

float RGBToY(vec3 Linear)
{
    return Linear.x * 0.2126 + Linear.y * 0.7152 + Linear.z * 0.0722;
}

#endif // INCLUDE_GUARD_GLSL_COLOR_SPACES