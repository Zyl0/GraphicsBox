#pragma once

#include "Math/RMath.h"
#include "MathSimt/RMath.h"

namespace Rendering
{
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Math::Simt::Scalar<DataType, ThreadCount> Activation_Heitz2014(const Math::Simt::Scalar<DataType, ThreadCount>& In)
    {
        return Select(Math::Simt::Scalar<DataType, ThreadCount>(0), Math::Simt::Scalar<DataType, ThreadCount>(1), In < DataType(0));
    }
    
    float D_GGX_Heitz2014_EQ71(Math::Vector3f h, Math::Vector3f n, float alpha);
    
    float D_GGX_Heitz2014_EQ71_Simplified(const Math::Vector3f& h, const Math::Vector3f& n, float alpha);
    
    float Lambda_Heitz2014_EQ72(const Math::Vector3f& n, const Math::Vector3f& w, float alpha2);
    
    float G1_Heitz2014_EQ98(const Math::Vector3f& h, const Math::Vector3f& n, const Math::Vector3f& v, float alpha);
    
    float G2_Heitz2014_EQ99(const Math::Vector3f& h, const Math::Vector3f& n, const Math::Vector3f& v, const Math::Vector3f& l, float alpha);
    
    Math::Vector3f SampleGGX(const Math::Vector3f& v, float alpha_x, float alpha_y, float U1, float U2);
    
    Math::Vector3f SampleGGXVNDF_Heitz2018(const Math::Vector3f& v, float alpha_x, float alpha_y, float U1, float U2);
    
    Math::Vector3f SampleVndf_Hemisphere(const Math::Vector2f& u, const Math::Vector3f& wi);
    
