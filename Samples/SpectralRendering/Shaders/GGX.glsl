#ifndef INCLUDE_GUARD_GLSL_GGX
#define INCLUDE_GUARD_GLSL_GGX

#include "Include/Math.glsl"

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
    float NdotH  = max(dot(h, h), 0.0);
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

    return (-1/2) + (1/2) * sqrt(1 + (alpha2 * Tan2Theta));
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

#endif // INCLUDE_GUARD_GLSL_GGX