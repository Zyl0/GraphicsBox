#ifndef INCLUDE_GUARD_GLSL_MATH
#define INCLUDE_GUARD_GLSL_MATH

#define M_PI 3.1415926535897932384626433832795

vec2 SampleSphericalMap(vec3 v)
{
    const vec2 invAtan = vec2(0.1591, 0.3183);
    
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float RGBToLuminance(vec3 color)
{
    return color.x * 0.3086 + color.y * 0.6094 + color.z * 0.0820;
}

#endif // INCLUDE_GUARD_GLSL_MATH