    Math::Vector3f SampleGGXVNDF_Intel2023(const Math::Vector3f& v, float alpha_x, float alpha_y, float U1, float U2);
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Scalar<DataType, ThreadCount> D_GGX_Heitz2014_EQ71(
        const Math::Simt::Vector3<DataType, ThreadCount>& h,
        const Math::Simt::Vector3<DataType, ThreadCount>& n,
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha
        )
    {
        Math::Simt::Scalar<DataType, ThreadCount> Cos2Theta = Math::Simt::Dot(h, n) * Math::Simt::Dot(h, n);
        Math::Simt::Scalar<DataType, ThreadCount> Tan2Theta = (DataType(1) / Cos2Theta) - DataType(1);
        Math::Simt::Scalar<DataType, ThreadCount> alpha2 = alpha * alpha;

        //todo name
        Math::Simt::Scalar<DataType, ThreadCount> t = ( DataType(1) + ( Tan2Theta / alpha2 ) );

        Math::Simt::Scalar<DataType, ThreadCount> numerator = Activation_Heitz2014(Math::Simt::Dot(h, n));
        Math::Simt::Scalar<DataType, ThreadCount> denominator = DataType(M_PI) * alpha2 * Cos2Theta * Cos2Theta * t * t;

        return numerator / denominator;
    }
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Scalar<DataType, ThreadCount> D_GGX_Heitz2014_EQ71_Simplified(
        const Math::Simt::Vector3<DataType, ThreadCount>& h,
        const Math::Simt::Vector3<DataType, ThreadCount>& n,
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha
        )
    {
        Math::Simt::Scalar<DataType, ThreadCount> a2     = alpha*alpha;
        Math::Simt::Scalar<DataType, ThreadCount> NdotH  = Math::Simt::Max(Math::Simt::Dot(n, h), Math::Simt::Scalar<DataType, ThreadCount>(0.0));
        Math::Simt::Scalar<DataType, ThreadCount> NdotH2 = NdotH*NdotH;

        Math::Simt::Scalar<DataType, ThreadCount> num   = a2;
        Math::Simt::Scalar<DataType, ThreadCount> denom = (NdotH2 * (a2 - DataType(1.0)) + DataType(1.0));
        denom = DataType(M_PI) * denom * denom;

        return num / denom;
    }
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Scalar<DataType, ThreadCount> Lambda_Heitz2014_EQ72(
        const Math::Simt::Vector3<DataType, ThreadCount>& n,
        const Math::Simt::Vector3<DataType, ThreadCount>& w,
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha2
        )
    {
        Math::Simt::Vector3<DataType, ThreadCount> wNormalized = Normalize(w);
        Math::Simt::Scalar<DataType, ThreadCount> Cos2Theta = Math::Simt::Dot(wNormalized, n) * Math::Simt::Dot(wNormalized, n);
        Math::Simt::Scalar<DataType, ThreadCount> Tan2Theta = (DataType(1) / Cos2Theta) - DataType(1);

        return DataType(-1.0/2.0) + DataType(1.0/2.0) * Math::Simt::Sqrt(DataType(1.0) + (alpha2 * Tan2Theta));
    }
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Scalar<DataType, ThreadCount> G1_Heitz2014_EQ98(
        const Math::Simt::Vector3<DataType, ThreadCount>& h, 
        const Math::Simt::Vector3<DataType, ThreadCount>& n, 
        const Math::Simt::Vector3<DataType, ThreadCount>& v, 
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha
        )
    {
        Math::Simt::Scalar<DataType, ThreadCount> vdoth = Math::Simt::Dot(v, h);
        Math::Simt::Scalar<DataType, ThreadCount> alpha2 = alpha * alpha;

        Math::Simt::Scalar<DataType, ThreadCount> numerator = Activation_Heitz2014(vdoth);
        Math::Simt::Scalar<DataType, ThreadCount> denominator = ThreadCount(1) + Lambda_Heitz2014_EQ72(n, v, alpha2);

        return numerator / denominator;
    }
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Scalar<DataType, ThreadCount> G2_Heitz2014_EQ99(
        const Math::Simt::Vector3<DataType, ThreadCount>& h, 
        const Math::Simt::Vector3<DataType, ThreadCount>& n, 
        const Math::Simt::Vector3<DataType, ThreadCount>& v, 
        const Math::Simt::Vector3<DataType, ThreadCount>& l, 
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha
        )
    {
        Math::Simt::Scalar<DataType, ThreadCount> vdoth = Math::Simt::Dot(v, h);
        Math::Simt::Scalar<DataType, ThreadCount> ldoth = Math::Simt::Dot(l, h);
        Math::Simt::Scalar<DataType, ThreadCount> alpha2 = alpha * alpha;

        Math::Simt::Scalar<DataType, ThreadCount> numerator = Activation_Heitz2014(vdoth) * Activation_Heitz2014(ldoth);
        Math::Simt::Scalar<DataType, ThreadCount> denominator = DataType(1) + Lambda_Heitz2014_EQ72(n, v, alpha2) + Lambda_Heitz2014_EQ72(n, l, alpha2);

        return numerator / denominator;
    }
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector3<DataType, ThreadCount> SampleGGX(
        const Math::Simt::Vector3<DataType, ThreadCount>& v, 
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha_x, 
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha_y, 
        const Math::Simt::Scalar<DataType, ThreadCount>& U1, 
        const Math::Simt::Scalar<DataType, ThreadCount>& U2
        )
    {
        //float cosTheta = U1;
        //float sinTheta = sqrt(1 - U1*U1);
        //float phi = 2 * PI * U2;

        //vec3 l = vec3( cos(phi) * sinTheta, cosTheta, sinTheta * sin(phi) );

        Math::Simt::Scalar<DataType, ThreadCount> phi = DataType(2.0 * M_PI) * U2;
        Math::Simt::Scalar<DataType, ThreadCount> cosTheta = Math::Simt::Sqrt( (DataType(1.0) - U1) / (DataType(1.0) + (alpha_x * alpha_y - DataType(1.0)) * U2) );
        Math::Simt::Scalar<DataType, ThreadCount> sinTheta = Math::Simt::Sqrt( DataType(1.0) - cosTheta * cosTheta );
        // todo use alpha_y

        // from spherical coordinates to cartesian coordinates
        Math::Simt::Vector3<DataType, ThreadCount> H;
        H.x = Math::Simt::Cos(phi) * sinTheta;
        H.y = Math::Simt::Sin(phi) * sinTheta;
        H.z = cosTheta;

        // from tangent-space vector to world-space sample vector
        Math::Simt::Vector3<DataType, ThreadCount> up        = Math::Simt::Select(Math::Simt::Vector3<DataType, ThreadCount>(0.0, 0.0, 1.0), Math::Simt::Vector3<DataType, ThreadCount>(1.0, 0.0, 0.0), abs(v.z) < DataType(0.999));
        Math::Simt::Vector3<DataType, ThreadCount> tangent   = Math::Simt::Normalize(cross(up, v));
        Math::Simt::Vector3<DataType, ThreadCount> bitangent = Math::Simt::Cross(v, tangent);

        Math::Simt::Vector3<DataType, ThreadCount> sampleVec = tangent * H.x + bitangent * H.y + v * H.z;
        return Math::Simt::Normalize(sampleVec);
    }
    
