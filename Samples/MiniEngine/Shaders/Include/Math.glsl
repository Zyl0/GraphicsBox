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

vec2 PolarToCartesian(vec2 Polar)
{
    return Polar.y * vec2( cos(Polar.x * M_PI * 2.), sin(Polar.x * M_PI * 2.));
}

vec2 CartesianToPolar(vec2 Cartesian)
{
    float radius = length(Cartesian);

    float angle = atan(Cartesian.y, Cartesian.x);
    angle = fract(angle / (2.0 * M_PI) + 1.0);

    return vec2(angle, radius);
}

float RGBToLuminance(vec3 color)
{
    return color.x * 0.3086 + color.y * 0.6094 + color.z * 0.0820;
}

vec2 EncodeOctahedron(vec3 v) 
{
    v /= (abs(v.x) + abs(v.y) + abs(v.z));
    v.xy = v.z >= 0.0 ? v.xy : (1.0 - abs(v.yx)) * sign(v.xy);
    return v.xy;
}

vec3 DecodeOctahedron(vec2 f) 
{
    vec3 v = vec3(f.xy, 1.0 - abs(f.x) - abs(f.y));
    if (v.z < 0.0) v.xy = (1.0 - abs(v.yx)) * sign(v.xy);
    return normalize(v);
}

#endif // INCLUDE_GUARD_GLSL_MATH
