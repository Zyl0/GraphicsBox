#pragma once

#include "Types.h"

#include "Math/Functons.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Abs(const Scalar<DataType, ThreadCount>& v)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v < 0;
        return Select(-v, v, mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Sqrt(const Scalar<DataType, ThreadCount>& v)
    {
        Scalar<DataType, ThreadCount> r;
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::sqrt(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow(const Scalar<DataType, ThreadCount>&v, const Scalar<DataType, ThreadCount>&exp)
    {
        Scalar<DataType, ThreadCount> r;
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::pow(v.m[i], exp.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow(const Scalar<DataType, ThreadCount>&v, DataType exp)
    {
        Scalar<DataType, ThreadCount> r;
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::pow(v.m[i], exp);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow2(const Scalar<DataType, ThreadCount>&v)
    {
        return v * v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Log(const Scalar<DataType, ThreadCount>&v)
    {
        Scalar<DataType, ThreadCount> r;
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::log(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Exp(const Scalar<DataType, ThreadCount>&v)
    {
        Scalar<DataType, ThreadCount> r;
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::exp(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Radians(const Scalar<DataType, ThreadCount>&degrees)
    {
        constexpr DataType w = static_cast<DataType>(M_PI) / DataType(180);
        return w * degrees;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Degrees(const Scalar<DataType, ThreadCount>&radians)
    {
        constexpr DataType w = static_cast<DataType>(180) / DataType(M_PI);
        return w * radians;
    }
    
    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Min(const Scalar<DataType, ThreadCount>&v, DataType min)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v < min;
        return Select(v, Scalar<DataType, ThreadCount>(min), mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Min(const Scalar<DataType, ThreadCount>&v, const Scalar<DataType, ThreadCount>&min)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v < min;
        return Select(v, min, mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Max(const Scalar<DataType, ThreadCount>&v, DataType max)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v > max;
        return Select(v, Scalar<DataType, ThreadCount>(max), mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Max(const Scalar<DataType, ThreadCount>&v, const Scalar<DataType, ThreadCount>&max)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v > max;
        return Select(v, max, mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(const Scalar<DataType, ThreadCount>&v, DataType min, DataType max)
    {
        return Min(Max(v, min), max);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(const Scalar<DataType, ThreadCount>&v, const Scalar<DataType, ThreadCount>&min, DataType max)
    {
        return Min(Max(v, min), max);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(const Scalar<DataType, ThreadCount>&v, DataType min, const Scalar<DataType, ThreadCount>&max)
    {
        return Min(Max(v, min), max);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(const Scalar<DataType, ThreadCount>&v, const Scalar<DataType, ThreadCount>&min, const Scalar<DataType, ThreadCount>&max)
    {
        return Min(Max(v, min), max);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Saturate(const Scalar<DataType, ThreadCount>&v)
    {
        return Clamp(v, static_cast<DataType>(0), static_cast<DataType>(1));
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(const Scalar<DataType, ThreadCount>& a, const Scalar<DataType, ThreadCount>& b, const Scalar<DataType, ThreadCount>& alpha)
    {
        return alpha * b + (DataType(1) - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, const Scalar<DataType, ThreadCount>& b, const Scalar<DataType, ThreadCount>& alpha)
    {
        return alpha * b + (DataType(1) - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(const Scalar<DataType, ThreadCount>& a, DataType b, const Scalar<DataType, ThreadCount>& alpha)
    {
        return alpha * b + (DataType(1) - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, DataType b, const Scalar<DataType, ThreadCount>& alpha)
    {
        return alpha * b + (DataType(1) - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(const Scalar<DataType, ThreadCount>& a, const Scalar<DataType, ThreadCount>& b, DataType alpha)
    {
        return alpha * b + (DataType(1) - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, const Scalar<DataType, ThreadCount>& b, DataType alpha)
    {
        return alpha * b + (DataType(1) - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(const Scalar<DataType, ThreadCount>& a, DataType b, DataType alpha)
    {
        return alpha * b + (DataType(1) - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(const Scalar<DataType, ThreadCount>& a, const Scalar<DataType, ThreadCount>& b, const Scalar<DataType, ThreadCount>& value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, const Scalar<DataType, ThreadCount>& b, const Scalar<DataType, ThreadCount>& value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(const Scalar<DataType, ThreadCount>& a, DataType b, const Scalar<DataType, ThreadCount>& value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, DataType b, const Scalar<DataType, ThreadCount>& value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(const Scalar<DataType, ThreadCount>& a, const Scalar<DataType, ThreadCount>& b, DataType value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, const Scalar<DataType, ThreadCount>& b, DataType value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(const Scalar<DataType, ThreadCount>& a, DataType b, DataType value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> BiLinearInterpolate(
        const Scalar<DataType, ThreadCount>& p00,
        const Scalar<DataType, ThreadCount>& p10,
        const Scalar<DataType, ThreadCount>& p01,
        const Scalar<DataType, ThreadCount>& p11,
        const Scalar<DataType, ThreadCount>& u,
        const Scalar<DataType, ThreadCount>& v)
    {
        // Interpolate along the v direction
        Scalar<DataType, ThreadCount> p0 = LinearInterpolate(p00, p10, v);
        Scalar<DataType, ThreadCount> p1 = LinearInterpolate(p01, p11, v);

        // Interpolate along the u direction
        Scalar<DataType, ThreadCount> result = LinearInterpolate(p0, p1, u);

        return result;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> SmoothStep(const Scalar<DataType, ThreadCount>& v)
    {
        Scalar<DataType, ThreadCount> X2 = v * v;
        Scalar<DataType, ThreadCount> X3 = X2 * v;
        return static_cast<DataType>(3) * X2 - static_cast<DataType>(2) * X3;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> SmoothStepClamped(const Scalar<DataType, ThreadCount>& v)
    {
        v = Saturate(v);

        Scalar<DataType, ThreadCount> X2 = v * v;
        Scalar<DataType, ThreadCount> X3 = X2 * v;
        return static_cast<DataType>(3) * X2 - static_cast<DataType>(2) * X3;
    }


    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Sin(const Scalar<DataType, ThreadCount>& v)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::sin(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> ASin(const Scalar<DataType, ThreadCount>& v)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::asin(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Cos(const Scalar<DataType, ThreadCount>& v)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::cos(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> ACos(const Scalar<DataType, ThreadCount>& v)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::acos(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Tan(const Scalar<DataType, ThreadCount>& v)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::tan(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> ATan(const Scalar<DataType, ThreadCount>& v)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::atan(v.m[i]);
        }

        return v;
    }
    
    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> ATan2(const Scalar<DataType, ThreadCount>& Y, const Scalar<DataType, ThreadCount>& X)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = std::atan2(Y.m[i], X.m[i]);
        }

        return Y;
    }
}