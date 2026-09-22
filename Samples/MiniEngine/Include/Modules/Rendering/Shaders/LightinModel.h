#pragma once

#include "Math/RMath.h"
#include "MathSimt/RMath.h"

namespace Rendering
{
    Math::Vector3f fDielectrical( const Math::Vector3f& DiffuseColor, float Normalisation, const Math::Vector3f& F );

    Math::Vector3f fDielectricalIndirect( const Math::Vector3f& DiffuseColor, float Normalisation, const Math::Vector3f& F );
    
    Math::Vector3f fMetallic(float Normalisation, const Math::Vector3f& F );

    Math::Vector3f fMetallicIndirect( const Math::Vector3f& DiffuseColor, float Normalisation, const Math::Vector3f& F );
    
    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector3<DataType, ThreadCount> fDielectrical(
        const Math::Simt::Vector3<DataType, ThreadCount>& DiffuseColor, 
        const Math::Simt::Scalar<DataType, ThreadCount>& Normalisation, 
        const Math::Simt::Vector3<DataType, ThreadCount>& F
        )
    {
        return ((DataType(1) - F) * DiffuseColor / DataType(M_PI)) + (F * Normalisation);
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector3<DataType, ThreadCount> fDielectricalIndirect(
        const Math::Simt::Vector3<DataType, ThreadCount>& DiffuseColor, 
        const Math::Simt::Scalar<DataType, ThreadCount>& Normalisation, 
        const Math::Simt::Vector3<DataType, ThreadCount>& F
        )
    {
        return F * Normalisation * DiffuseColor;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector3<DataType, ThreadCount> fMetallic(
        float Normalisation, 
        const Math::Simt::Vector3<DataType, ThreadCount>& F
        )
    {
        return ((DataType(1) - F) * Math::Simt::Vector3<DataType, ThreadCount>(0)) + (F * Normalisation);
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Math::Simt::Vector3<DataType, ThreadCount> fMetallicIndirect(
        const Math::Simt::Vector3<DataType, ThreadCount>& DiffuseColor, 
        const Math::Simt::Scalar<DataType, ThreadCount>& Normalisation, 
        const Math::Simt::Vector3<DataType, ThreadCount>& F
        )
    {
        return F * Normalisation * Math::Vector3f(1);
    }
}
