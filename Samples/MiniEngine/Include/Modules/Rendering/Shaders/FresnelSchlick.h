#pragma once

#include "Math/RMath.h"
#include "MathSimt/RMath.h"

namespace Rendering
{
    float FresnelSchlick(float NdotL, float F0);

    Math::Vector2f FresnelSchlick(float NdotL, const Math::Vector2f& F0);

    Math::Vector3f FresnelSchlick(float NdotL, const Math::Vector3f& F0);

    Math::Vector4f FresnelSchlick(float NdotL, const Math::Vector4f& F0);
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Scalar<DataType, ThreadCount> FresnelSchlick(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Scalar<DataType, ThreadCount>& F0
        )
    {
        // return F0 + (DataType(1) - F0) * Math::Simt::Pow(DataType(1) - Math::Simt::Clamp(NdotL, DataType(0), DataType(1)), DataType(5));
        
        Math::Simt::Scalar<DataType, ThreadCount> Weight = DataType(1) - Math::Simt::Clamp(NdotL, DataType(0), DataType(1));
        return F0 + (DataType(1) - F0) * /* pow(Weight, 5) */ Weight * Weight * Weight * Weight * Weight;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector2<DataType, ThreadCount> FresnelSchlick(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector2<DataType, ThreadCount>& F0
        )
    {
        // return F0 + (DataType(1) - F0) * Math::Simt::Pow(DataType(1) - Math::Simt::Clamp(NdotL, DataType(0), DataType(1)), DataType(5));
        
        Math::Simt::Scalar<DataType, ThreadCount> Weight = DataType(1) - Math::Simt::Clamp(NdotL, DataType(0), DataType(1));
        return F0 + (DataType(1) - F0) * /* pow(Weight, 5) */ Weight * Weight * Weight * Weight * Weight;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector3<DataType, ThreadCount> FresnelSchlick(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector3<DataType, ThreadCount>& F0
        )
    {
        // return F0 + (DataType(1) - F0) * Math::Simt::Pow(DataType(1) - Math::Simt::Clamp(NdotL, DataType(0), DataType(1)), DataType(5));
        
        Math::Simt::Scalar<DataType, ThreadCount> Weight = DataType(1) - Math::Simt::Clamp(NdotL, DataType(0), DataType(1));
        return F0 + (DataType(1) - F0) * /* pow(Weight, 5) */ Weight * Weight * Weight * Weight * Weight;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector4<DataType, ThreadCount> FresnelSchlick(
        const Math::Simt::Scalar<DataType, ThreadCount>& NdotL, 
        const Math::Simt::Vector4<DataType, ThreadCount>& F0
        )
    {
        // return F0 + (DataType(1) - F0) * Math::Simt::Pow(DataType(1) - Math::Simt::Clamp(NdotL, DataType(0), DataType(1)), DataType(5));
        
        Math::Simt::Scalar<DataType, ThreadCount> Weight = DataType(1) - Math::Simt::Clamp(NdotL, DataType(0), DataType(1));
        return F0 + (DataType(1) - F0) * /* pow(Weight, 5) */ Weight * Weight * Weight * Weight * Weight;
    }
}
