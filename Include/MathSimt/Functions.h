#pragma once

#include "Types.h"

#include "Math/Functons.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Abs(Scalar<DataType, ThreadCount> v);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Sqrt(Scalar<DataType, ThreadCount> v);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> exp);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow(Scalar<DataType, ThreadCount> v, DataType exp);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow2(Scalar<DataType, ThreadCount> v);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Log(Scalar<DataType, ThreadCount> v);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Exp(Scalar<DataType, ThreadCount> v);

    // template<typename DataType, size_t ThreadCount>
    // INLINE Scalar<DataType, ThreadCount> Exp10(Scalar<DataType, ThreadCount> v);

    // template<typename DataType, size_t ThreadCount>
    // INLINE Scalar<DataType, ThreadCount> Log10(Scalar<DataType, ThreadCount> v);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Radians(Scalar<DataType, ThreadCount> v);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Degrees(Scalar<DataType, ThreadCount> v);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Min(Scalar<DataType, ThreadCount> v, DataType min);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Min(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> min);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Max(Scalar<DataType, ThreadCount> v, DataType max);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Max(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> max);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(Scalar<DataType, ThreadCount> v, DataType min, DataType max);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> min, DataType max);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(Scalar<DataType, ThreadCount> v, DataType min, Scalar<DataType, ThreadCount> max);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> min, Scalar<DataType, ThreadCount> max);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Saturate(Scalar<DataType, ThreadCount> v);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b, Scalar<DataType, ThreadCount> alpha);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, Scalar<DataType, ThreadCount> b, Scalar<DataType, ThreadCount> alpha);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(Scalar<DataType, ThreadCount> a, DataType b, Scalar<DataType, ThreadCount> alpha);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, DataType b, Scalar<DataType, ThreadCount> alpha);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b, DataType alpha);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, Scalar<DataType, ThreadCount> b, DataType alpha);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(Scalar<DataType, ThreadCount> a, DataType b, DataType alpha);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b, Scalar<DataType, ThreadCount> value);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, Scalar<DataType, ThreadCount> b, Scalar<DataType, ThreadCount> value);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(Scalar<DataType, ThreadCount> a, DataType b, Scalar<DataType, ThreadCount> value);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, DataType b, Scalar<DataType, ThreadCount> value);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b, DataType value);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, Scalar<DataType, ThreadCount> b, DataType value);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(Scalar<DataType, ThreadCount> a, DataType b, DataType value);

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> SmoothStep(Scalar<DataType, ThreadCount> v);




    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Abs(Scalar<DataType, ThreadCount> v)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v < 0;
        return Select(-v, v, mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Sqrt(Scalar<DataType, ThreadCount> v)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            v.m[i] = std::sqrt(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> exp)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            v.m[i] = std::pow(v.m[i], exp.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow(Scalar<DataType, ThreadCount> v, DataType exp)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            v.m[i] = std::pow(v.m[i], exp);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Pow2(Scalar<DataType, ThreadCount> v)
    {
        return v * v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Log(Scalar<DataType, ThreadCount> v)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            v.m[i] = std::log(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Exp(Scalar<DataType, ThreadCount> v)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            v.m[i] = std::exp(v.m[i]);
        }

        return v;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Radians(Scalar<DataType, ThreadCount> degrees)
    {
        constexpr DataType w = static_cast<DataType>(M_PI) / DataType(180);
        return w * degrees;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Degrees(Scalar<DataType, ThreadCount> radians)
    {
        constexpr DataType w = static_cast<DataType>(180) / DataType(M_PI);
        return w * radians;
    }
    
    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Min(Scalar<DataType, ThreadCount> v, DataType min)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v < min;
        return Select(v, min, mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Min(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> min)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v < min;
        return Select(v, min, mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Max(Scalar<DataType, ThreadCount> v, DataType max)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v > max;
        return Select(v, max, mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Max(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> max)
    {
        typename Scalar<DataType, ThreadCount>::MaskType mask = v > max;
        return Select(v, max, mask);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(Scalar<DataType, ThreadCount> v, DataType min, DataType max)
    {
        return Min(Max(v, min), max);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> min, DataType max)
    {
        return Min(Max(v, min), max);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(Scalar<DataType, ThreadCount> v, DataType min, Scalar<DataType, ThreadCount> max)
    {
        return Min(Max(v, min), max);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Clamp(Scalar<DataType, ThreadCount> v, Scalar<DataType, ThreadCount> min, Scalar<DataType, ThreadCount> max)
    {
        return Min(Max(v, min), max);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> Saturate(Scalar<DataType, ThreadCount> v)
    {
        return Clamp(v, static_cast<DataType>(0), static_cast<DataType>(1));
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b, Scalar<DataType, ThreadCount> alpha)
    {
        return alpha * b + (1 - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, Scalar<DataType, ThreadCount> b, Scalar<DataType, ThreadCount> alpha)
    {
        return alpha * b + (1 - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(Scalar<DataType, ThreadCount> a, DataType b, Scalar<DataType, ThreadCount> alpha)
    {
        return alpha * b + (1 - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, DataType b, Scalar<DataType, ThreadCount> alpha)
    {
        return alpha * b + (1 - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b, DataType alpha)
    {
        return alpha * b + (1 - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(DataType a, Scalar<DataType, ThreadCount> b, DataType alpha)
    {
        return alpha * b + (1 - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> LinearInterpolate(Scalar<DataType, ThreadCount> a, DataType b, DataType alpha)
    {
        return alpha * b + (1 - alpha) * a;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b, Scalar<DataType, ThreadCount> value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, Scalar<DataType, ThreadCount> b, Scalar<DataType, ThreadCount> value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(Scalar<DataType, ThreadCount> a, DataType b, Scalar<DataType, ThreadCount> value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, DataType b, Scalar<DataType, ThreadCount> value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b, DataType value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(DataType a, Scalar<DataType, ThreadCount> b, DataType value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> InverseLinearInterpolate(Scalar<DataType, ThreadCount> a, DataType b, DataType value)
    {
        return (value - a) / (b - a);
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> SmoothStep(Scalar<DataType, ThreadCount> v)
    {
        Scalar<DataType, ThreadCount> X2 = v * v;
        Scalar<DataType, ThreadCount> X3 = X2 * v;
        return static_cast<DataType>(3) * X2 - static_cast<DataType>(2) * X3;
    }

    template<typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> SmoothStepClamped(Scalar<DataType, ThreadCount> v)
    {
        v = Saturate(v);

        Scalar<DataType, ThreadCount> X2 = v * v;
        Scalar<DataType, ThreadCount> X3 = X2 * v;
        return static_cast<DataType>(3) * X2 - static_cast<DataType>(2) * X3;
    }
}