#include "Modules/Rendering/Shaders/GGX.h"

#include <numbers>

using namespace Math;

namespace Rendering
{
    
#define Activation(x) ((x) < 0 ? 0 : 1)

    float D_GGX_Heitz2014_EQ71(Vector3f h, Vector3f n, float alpha)
    {
        float Cos2Theta = Dot(h, n) * Dot(h, n);
        float Tan2Theta = (1.f / Cos2Theta) - 1.f;
        float alpha2 = alpha * alpha;

        //todo name
        float t = ( 1 + ( Tan2Theta / alpha2 ) );

        float numerator = Activation(Dot(h, n));
        float denominator = static_cast<float>(M_PI) * alpha2 * Cos2Theta * Cos2Theta * t * t;

        return numerator / denominator;
    }
    
    float D_GGX_Heitz2014_EQ71_Simplified(const Vector3f& h, const Vector3f& n, float alpha)
    {
        float a2     = alpha*alpha;
        float NdotH  = std::max(Dot(n, h), 0.0f);
        float NdotH2 = NdotH*NdotH;

        float num   = a2;
        float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
        denom = static_cast<float>(M_PI) * denom * denom;

        return num / denom;
    }
    
    float Lambda_Heitz2014_EQ72(const Vector3f& n, const Vector3f& w, float alpha2)
    {
        Vector3f wNormalized = Normalize(w);
        float Cos2Theta = Dot(wNormalized, n) * Dot(wNormalized, n);
        float Tan2Theta = (1.f / Cos2Theta) - 1.f;

        return (-1.0f/2.0f) + (1.0f/2.0f) * sqrt(1.0f + (alpha2 * Tan2Theta));
    }

    float G1_Heitz2014_EQ98(const Vector3f& h, const Vector3f& n, const Vector3f& v, float alpha)
    {
        float vdoth = Dot(v, h);
        float alpha2 = alpha * alpha;

        float numerator = Activation(vdoth);
        float denominator = 1 + Lambda_Heitz2014_EQ72(n, v, alpha2);

        return numerator / denominator;
    }

    float G2_Heitz2014_EQ99(const Vector3f& h, const Vector3f& n, const Vector3f& v, const Vector3f& l, float alpha)
    {
        float vdoth = Dot(v, h);
        float ldoth = Dot(l, h);
        float alpha2 = alpha * alpha;

        float numerator = Activation(vdoth) * Activation(ldoth);
        float denominator = 1 + Lambda_Heitz2014_EQ72(n, v, alpha2) + Lambda_Heitz2014_EQ72(n, l, alpha2);

        return numerator / denominator;
    }
    
    
    Vector3f SampleGGX(const Vector3f& v, float alpha_x, float alpha_y, float U1, float U2)
    {
        //float cosTheta = U1;
        //float sinTheta = sqrt(1 - U1*U1);
        //float phi = 2 * PI * U2;

        //vec3 l = vec3( cos(phi) * sinTheta, cosTheta, sinTheta * sin(phi) );

        float phi = 2.0f * std::numbers::pi_v<float> * U2;
        float cosTheta = sqrt( (1.0f - U1) / (1.0f + (alpha_x * alpha_y - 1.0f) * U2) );
        float sinTheta = sqrt( 1.0f - cosTheta * cosTheta );

        // from spherical coordinates to cartesian coordinates
        Vector3f H;
        H.x = cos(phi) * sinTheta;
        H.y = sin(phi) * sinTheta;
        H.z = cosTheta;

        // from tangent-space vector to world-space sample vector
        Vector3f up        = abs(v.z) < 0.999 ? Vector3f(0.0, 0.0, 1.0) : Vector3f(1.0, 0.0, 0.0);
        Vector3f tangent   = Normalize(Cross(up, v));
        Vector3f bitangent = Cross(v, tangent);

        Vector3f sampleVec = tangent * H.x + bitangent * H.y + v * H.z;
        return Normalize(sampleVec);
    }
    
    INLINE float inversesqrt(float in)
    {
        // TODO maybe use fast inverse square root algorithm or find hardware accelerated instruction
        return 1.f / sqrt(in);
    }

    // Input Ve: view direction
    // Input alpha_x, alpha_y: roughness parameters
    // Input U1, U2: uniform random numbers
    // Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
    Vector3f SampleGGXVNDF_Heitz2018(const Vector3f& v, float alpha_x, float alpha_y, float U1, float U2)
    {
        // paper: Heitz2018GGX

        // Section 3.2: transforming the view direction to the hemisphere configuration
        Vector3f Vh = Normalize(Vector3f(alpha_x * v.x, alpha_y * v.y, v.z));

        // Section 4.1: orthonormal basis (with special case if cross product is zero)
        float lensq = Vh.x * Vh.x + Vh.y * Vh.y;
        Vector3f T1 = lensq > 0 ? Vector3f(-Vh.y, Vh.x, 0) * inversesqrt(lensq) : Vector3f(1,0,0);
        Vector3f T2 = Cross(Vh, T1);

        // Section 4.2: parameterization of the projected area
        float r = sqrt(U1);
        float phi = 2.0f * std::numbers::pi_v<float> * U2;
        float t1 = r * cos(phi);
        float t2 = r * sin(phi);
        float s = 0.5f * (1.0f + Vh.z);
        t2 = (1.0f - s)*sqrt(1.0f - t1*t1) + s*t2;

        // Section 4.3: reprojection onto hemisphere
        Vector3f Nh = t1*T1 + t2*T2 + sqrt(std::max(0.0f, 1.0f - t1*t1 - t2*t2))*Vh;

        // Section 3.4: transforming the normal back to the ellipsoid configuration
        Vector3f Ne = Normalize(Vector3f(alpha_x * Nh.x, alpha_y * Nh.y, std::max(0.0f, Nh.z)));
        return Ne;
    }

    // Sampling the visible hemisphere as half vectors (our method)
    Vector3f SampleVndf_Hemisphere(const Vector2f& u, const Vector3f& wi)
    {
        // sample a spherical cap in (-wi.z, 1]
        float phi = 2.0f * std::numbers::pi_v<float> * u.x;
        float z = fma((1.0f - u.y), (1.0f + wi.z), -wi.z);
        float sinTheta = sqrt(std::clamp(1.0f - z * z, 0.0f, 1.0f));
        float x = sinTheta * cos(phi);
        float y = sinTheta * sin(phi);
        Vector3f c = Vector3f(x, y, z);
        // compute halfway direction;
        Vector3f h = c + wi;
        // return without normalization (as this is done later)
        // return h;
        
        return Normalize(h);
    }

    // Input Ve: view direction
    // Input alpha_x, alpha_y: roughness parameters
    // Input U1, U2: uniform random numbers
    // Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
    Vector3f SampleGGXVNDF_Intel2023(const Vector3f& v, float alpha_x, float alpha_y, float U1, float U2)
    {
        // paper: arXiv:2306.05044
        
        // TODO introduce v.xy v.xx, etc. math header generation

        // warp to the hemisphere configuration
        Vector3f wiStd = Normalize(Vector3f(v.x * alpha_x, v.y * alpha_y, v.z));
        // sample the hemisphere (see implementation 2 or 3)
        Vector3f wmStd = SampleVndf_Hemisphere(Vector2f(U1, U2), wiStd);
        // warp back to the ellipsoid configuration
        Vector3f wm = Normalize(Vector3f(wmStd.x * alpha_x, wmStd.y * alpha_y, wmStd.z));
        // return final normal
        return wm;
    }
}