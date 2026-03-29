#ifndef INCLUDE_GUARD_GLSL_GGX
#define INCLUDE_GUARD_GLSL_GGX

#include "Math.glsl"

#define Activation(x) ((x) < 0 ? 0 : 1)

float D_GGX_Heitz2014_EQ71(vec3 h, vec3 n, float alpha)
{
    float Cos2Theta = dot(h, n) * dot(h, n);
    float Tan2Theta = (1 / Cos2Theta) - 1;
    float alpha2 = alpha * alpha;

    //todo name
    float t = ( 1 + ( Tan2Theta / alpha2 ) );

    float numerator = Activation(dot(h, n));
    float denominator = M_PI * alpha2 * Cos2Theta * Cos2Theta * t * t;

    return numerator / denominator;
}

float D_GGX_Heitz2014_EQ71_Simplified(vec3 h, vec3 n, float alpha)
{
    float a2     = alpha*alpha;
    float NdotH  = max(dot(n, h), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;

    return num / denom;
}

float Lambda_Heitz2014_EQ72(vec3 n, vec3 w, float alpha2)
{
    vec3 wNormalized = normalize(w);
    float Cos2Theta = dot(wNormalized, n) * dot(wNormalized, n);
    float Tan2Theta = (1 / Cos2Theta) - 1;

    return (-1.0/2.0) + (1.0/2.0) * sqrt(1.0 + (alpha2 * Tan2Theta));
}

float G1_Heitz2014_EQ98(vec3 h, vec3 n, vec3 v, float alpha)
{
    float vdoth = dot(v, h);
    float alpha2 = alpha * alpha;

    float numerator = Activation(vdoth);
    float denominator = 1 + Lambda_Heitz2014_EQ72(n, v, alpha2);

    return numerator / denominator;
}

float G2_Heitz2014_EQ99(vec3 h, vec3 n, vec3 v, vec3 l, float alpha)
{
    float vdoth = dot(v, h);
    float ldoth = dot(l, h);
    float alpha2 = alpha * alpha;

    float numerator = Activation(vdoth) * Activation(ldoth);
    float denominator = 1 + Lambda_Heitz2014_EQ72(n, v, alpha2) + Lambda_Heitz2014_EQ72(n, l, alpha2);

    return numerator / denominator;
}


vec3 SampleGGX(vec3 v, float alpha_x, float alpha_y, float U1, float U2)
{
    //float cosTheta = U1;
    //float sinTheta = sqrt(1 - U1*U1);
    //float phi = 2 * PI * U2;

    //vec3 l = vec3( cos(phi) * sinTheta, cosTheta, sinTheta * sin(phi) );

    float phi = 2.0 * M_PI * U2;
    float cosTheta = sqrt( (1.0 - U1) / (1.0 + (alpha_x * alpha_x - 1.0) * U2) );
    float sinTheta = sqrt( 1.0 - cosTheta * cosTheta );
    // todo use alpha_y

    // from spherical coordinates to cartesian coordinates
    vec3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

    // from tangent-space vector to world-space sample vector
    vec3 up        = abs(v.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, v));
    vec3 bitangent = cross(v, tangent);

    vec3 sampleVec = tangent * H.x + bitangent * H.y + v * H.z;
    return normalize(sampleVec);
}

// Input Ve: view direction
// Input alpha_x, alpha_y: roughness parameters
// Input U1, U2: uniform random numbers
// Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
vec3 SampleGGXVNDF_Heitz2018(vec3 v, float alpha_x, float alpha_y, float U1, float U2)
{
    // paper: Heitz2018GGX

    // Section 3.2: transforming the view direction to the hemisphere configuration
    vec3 Vh = normalize(vec3(alpha_x * v.x, alpha_y * v.y, v.z));

    // Section 4.1: orthonormal basis (with special case if cross product is zero)
    float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
    vec3 T1 = lensq > 0 ? vec3(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : vec3(1,0,0);
    vec3 T2 = cross(Vh, T1);

    // Section 4.2: parameterization of the projected area
    float r = sqrt(U1);
    float phi = 2.0 * M_PI * U2;
    float t1 = r * cos(phi);
    float t2 = r * sin(phi);
    float s = 0.5 * (1.0 + Vh.z);
    t2 = (1.0 - s)*sqrt(1.0 - t1*t1) + s*t2;

    // Section 4.3: reprojection onto hemisphere
    vec3 Nh = t1*T1 + t2*T2 + sqrt(max(0.0, 1.0 - t1*t1 - t2*t2))*Vh;

    // Section 3.4: transforming the normal back to the ellipsoid configuration
    vec3 Ne = normalize(vec3(alpha_x * Nh.x, alpha_y * Nh.y, max(0.0, Nh.z)));
    return Ne;
}

// requires GLSL 400+
// Sampling the visible hemisphere as half vectors (our method)
vec3 SampleVndf_Hemisphere(vec2 u, vec3 wi)
{
    // sample a spherical cap in (-wi.z, 1]
    float phi = 2.0f * M_PI * u.x;
    float z = fma((1.0f - u.y), (1.0f + wi.z), -wi.z);
    float sinTheta = sqrt(clamp(1.0f - z * z, 0.0f, 1.0f));
    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    vec3 c = vec3(x, y, z);
    // compute halfway direction;
    vec3 h = c + wi;
    // return without normalization (as this is done later)
    // return h;
    
    return normalize(h);
}

// Input Ve: view direction
// Input alpha_x, alpha_y: roughness parameters
// Input U1, U2: uniform random numbers
// Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
vec3 SampleGGXVNDF_Intel2023(vec3 v, float alpha_x, float alpha_y, float U1, float U2)
{
    // paper: arXiv:2306.05044

    vec2 alpha = vec2(alpha_x, alpha_y);

    // warp to the hemisphere configuration
    vec3 wiStd = normalize(vec3(v.xy * alpha, v.z));
    // sample the hemisphere (see implementation 2 or 3)
    vec3 wmStd = SampleVndf_Hemisphere(vec2(U1, U2), wiStd);
    // warp back to the ellipsoid configuration
    vec3 wm = normalize(vec3(wmStd.xy * alpha, wmStd.z));
    // return final normal
    return wm;
}

#endif // INCLUDE_GUARD_GLSL_GGX