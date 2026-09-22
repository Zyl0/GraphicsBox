#pragma once

#include "Types.h"
#include "Vector.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Line
    {
        using Type = DataType;
        using ScalarType =  Scalar<DataType, ThreadCount>;
        using MaskType = typename Scalar<DataType, ThreadCount>::MaskType;
        using IndexerType = typename Scalar<DataType, ThreadCount>::IndexerType;

        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;
        static constexpr size_t kComponentCount = 4;

        static consteval size_t Size() {return ThreadCount;}
        
        Vector3<DataType, ThreadCount> direction;
        Vector3<DataType, ThreadCount> moment;

        Line() = default;

        Line(const ScalarType& vx, const ScalarType& vy, const ScalarType& vz, const ScalarType& mx, const ScalarType& my, const ScalarType& mz) :
            direction(vx, vy, vz), moment(mx, my, mz)
        {}

        Line(const Vector3<DataType, ThreadCount> &v, const Vector3<DataType, ThreadCount> &m) :
            direction(v), moment(m)
        {}
    };
}