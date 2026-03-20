/*
This implementation of Fresnel is the one with no optimisation or aproximation, using the base formula
*/

#ifndef INCLUDE_GUARD_GLSL_FRESNEL
#define INCLUDE_GUARD_GLSL_FRESNEL

float FresnelG(float NdotL, float Refraction)
{
    return sqrt(Refraction * Refraction + NdotL * NdotL - 1);
}

vec2 FresnelG(float NdotL, vec2 Refraction)
{
    return sqrt(Refraction * Refraction + NdotL * NdotL - 1);
}

vec3 FresnelG(float NdotL, vec3 Refraction)
{
    return sqrt(Refraction * Refraction + NdotL * NdotL - 1);
}

vec4 FresnelG(float NdotL, vec4 Refraction)
{
    return sqrt(Refraction * Refraction + NdotL * NdotL - 1);
}


#define FresnelC(n, l) dot(n, l)


float Fresnel(float NdotL, float F0)
{
    float sqrtF0 = sqrt(F0);
    float Refraction = (1 + sqrtF0) / (1 - sqrtF0);

    float G = FresnelG(NdotL, Refraction);
    float C = FresnelC(NdotL, G);

    float PartA = (G - C) / (G + C);
    float PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

    return (1 / 2) * (PartA * PartA) * (1 + (PartB * PartB));
}

vec2 Fresnel(float NdotL, vec2 F0)
{
    vec2 sqrtF0 = sqrt(F0);
    vec2 Refraction = (1 + sqrtF0) / (1 - sqrtF0);

    vec2 G = FresnelG(NdotL, Refraction);
    float C = FresnelC(NdotL, G);

    vec2 PartA = (G - C) / (G + C);
    vec2 PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

    return (1 / 2) * (PartA * PartA) * (1 + (PartB * PartB));
}

vec3 Fresnel(float NdotL, vec3 F0)
{
    vec3 sqrtF0 = sqrt(F0);
    vec3 Refraction = (1 + sqrtF0) / (1 - sqrtF0);

    vec3 G = FresnelG(NdotL, Refraction);
    float C = FresnelC(NdotL, G);

    vec3 PartA = (G - C) / (G + C);
    vec3 PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

    return (1 / 2) * (PartA * PartA) * (1 + (PartB * PartB));
}

vec4 Fresnel(float NdotL, vec4 F0)
{
    vec4 sqrtF0 = sqrt(F0);
    vec4 Refraction = (1 + sqrtF0) / (1 - sqrtF0);

    vec4 G = FresnelG(NdotL, Refraction);
    float C = FresnelC(NdotL, G);

    vec4 PartA = (G - C) / (G + C);
    vec4 PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

    return (1 / 2) * (PartA * PartA) * (1 + (PartB * PartB));
}

#endif // INCLUDE_GUARD_GLSL_FRESNEL