
#ifndef INCLUDE_GUARD_TONE_MAPPING
#define INCLUDE_GUARD_TONE_MAPPING

/* ____________________________________ OOTF ____________________________________ */

vec3 SRGB_OETF(vec3 L, float InvGamma)
{
    vec3 dark = L * 12.92;
    vec3 light = 1.055 * pow(L, vec3(1.0 / InvGamma)) - 0.055;

    bvec3 cri;
    cri.x = L.x <= 0.0031308;
    cri.y = L.y <= 0.0031308;
    cri.z = L.z <= 0.0031308;

    vec3 r;
    r.x = cri.x ? dark.x : light.x;
    r.y = cri.y ? dark.y : light.y;
    r.z = cri.z ? dark.z : light.z;
    return r;
}

vec3 SRGB_EOTF(vec3 E, float Gamma )
{
    vec3 dark = E/12.92;
    vec3 light = pow((E + 0.055) / (1 + 0.055), vec3(Gamma));

    bvec3 cri;
    cri.x = E.x <= 0.04045;
    cri.y = E.y <= 0.04045;
    cri.z = E.z <= 0.04045;

    vec3 r;
    r.x = cri.x ? dark.x : light.x;
    r.y = cri.y ? dark.y : light.y;
    r.z = cri.z ? dark.z : light.z;
    return r;
}

// PQ OETF - Linear luminance (cd/m^2) to PQ encoded value [0,1]
vec3 OETF_PQ(vec3 L)
{
    const float m1 = 0.1593017578125;  // 2610 / 16384
    const float m2 = 78.84375;          // 2523 / 32
    const float c1 = 0.8359375;         // 3424 / 4096
    const float c2 = 18.8515625;        // 2413 / 128
    const float c3 = 18.6875;           // 2392 / 128

    L = max(L, vec3(0.0));
    vec3 Lm1 = pow(L, vec3(m1));
    vec3 numerator = c1 + c2 * Lm1;
    vec3 denominator = 1.0 + c3 * Lm1;
    vec3 value = pow(numerator / denominator, vec3(m2));
    return clamp(value, vec3(0.0), vec3(1.0));
}

// PQ EOTF - PQ encoded value [0,1] to linear luminance (cd/m^2)
vec3 EOTF_PQ(vec3 V)
{
    const float m1 = 0.1593017578125;  // 2610 / 16384
    const float m2 = 78.84375;          // 2523 / 32
    const float c1 = 0.8359375;         // 3424 / 4096
    const float c2 = 18.8515625;        // 2413 / 128
    const float c3 = 18.6875;           // 2392 / 128

    V = clamp(V, vec3(0.0), vec3(1.0));
    vec3 V1m2 = pow(V, vec3(1.0 / m2));
    vec3 numerator = max(V1m2 - vec3(c1), vec3(0.0));
    vec3 denominator = vec3(c2) - vec3(c3) * V1m2;
    vec3 L = pow(numerator / denominator, vec3(1.0 / m1));
    return L;
}

#endif // INCLUDE_GUARD_TONE_MAPPING