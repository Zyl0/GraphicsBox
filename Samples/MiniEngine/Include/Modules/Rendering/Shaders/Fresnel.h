#pragma once

#include "Math/RMath.h"
#include "MathSimt/RMath.h"

namespace Rendering
{
    float FresnelG(float NdotL, float Refraction);

    Math::Vector2f FresnelG(float NdotL, Math::Vector2f Refraction);

    Math::Vector3f FresnelG(float NdotL, Math::Vector3f Refraction);

    Math::Vector4f FresnelG(float NdotL, Math::Vector4f Refraction);

    float Fresnel(float NdotL, float F0);

    Math::Vector2f Fresnel(float NdotL, Math::Vector2f F0);

    Math::Vector3f Fresnel(float NdotL, Math::Vector3f F0);

    Math::Vector4f Fresnel(float NdotL, Math::Vector4f F0);
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
     Math::Simt::Scalar<DataType, ThreadCount> FresnelG(
         const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
         const Math::Simt::Scalar<DataType, ThreadCount>& Refraction
        )
    {
        return Math::Simt::Sqrt(Refraction * Refraction + NdotL * NdotL - 1);
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Vector2f FresnelG(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector2<DataType, ThreadCount>& Refraction
        )
    {
        return Math::Simt::Sqrt(Refraction * Refraction + NdotL * NdotL - DataType(1));
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Vector3f FresnelG(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector3<DataType, ThreadCount>& Refraction
        )
    {
        return Math::Simt::Sqrt(Refraction * Refraction + NdotL * NdotL - DataType(1));
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Vector4f FresnelG(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector4<DataType, ThreadCount>& Refraction
        )
    {
        return Math::Simt::Sqrt(Refraction * Refraction + NdotL * NdotL - DataType(1));
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Scalar<DataType, ThreadCount> Fresnel(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Scalar<DataType, ThreadCount>& F0
        )
    {
        Math::Simt::Scalar<DataType, ThreadCount> sqrtF0 = Math::Simt::Sqrt(F0);
        Math::Simt::Scalar<DataType, ThreadCount> Refraction = (DataType(1) + sqrtF0) / (DataType(1) - sqrtF0);

        Math::Simt::Scalar<DataType, ThreadCount> G = FresnelG(NdotL, Refraction);
        Math::Simt::Scalar<DataType, ThreadCount> C = NdotL;

        Math::Simt::Scalar<DataType, ThreadCount> PartA = (G - C) / (G + C);
        Math::Simt::Scalar<DataType, ThreadCount> PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

        return DataType(1. / 2) * (PartA * PartA) * (DataType(1) + (PartB * PartB));
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector2<DataType, ThreadCount> Fresnel(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector2<DataType, ThreadCount>& F0
        )
    {
        Math::Simt::Vector2<DataType, ThreadCount> sqrtF0 = Math::Simt::Sqrt(F0);
        Math::Simt::Vector2<DataType, ThreadCount> Refraction = (DataType(1) + sqrtF0) / (DataType(1) - sqrtF0);

        Math::Simt::Vector2<DataType, ThreadCount> G = FresnelG(NdotL, Refraction);
        Math::Simt::Vector2<DataType, ThreadCount> C = NdotL;

        Math::Simt::Vector2<DataType, ThreadCount> PartA = (G - C) / (G + C);
        Math::Simt::Vector2<DataType, ThreadCount> PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

        return DataType(1. / 2) * (PartA * PartA) * (DataType(1) + (PartB * PartB));
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector3<DataType, ThreadCount> Fresnel(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector3<DataType, ThreadCount>& F0
        )
    {
        Math::Simt::Vector3<DataType, ThreadCount> sqrtF0 = Math::Simt::Sqrt(F0);
        Math::Simt::Vector3<DataType, ThreadCount> Refraction = (DataType(1) + sqrtF0) / (DataType(1) - sqrtF0);

        Math::Simt::Vector3<DataType, ThreadCount> G = FresnelG(NdotL, Refraction);
        Math::Simt::Vector3<DataType, ThreadCount> C = NdotL;

        Math::Simt::Vector3<DataType, ThreadCount> PartA = (G - C) / (G + C);
        Math::Simt::Vector3<DataType, ThreadCount> PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

        return DataType(1. / 2) * (PartA * PartA) * (DataType(1) + (PartB * PartB));
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector4<DataType, ThreadCount> Fresnel(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector4<DataType, ThreadCount>& F0
        )
    {
        Math::Simt::Vector4<DataType, ThreadCount> sqrtF0 = Math::Simt::Sqrt(F0);
        Math::Simt::Vector4<DataType, ThreadCount> Refraction = (DataType(1) + sqrtF0) / (DataType(1) - sqrtF0);

        Math::Simt::Vector4<DataType, ThreadCount> G = FresnelG(NdotL, Refraction);
        Math::Simt::Vector4<DataType, ThreadCount> C = NdotL;

        Math::Simt::Vector4<DataType, ThreadCount> PartA = (G - C) / (G + C);
        Math::Simt::Vector4<DataType, ThreadCount> PartB = (C * (G + C) - 1) / (C * (G + C) + 1);

        return DataType(1. / 2) * (PartA * PartA) * (DataType(1) + (PartB * PartB));
    }
}
