#pragma once

#include "Types.h"
#include "Vector.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Plane
    {
        using Type = DataType;
        using ScalarType =  Scalar<DataType, ThreadCount>;
        using MaskType = typename Scalar<DataType, ThreadCount>::MaskType;
        using IndexerType = typename Scalar<DataType, ThreadCount>::IndexerType;

        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;
        static constexpr size_t kComponentCount = 4;

        static consteval size_t Size() {return ThreadCount;}
        
        ScalarType x, y, z, w;

        Plane(Type nx, Type ny, Type nz, Type d) : x(nx), y(ny), z(nz), w(d) {}
        Plane(const ScalarType& nx, const ScalarType& ny, const ScalarType& nz, const ScalarType& d) : x(nx), y(ny), z(nz), w(d) {}

        Plane(const Vector3t<DataType> &n, Type d) : x(n.x), y(n.y), z(n.z), w(d) {}
        Plane(const Vector3t<DataType> &n, const ScalarType& d) : x(n.x), y(n.y), z(n.z), w(d) {}
        Plane(const Vector3<DataType, ThreadCount> &n, Type d) : x(n.x), y(n.y), z(n.z), w(d) {}
        Plane(const Vector3<DataType, ThreadCount> &n, const ScalarType& d) : x(n.x), y(n.y), z(n.z), w(d) {}

        INLINE const Vector3<DataType, ThreadCount> &GetNormal() const
        {
            return (reinterpret_cast<const Vector3<DataType, ThreadCount> &>(x));
        }
    };

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Plane<DataType, ThreadCount> Select(
        const Plane<DataType, ThreadCount>& A,
        const Plane<DataType, ThreadCount>& B,
        const typename Plane<DataType, ThreadCount>::MaskType& mask)
    {
        Plane<DataType, ThreadCount> r;
        r.x = Select(A.x, B.x, mask);
        r.y = Select(A.y, B.y, mask);
        r.z = Select(A.z, B.z, mask);
        r.w = Select(A.w, B.w, mask);
        return r;
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Plane<DataType, ThreadCount>::ScalarType Dot(const Plane<DataType, ThreadCount> &f, const Vector3<DataType, ThreadCount> &v)
    {
        return (f.x * v.x + f.y * v.y + f.z * v.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Plane<DataType, ThreadCount>::ScalarType Dot(const Plane<DataType, ThreadCount> &f, const Point3<DataType, ThreadCount> &p)
    {
        return (f.x * p.x + f.y * p.y + f.z * p.z);
    }
}