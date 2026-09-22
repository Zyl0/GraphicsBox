#pragma once

#include "Types.h"
#include "Functions.h"
#include "Vector.h"
#include "Plane.h"
#include <type_traits>

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> DistancePointLine(const Point3<DataType, ThreadCount> &q, const Point3<DataType, ThreadCount> &p, const Vector3<DataType, ThreadCount> &v)
    {
        Vector3<DataType, ThreadCount> a = Cross(q - p, v);
        return Sqrt(SquareMagnitude(a) / SquareMagnitude(v));
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> DistanceLineLine(const Point3<DataType, ThreadCount> &p1, const Vector3<DataType, ThreadCount> &v1, const Point3<DataType, ThreadCount> &p2, const Vector3<DataType, ThreadCount> &v2)
    {
        Vector3<DataType, ThreadCount> dp = p2 - p1;

        Scalar<DataType, ThreadCount> v12 = SquareMagnitude(v1);
        Scalar<DataType, ThreadCount> v22 = SquareMagnitude(v2);
        Scalar<DataType, ThreadCount> v1v2 = Dot(v1, v2);

        Scalar<DataType, ThreadCount> det = v1v2 * v1v2 - v12 * v22;

        constexpr DataType min = std::is_floating_point_v<DataType> ? std::numeric_limits<DataType>::min() : DataType(0);
        typename Scalar<DataType, ThreadCount>::MaskType mask = Abs(det) > min;

        // det == 0;
        Scalar<DataType, ThreadCount> dpv1 = Dot(dp, v1);
        Scalar<DataType, ThreadCount> dpv2 = Dot(dp, v2);
        Scalar<DataType, ThreadCount> t1 = (v1v2 * dpv2 -  v22 * dpv1) * det;
        Scalar<DataType, ThreadCount> t2 = (v12  * dpv2 - v1v2 * dpv1) * det;
        Scalar<DataType, ThreadCount> r1 = Magnitude(dp + v2 * t2 - v1 * t1);

        // det > 0, The lines are nearly parallel
        Vector3<DataType, ThreadCount> a = Cross(dp, v1);
        Scalar<DataType, ThreadCount> r2 = Sqrt(SquareMagnitude(a) / v12); 

        return Select(r1, r2, mask);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount>::MaskType IntersectionLinePlan(const Point3<DataType, ThreadCount> &p, const Vector3<DataType, ThreadCount> &v, const Plane<DataType, ThreadCount> &f, Point3<DataType, ThreadCount> *q)
    {
        Scalar<DataType, ThreadCount> fv = Dot(f, v);

        constexpr DataType min = std::is_floating_point_v<DataType> ? std::numeric_limits<DataType>::min() : DataType(0);
        typename Scalar<DataType, ThreadCount>::MaskType mask = Abs(fv) > min;

        if (q != nullptr) *q = Select(p - v * (Dot(f, p) / fv), *q, mask); 

        return mask; 
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount>::MaskType IntersectionThreePlan(const Plane<DataType, ThreadCount> &f1, const Plane<DataType, ThreadCount> &f2, const Plane<DataType, ThreadCount> &f3, Point3<DataType, ThreadCount> *p)
    {
        const Vector3<DataType, ThreadCount>& n1 = f1.GetNormal();
        const Vector3<DataType, ThreadCount>& n2 = f2.GetNormal();
        const Vector3<DataType, ThreadCount>& n3 = f3.GetNormal();

        Vector3<DataType, ThreadCount> n1xn2 = Cross(n1, n2);
        Scalar<DataType, ThreadCount> det = Dot(n1xn2, n3);

        constexpr DataType min = std::is_floating_point_v<DataType> ? std::numeric_limits<DataType>::min() : DataType(0);
        typename Scalar<DataType, ThreadCount>::MaskType mask = Abs(det) > min;

        if (p != nullptr) *p = Select(
            (Cross(n3, n2) * f1.w + Cross(n1, n3) * f2.w - n1xn2 * f3.w) / det,
            *p,
            mask);
        
        return mask; 
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount>::MaskType IntersectionTwoPlan(const Plane<DataType, ThreadCount> &f1, const Plane<DataType, ThreadCount> &f2, Point3<DataType, ThreadCount> *p, Vector3<DataType, ThreadCount> *v)
    {
        if (v == nullptr) return {};
        
        const Vector3<DataType, ThreadCount>& n1 = f1.GetNormal();
        const Vector3<DataType, ThreadCount>& n2 = f2.GetNormal();

        *v = Cross(n1, n2);
        Scalar<DataType, ThreadCount> det = SquareMagnitude(*v);

        constexpr DataType min = std::is_floating_point_v<DataType> ? std::numeric_limits<DataType>::min() : DataType(0);
        typename Scalar<DataType, ThreadCount>::MaskType mask = Abs(det) > min;

        *p = Select(
            (Cross(*v, n2) * f1.w + Cross(n1, *v) * f2.w) / det,
            *p,
            mask);
        
        return mask; 
    }
}