    // Input Ve: view direction
    // Input alpha_x, alpha_y: roughness parameters
    // Input U1, U2: uniform random numbers
    // Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector3<DataType, ThreadCount> SampleGGXVNDF_Heitz2018(
        const Math::Simt::Vector3<DataType, ThreadCount>& v, 
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha_x, 
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha_y, 
        const Math::Simt::Scalar<DataType, ThreadCount>& U1, 
        const Math::Simt::Scalar<DataType, ThreadCount>& U2
        )
    {
        // Section 3.2: transforming the view direction to the hemisphere configuration
        Math::Simt::Vector3<DataType, ThreadCount> Vh = normalize(vec3(alpha_x * v.x, alpha_y * v.y, v.z));

        // Section 4.1: orthonormal basis (with special case if cross product is zero)
        Math::Simt::Scalar<DataType, ThreadCount> lensq = Vh.x * Vh.x + Vh.y * Vh.y;
        Math::Simt::Vector3<DataType, ThreadCount> T1 = Math::Simt::Select(
            Math::Simt::Vector3<DataType, ThreadCount>(-Vh.y, Vh.x, 0) * Math::Simt::InverseSqrt(lensq), 
            Math::Simt::Vector3<DataType, ThreadCount>(1,0,0), 
            lensq > 0);
        Math::Simt::Vector3<DataType, ThreadCount> T2 = Math::Simt::Cross(Vh, T1);

        // Section 4.2: parameterization of the projected area
        Math::Simt::Scalar<DataType, ThreadCount> r = Math::Simt::Sqrt(U1);
        Math::Simt::Scalar<DataType, ThreadCount> phi = DataType(2.0 * M_PI) * U2;
        Math::Simt::Scalar<DataType, ThreadCount> t1 = r * Math::Simt::Cos(phi);
        Math::Simt::Scalar<DataType, ThreadCount> t2 = r * Math::Simt::Sin(phi);
        Math::Simt::Scalar<DataType, ThreadCount> s = 0.5 * (1.0 + Vh.z);
        t2 = (DataType(1.0) - s) * Math::Simt::Sqrt(DataType(1.0) - t1 * t1) + s * t2;

        // Section 4.3: reprojection onto hemisphere
        Math::Simt::Vector3<DataType, ThreadCount> Nh = t1*T1 + t2*T2 + Math::Simt::Sqrt(Max(DataType(0.0), DataType(1.0) - t1 * t1 - t2 * t2)) * Vh;

        // Section 3.4: transforming the normal back to the ellipsoid configuration
        // Math::Simt::Vector3<DataType, ThreadCount> Ne = Normalize(Math::Simt::Vector3<DataType, ThreadCount>(alpha_x * Nh.x, alpha_y * Nh.y, Max(0.0, Nh.z)));
        // return Ne;
        return Math::Simt::Normalize(Math::Simt::Vector3<DataType, ThreadCount>(alpha_x * Nh.x, alpha_y * Nh.y, Max(0.0, Nh.z)));
    }
    
    // Sampling the visible hemisphere as half vectors (our method)
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Vector3f SampleVndf_Hemisphere(
        const Math::Simt::Scalar<DataType, ThreadCount>& U1, 
        const Math::Simt::Scalar<DataType, ThreadCount>& U2,
        const Math::Simt::Vector3<DataType, ThreadCount>& wi
        )
    {
        // sample a spherical cap in (-wi.z, 1]
        Math::Simt::Scalar<DataType, ThreadCount> phi = DataType(2.0f * M_PI) * U1;
        Math::Simt::Scalar<DataType, ThreadCount> z = Math::Simt::Mul_Add((DataType(1.0) - U2), (DataType(1.0) + wi.z), -wi.z);
        Math::Simt::Scalar<DataType, ThreadCount> sinTheta = Math::Simt::Sqrt(Math::Simt::Clamp(DataType(1.0) - z * z, DataType(0.0), DataType(1.0)));
        Math::Simt::Scalar<DataType, ThreadCount> x = sinTheta * Math::Simt::Cos(phi);
        Math::Simt::Scalar<DataType, ThreadCount> y = sinTheta * Math::Simt::Sin(phi);
        Math::Simt::Vector3<DataType, ThreadCount> c = Math::Simt::Vector3<DataType, ThreadCount>(x, y, z);
        // compute halfway direction;
        Math::Simt::Vector3<DataType, ThreadCount> h = c + wi;
        // return without normalization (as this is done later)
        // return h;
    
        return Normalize(h);
    }
    
    // Input Ve: view direction
    // Input alpha_x, alpha_y: roughness parameters
    // Input U1, U2: uniform random numbers
    // Output Ne: normal sampled with PDF D_Ve(Ne) = G1(Ve) * max(0, dot(Ve, Ne)) * D(Ne) / Ve.z
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Vector3f SampleGGXVNDF_Intel2023(
        const Math::Simt::Vector3<DataType, ThreadCount>& v, 
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha_x, 
        const Math::Simt::Scalar<DataType, ThreadCount>& alpha_y, 
        const Math::Simt::Scalar<DataType, ThreadCount>& U1, 
        const Math::Simt::Scalar<DataType, ThreadCount>& U2
        )
    {
        // paper: arXiv:2306.05044

        // warp to the hemisphere configuration
        Math::Simt::Vector3<DataType, ThreadCount> wiStd = Math::Simt::Normalize(Math::Simt::Vector3<DataType, ThreadCount>(v.x * alpha_x, v.y * alpha_y, v.z));
        // sample the hemisphere (see implementation 2 or 3)
        Math::Simt::Vector3<DataType, ThreadCount> wmStd = SampleVndf_Hemisphere(U1, U2, wiStd);
        // warp back to the ellipsoid configuration
        Math::Simt::Vector3<DataType, ThreadCount> wm = Math::Simt::Normalize(Math::Simt::Vector3<DataType, ThreadCount>(wmStd.x * alpha_x, wmStd.y * alpha_y, wmStd.z));
        // return final normal
        return wm;
    }
}