#pragma once

#include <algorithm>
#include <initializer_list>

#include "Shared/Annotations.h"

#ifdef USE_INTRINSICS
#include <immintrin.h>
#define USE_SSE defined(__SSE__) && defined(__SSE2__)
#define USE_AVX defined(__AVX__) && defined(__AVX2__)
#define USE_AVX512 defined(__AVX512__) && defined(__AVX512F__) && defined(__AVX512VL__)
#else // USE_INTRINSICS
#define USE_SSE 0
#define USE_AVX 0
#define USE_AVX512 0
#endif // !USE_INTRINSICS

#ifdef USE_OPENMP
#include <omp.h>
#define MATH_SIMT_SIMDIFY_FOR #pragma omp simd
#else // USE_OPENMP
#define MATH_SIMT_SIMDIFY_FOR
#endif // !USE_OPENMP

namespace Math::Simt
{
    using half = _Float16;

    template<typename DataType, size_t ThreadCount>
    struct Scalar
    {
        using Type = DataType;
        static constexpr size_t kThreadCount = ThreadCount;

        DataType ALIGNED_VECTOR(sizeof(DataType) * kThreadCount, sizeof(DataType) * kThreadCount) m;

        Scalar();
        Scalar(Type v);
        Scalar(std::initializer_list<Type> vs);
        Scalar(const Scalar& other);
        INLINE Scalar& operator = (Type v);
        INLINE Scalar& operator = (std::initializer_list<Type> vs);

        INLINE const Type& operator [] (size_t index) const {return m[index];}

        INLINE Scalar& operator += (Scalar other);
        INLINE Scalar& operator -= (Scalar other);
        INLINE Scalar& operator *= (Scalar other);
        INLINE Scalar& operator /= (Scalar other);
        INLINE Scalar& operator += (Type other);
        INLINE Scalar& operator -= (Type other);
        INLINE Scalar& operator *= (Type other);
        INLINE Scalar& operator /= (Type other);
    };

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>::Scalar()
    {
        m = {};
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>::Scalar(Type v)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            m[i] = v;
        }
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>::Scalar(std::initializer_list<DataType> vs)
    {
        if (vs.size() == 1)
        {
            Type value = *(vs.begin());

MATH_SIMT_SIMDIFY_FOR
            for (size_t i = 0; i < ThreadCount; ++i)
            {
                m[i] = value;
            }
        }
        else
        {
            m = {};
            size_t CopySize = std::min(kThreadCount, vs.size());
            for (size_t i = 0; i < CopySize; ++i)
            {
                m[i] = *(vs.begin() + i);
            }
        }
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>::Scalar(const Scalar& other)
    {
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] = other.m[i];
        }
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator=(Type v)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] = v;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator=(std::initializer_list<Type> vs)
    {
        if (vs.size() == 1)
        {
            Type v = *(vs.begin());

MATH_SIMT_SIMDIFY_FOR
            for (size_t i = 0; i < ThreadCount; ++i)
            {
                m[i] = v;
            }
        }
        else
        {
            m = {};
            size_t CopySize = std::min(kThreadCount, vs.size());
            for (size_t i = 0; i < CopySize; ++i)
            {
                m[i] = *(vs.begin() + i);
            }
        }
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator + (Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] + b.m[i];
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator - (Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] - b.m[i];
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator * (Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] * b.m[i];
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator / (Scalar<DataType, ThreadCount> a, Scalar<DataType, ThreadCount> b)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] / b.m[i];
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator + (Scalar<DataType, ThreadCount> a, DataType b)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] + b;
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator + (DataType b, Scalar<DataType, ThreadCount> a)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] + b;
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator - (Scalar<DataType, ThreadCount> a, DataType b)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] - b;
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator - (DataType b, Scalar<DataType, ThreadCount> a)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = b - a.m[i];
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator * (Scalar<DataType, ThreadCount> a, DataType b)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] * b;
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator * (DataType b, Scalar<DataType, ThreadCount> a)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] * b;
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator / (Scalar<DataType, ThreadCount> a, DataType b)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = a.m[i] / b;
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount> operator / (DataType b, Scalar<DataType, ThreadCount> a)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            a.m[i] = b / a.m[i];
        }

        return a;
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator+=(Scalar other)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] += other.m[i];
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator-=(Scalar other)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] -= other.m[i];
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator*=(Scalar other)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] *= other.m[i];
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator/=(Scalar other)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] /= other.m[i];
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator+=(Type other)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] += other;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator-=(Type other)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] -= other;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount>
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator*=(Type other)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] *= other;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount>
    INLINE Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator/=(Type other)
    {
MATH_SIMT_SIMDIFY_FOR
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] /= other;
        }

        return *this;
    }

#if USE_SSE
    template<>
    struct Scalar<float, 4>
    {
        using Type = float;
        static constexpr size_t kThreadCount = 4;

        __m128 m;
    };
#endif // USE_SSE

#if USE_AVX
    template<>
    struct Scalar<double, 4>
    {
        using Type = float;
        static constexpr size_t kThreadCount = 4;

        __m256d m;
    };
#endif // USE_AVX

#if USE_SSE
    template<>
    struct Scalar<half, 8>
    {
        using Type = half;
        static constexpr size_t kThreadCount = 8;

        __m128h m;
    };
#endif // USE_SSE

#if USE_AVX
    template<>
    struct Scalar<float, 8>
    {
        using Type = float;
        static constexpr size_t kThreadCount = 8;

        __m256 m;
    };
#endif // USE_AVX

#if USE_AVX512
    template<>
    struct Scalar<double, 8>
    {
        using Type = float;
        static constexpr size_t kThreadCount = 8;

        __m512d m;
    };
#endif // USE_AVX512

#if USE_AVX
    template<>
    struct Scalar<half, 16>
    {
        using Type = half;
        static constexpr size_t kThreadCount = 16;

        __m256h m;
    };
#endif // USE_AVX

#if USE_AVX512
    template<>
    struct Scalar<float, 16>
    {
        using Type = float;
        static constexpr size_t kThreadCount = 16;

        __m512 m;
    };
#endif // USE_AVX512

#if USE_AVX512
    template<>
    struct Scalar<half, 32>
    {
        using Type = half;
        static constexpr size_t kThreadCount = 32;

        __m512h m;
    };
#endif // USE_AVX512
}