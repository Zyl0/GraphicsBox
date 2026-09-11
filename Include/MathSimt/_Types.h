#pragma once

#include <algorithm>
#include <bit>
#include <initializer_list>

#include "Memory/Functions.h"
#include "Shared/Annotations.h"

#ifdef CONFIG_DEBUG
#include "Shared/Assertion.h"
#endif // CONFIG_DEBUG

#ifdef USE_OPENMP
#include <omp.h>
#define MATH_SIMT_SIMDIFY_FOR #pragma omp simd
#define MATH_SIMT_SIMDIFY_ALIGNED(Variable, Alignement) aligned(Variable:Alignement)
#else // USE_OPENMP
#define MATH_SIMT_SIMDIFY_FOR
#define MATH_SIMT_SIMDIFY_ALIGNED(Variable, Alignement)
#endif // !USE_OPENMP

namespace Math::Simt
{
    // using half = _Float16;

    template <size_t ThreadCount, size_t Scale = 1 /* Used only when agregating simd lanes*/>
    struct Mask
    {
        using Type =
            std::conditional_t<ThreadCount <= 8, uint8_t,
            std::conditional_t<ThreadCount <= 16, uint16_t,
            std::conditional_t<ThreadCount <= 32, uint32_t,
            uint64_t>>>;
        
        static constexpr size_t kThreadCount = ThreadCount * Scale;
        static constexpr size_t kScale = Scale;
        static constexpr size_t kPrimitiveThreadCount = ThreadCount;

        static consteval size_t Size() {return ThreadCount;}

        Type bits[Scale] {};
        
        Mask() = default;
        constexpr Mask(bool b)
        {
            for (size_t i = 0; i < Scale; ++i)
            {
                bits[i] = b ? FullBitMask() : 0;
            }
        }
        
        constexpr Mask(std::initializer_list<Type> b)
        {
            size_t chnk = 0;
            for (Type mask : b)
            {
                bits[chnk++] = mask;
            }

            for (; chnk < Scale; ++chnk) bits[chnk] = 0;
        }

        constexpr Mask& operator = (std::initializer_list<Type> b)
        {
            size_t chnk = 0;
            for (Type mask : b)
            {
                bits[chnk++] = mask;
            }

            for (; chnk < Scale; ++chnk) bits[chnk] = 0;

            return *this;
        }

        constexpr Mask& operator = (bool b)
        {
            for (size_t i = 0; i < Scale; ++i)
            {
                bits[i] = b ? std::numeric_limits<Type>::max() & FullBitMask() : 0;
            }

            return *this;
        }

        constexpr bool operator[](size_t i) const
        {
            return (bits[i / ThreadCount] >> (i % ThreadCount)) & 1;
        }

        struct Proxy
        {
            Mask& m;
            size_t i;
            Proxy& operator=(bool v)
            {
#ifdef CONFIG_DEBUG
                AssertOrError(i < kThreadCount && (i / ThreadCount) < Scale, "Index out of range")
#endif // CONFIG_DEBUG
                m.bits[i / ThreadCount] = SetBoolAt(m.bits[i / ThreadCount], (i % ThreadCount), v);
                return *this;
            }
            operator bool() const { return (m.bits[i / ThreadCount] >> (i % ThreadCount)) & 1; }
        };
    
        Proxy operator[](size_t i) { return {*this, i}; }

        constexpr Mask operator&(Mask o) const
        {
            Mask r;
            for (size_t i = 0; i < Scale; ++i)
                r.bits[i] = bits[i] & o.bits[i];
            return r;
        }
        constexpr Mask operator&&(Mask o) const
        {
            Mask r;
            for (size_t i = 0; i < Scale; ++i)
                r.bits[i] = bits[i] & o.bits[i];
            return r;
        }
        constexpr Mask operator|(Mask o) const
        {
            Mask r;
            for (size_t i = 0; i < Scale; ++i)
                r.bits[i] = bits[i] | o.bits[i];
            return r;
        }
        constexpr Mask operator||(Mask o) const
        {
            Mask r;
            for (size_t i = 0; i < Scale; ++i)
                r.bits[i] = bits[i] | o.bits[i];
            return r;
        }
        constexpr Mask operator^(Mask o) const
        {
            Mask r;
            for (size_t i = 0; i < Scale; ++i)
                r.bits[i] = bits[i] ^ o.bits[i];
            return r;
        }
        constexpr Mask operator~() const
        {
            Mask r;
            for (size_t i = 0; i < Scale; ++i)
                r.bits[i] =  ~(bits[i]) & FullBitMask();
            return r;
        }
        constexpr Mask operator!() const
        {
            Mask r;
            for (size_t i = 0; i < Scale; ++i)
                r.bits[i] = ~(bits[i]) & FullBitMask();
            return r;
        }

        constexpr Mask& operator&=(Mask o)
        {
            for (size_t i = 0; i < Scale; ++i)
                bits[i] &= o.bits[i];
            return *this;
        }
        constexpr Mask& operator|=(Mask o)
        {
            for (size_t i = 0; i < Scale; ++i)
                bits[i] |= o.bits[i];
            return *this;
        }
        constexpr Mask& operator^=(Mask o)
        {
            for (size_t i = 0; i < Scale; ++i)
                bits[i] ^= o.bits[i];
            return *this;
        }

        static constexpr Type FullBitMask()
        {
            if constexpr (ThreadCount == (sizeof(Type) * 8))
                return ~Type(0);
            else
                return (Type(1) << ThreadCount) - 1;
        }

        constexpr bool All() const
        {
            bool eqf = true;
            for (size_t i = 0; i < Scale; ++i)
                eqf &= ((bits[i] & FullBitMask()) == FullBitMask());
            return eqf;
        }
        
        constexpr bool Any() const
        { 
            return !None();
        }
        
        constexpr bool None() const
        {
            bool eq0 = true;
            for (size_t i = 0; i < Scale; ++i)
                eq0 &= bits[i] == 0;
            return eq0;
        }
    
        constexpr size_t Count() const
        {
            size_t r = 0;
            for (size_t i = 0; i < Scale; ++i)
                r += std::popcount(bits[i]);
            return r;
        }
    };
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct alignas(sizeof(DataType) * ThreadCount) Scalar
    {
        using Type = DataType;
        using MaskType = Mask<ThreadCount>;
        using IndexerType = Scalar<int32_t, ThreadCount>;
        
        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;

        static consteval size_t Size() {return ThreadCount;}

        DataType ALIGNED_VECTOR(sizeof(DataType) * kThreadCount, sizeof(DataType) * kThreadCount) m VECTOR_FALLBACK(ThreadCount);

        Scalar();
        Scalar(Type v);
        Scalar(std::initializer_list<Type> vs);
        Scalar(const Scalar& other);
        INLINE Scalar& operator = (Type v);
        INLINE Scalar& operator = (std::initializer_list<Type> vs);

        INLINE Type& operator [] (size_t index) {return m[index];}
        INLINE const Type& operator [] (size_t index) const {return m[index];}

        INLINE Scalar& operator += (const Scalar& other);
        INLINE Scalar& operator -= (const Scalar& other);
        INLINE Scalar& operator *= (const Scalar& other);
        INLINE Scalar& operator /= (const Scalar& other);
        INLINE Scalar& operator += (Type other);
        INLINE Scalar& operator -= (Type other);
        INLINE Scalar& operator *= (Type other);
        INLINE Scalar& operator /= (Type other);

        INLINE Scalar& Zero();

        static Scalar Load(const DataType* ptr);
        static Scalar Load(const DataType* ptr, const MaskType& mask);
        static Scalar LoadAligned(const DataType* ALIGNED(kAlignment) ptr);
        static Scalar LoadAligned(const DataType* ALIGNED(kAlignment) ptr, const MaskType& mask);
        void Store(DataType* ptr) const;
        void Store(DataType* ptr, const Mask<ThreadCount>& mask) const;
        void StoreAligned(DataType* ALIGNED(kAlignment) ptr) const;
        void StoreAligned(DataType* ALIGNED(kAlignment) ptr, const MaskType& mask) const;
        void Set(const DataType& rawValue);
        
        Scalar operator%(const Scalar& o) const requires std::is_integral_v<DataType> { Scalar r; for (size_t i=0; i<ThreadCount; ++i) r.m[i] = m[i] % o.m[i]; return r; }
        Scalar operator&(const Scalar& o) const requires std::is_integral_v<DataType> { Scalar r; for (size_t i=0; i<ThreadCount; ++i) r.m[i] = m[i] & o.m[i]; return r; }
        Scalar operator|(const Scalar& o) const requires std::is_integral_v<DataType> { Scalar r; for (size_t i=0; i<ThreadCount; ++i) r.m[i] = m[i] | o.m[i]; return r; }
        Scalar operator^(const Scalar& o) const requires std::is_integral_v<DataType> { Scalar r; for (size_t i=0; i<ThreadCount; ++i) r.m[i] = m[i] ^ o.m[i]; return r; }
        Scalar operator~() const requires std::is_integral_v<DataType> { Scalar r; for (size_t i=0; i<ThreadCount; ++i) r.m[i] = ~m[i]; return r; }
        Scalar operator<<(int shift) const requires std::is_integral_v<DataType> { Scalar r; for (size_t i=0; i<ThreadCount; ++i) r.m[i] = m[i] << shift; return r; }
        Scalar operator>>(int shift) const requires std::is_integral_v<DataType> { Scalar r; for (size_t i=0; i<ThreadCount; ++i) r.m[i] = m[i] >> shift; return r; }
        Scalar operator-() const { Scalar r; for (size_t i=0; i<ThreadCount; ++i) r.m[i] = -m[i]; return r; }

        Mask<ThreadCount> operator==(const Scalar& o) const { Mask<ThreadCount> mask; for (size_t i=0; i<ThreadCount; ++i) mask[i] = (m[i] == o.m[i]); return mask; }
        Mask<ThreadCount> operator!=(const Scalar& o) const { Mask<ThreadCount> mask; for (size_t i=0; i<ThreadCount; ++i) mask[i] = (m[i] != o.m[i]); return mask; }
        Mask<ThreadCount> operator<(const Scalar& o) const  { Mask<ThreadCount> mask; for (size_t i=0; i<ThreadCount; ++i) mask[i] = (m[i] < o.m[i]); return mask; }
        Mask<ThreadCount> operator<=(const Scalar& o) const { Mask<ThreadCount> mask; for (size_t i=0; i<ThreadCount; ++i) mask[i] = (m[i] <= o.m[i]); return mask; }
        Mask<ThreadCount> operator>(const Scalar& o) const  { Mask<ThreadCount> mask; for (size_t i=0; i<ThreadCount; ++i) mask[i] = (m[i] > o.m[i]); return mask; }
        Mask<ThreadCount> operator>=(const Scalar& o) const { Mask<ThreadCount> mask; for (size_t i=0; i<ThreadCount; ++i) mask[i] = (m[i] >= o.m[i]); return mask; }

    };

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>::Scalar()
    {
        Zero();
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>::Scalar(Type v)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            m[i] = v;
        }
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
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
            Zero();
            size_t CopySize = std::min(kThreadCount, vs.size());
            for (size_t i = 0; i < CopySize; ++i)
            {
                m[i] = *(vs.begin() + i);
            }
        }
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>::Scalar(const Scalar& other)
    {
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] = other.m[i];
        }
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator=(Type v)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] = v;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator=(std::initializer_list<Type> vs)
    {
        if (vs.size() == 1)
        {
            Type v = *(vs.begin());

            MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
            for (size_t i = 0; i < ThreadCount; ++i)
            {
                m[i] = v;
            }
        }
        else
        {
            Zero();
            size_t CopySize = std::min(kThreadCount, vs.size());
            for (size_t i = 0; i < CopySize; ++i)
            {
                m[i] = *(vs.begin() + i);
            }
        }
        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator + (const Scalar<DataType, ThreadCount>& a, const Scalar<DataType, ThreadCount>& b)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(b.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] + b.m[i];
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator - (const Scalar<DataType, ThreadCount>& a, const Scalar<DataType, ThreadCount>& b)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(b.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] - b.m[i];
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator * (const Scalar<DataType, ThreadCount>& a, const Scalar<DataType, ThreadCount>& b)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(b.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] * b.m[i];
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator / (const Scalar<DataType, ThreadCount>& a, const Scalar<DataType, ThreadCount>& b)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(b.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] / b.m[i];
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator + (const Scalar<DataType, ThreadCount>& a, DataType b)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] + b;
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator + (DataType b, const Scalar<DataType, ThreadCount>& a)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] + b;
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator - (const Scalar<DataType, ThreadCount>& a, DataType b)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] - b;
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator - (DataType b, const Scalar<DataType, ThreadCount>& a)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = b - a.m[i];
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator * (const Scalar<DataType, ThreadCount>& a, DataType b)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] * b;
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator * (DataType b, const Scalar<DataType, ThreadCount>& a)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] * b;
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator / (const Scalar<DataType, ThreadCount>& a, DataType b)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = a.m[i] / b;
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> operator / (DataType b, const Scalar<DataType, ThreadCount>& a)
    {
        Scalar<DataType, ThreadCount> r;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(a.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r.m[i] = b / a.m[i];
        }

        return r;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator+=(const Scalar& other)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(other.m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] += other.m[i];
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator-=(const Scalar& other)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(other.m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] -= other.m[i];
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator*=(const Scalar& other)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(other.m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] *= other.m[i];
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator/=(const Scalar& other)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(other.m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] /= other.m[i];
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator+=(Type other)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] += other;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator-=(Type other)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] -= other;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator*=(Type other)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] *= other;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::operator/=(Type other)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] /= other;
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Scalar<DataType, ThreadCount>& Scalar<DataType, ThreadCount>::Zero()
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < kThreadCount; ++i)
        {
            m[i] = Type(0);
        }

        return *this;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Scalar<DataType, ThreadCount>::Load(const DataType* ptr)
    {
        Scalar m;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            m[i] = ptr[i];
        }
        return m;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Scalar<DataType, ThreadCount>::Load(const DataType* ptr,
        const Mask<ThreadCount>& mask)
    {
        Scalar m;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            m[i] = mask[i] ? ptr[i] : DataType(0);
        }
        return m;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Scalar<DataType, ThreadCount>::LoadAligned(const DataType* ptr)
    {
        Scalar m;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            m[i] = ptr[i];
        }
        return m;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Scalar<DataType, ThreadCount>::LoadAligned(const DataType* ptr,
        const Mask<ThreadCount>& mask)
    {
        Scalar m;
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            m[i] = mask[i] ? ptr[i] : m[i];
        }
        return m;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE void Scalar<DataType, ThreadCount>::Store(DataType* ptr) const
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            ptr[i] = m[i];
        }
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE void Scalar<DataType, ThreadCount>::Store(DataType* ptr, const Mask<ThreadCount>& mask) const
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            ptr[i] = mask[i] ? m[i] : ptr[i];
        }
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE void Scalar<DataType, ThreadCount>::StoreAligned(DataType* ptr) const
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            ptr[i] = m[i];
        }
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE void Scalar<DataType, ThreadCount>::StoreAligned(DataType* ptr, const Mask<ThreadCount>& mask) const
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment) MATH_SIMT_SIMDIFY_ALIGNED(ptr, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            ptr[i] = mask[i] ? m[i] : ptr[i];
        }
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    void Scalar<DataType, ThreadCount>::Set(const DataType& rawValue)
    {
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            m[i] = rawValue;
        }
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Select(
        const Scalar<DataType, ThreadCount>& A,
        const Scalar<DataType, ThreadCount>& B,
        const typename Scalar<DataType, ThreadCount>::MaskType& mask)
    {
        Scalar<DataType, ThreadCount> r;
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r[i] = mask[i] ? A[i] : B[i];
        }
        return r;
    }
    
    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Gather(
        const DataType* ptr,
        const typename Scalar<DataType, ThreadCount>::IndexerType& indices)
    {
        Scalar<DataType, ThreadCount> r;
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            r[i] = ptr[indices[i]];
        }
        return r;
    }
    
    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE void Scatter(
        const Scalar<DataType, ThreadCount>& values,
        DataType* ptr,
        const typename Scalar<DataType, ThreadCount>::IndexerType& indices)
    {
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            ptr[indices[i]] = values[i];
        }
    }
    
    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Permute(
        const Scalar<DataType, ThreadCount>& V,
        const typename Scalar<DataType, ThreadCount>::IndexerType& indices
        )
    {
        Scalar<DataType, ThreadCount> r;
        for (size_t i = 0; i < ThreadCount; ++i) r[i] = V[indices[i]];
        return r;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Pack(
        const Scalar<DataType, ThreadCount>& V,
        const typename Scalar<DataType, ThreadCount>::MaskType& mask)
    {
        Scalar<DataType, ThreadCount> r(0);
        size_t idx = 0;
        for (size_t i = 0; i < ThreadCount; ++i) if (mask[i]) r[idx++] = V[i];
        return r;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> UnPack(
        const Scalar<DataType, ThreadCount>& V,
        const typename Scalar<DataType, ThreadCount>::MaskType& mask)
    {
        Scalar<DataType, ThreadCount> r(0);
        size_t idx = 0;

        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(r.m, kAlignment)
        for (size_t i = 0; i < ThreadCount; ++i) if (mask[i]) r[i] = V[idx++];
        
        return r;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Split(
        const Scalar<DataType, ThreadCount>& V,
        const typename Scalar<DataType, ThreadCount>::MaskType& mask)
    {
        Scalar<DataType, ThreadCount> r(0);
        size_t count1 = 0;
        for (size_t i = 0; i < ThreadCount; ++i) if (mask[i]) count1++;
        
        size_t idx1 = count1, idx0 = 0;
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            if (mask[i])
                r[idx1++] = V[i];
            else
                r[idx0++] = V[i];
        }
        
        return r;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> UnSplit(
    const Scalar<DataType, ThreadCount>& V,
    const typename Scalar<DataType, ThreadCount>::MaskType& mask)
    {
        Scalar<DataType, ThreadCount> r(0);
        size_t count1 = 0;
        for (size_t i = 0; i < ThreadCount; ++i) if (mask[i]) count1++;
        
        size_t idx1 = count1, idx0 = 0;
        for (size_t i = 0; i < ThreadCount; ++i)
        {
            if (mask[i])
                r[i] = V[idx1++];
            else
                r[i] = V[idx0++];
        }
        
        return r;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Bin(
        const Scalar<DataType, ThreadCount>& V,
        const typename Scalar<DataType, ThreadCount>::IndexerType& bins)
    {
        Scalar<DataType, ThreadCount> r(0);
        int max_bin = 0;
        for (size_t i = 0; i < ThreadCount; ++i) if (bins[i] > max_bin) max_bin = bins[i];
        
        size_t out_idx = 0;
        for (int b = 0; b <= max_bin; ++b) 
            for (size_t i = 0; i < ThreadCount; ++i)
            {
                if (bins[i] == b) r[out_idx++] = V[i];
            }
        
        return r;
    }

    
    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> UnBin(
        const Scalar<DataType, ThreadCount>& V,
        const typename Scalar<DataType, ThreadCount>::IndexerType& bins)
    {
        Scalar<DataType, ThreadCount> r(0);
        int max_bin = 0;
        for (size_t i = 0; i < ThreadCount; ++i) if (bins[i] > max_bin) max_bin = bins[i];
        
        size_t out_idx = 0;
        for (int b = 0; b <= max_bin; ++b) 
            for (size_t i = 0; i < ThreadCount; ++i)
            {
                if (bins[i] == b) r[i] = V[out_idx++];
            }
        
        return r;
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Shift(
        const Scalar<DataType, ThreadCount>& V,
        int Amount)
    {
        typename Scalar<DataType, ThreadCount>::IndexerType indices;
        for (int i = 0; i < ThreadCount; ++i)
        {
            indices[i] = std::clamp(i + Amount, 0, static_cast<int>(ThreadCount));
        }
        return Permute(V, indices);
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Scalar<DataType, ThreadCount> Rotate(
        const Scalar<DataType, ThreadCount>& V,
        int Amount)
    {
        typename Scalar<DataType, ThreadCount>::IndexerType indices;
        const int base_offset = ThreadCount + (Amount % ThreadCount);
        for (int i = 0; i < ThreadCount; ++i)
        {
            indices[i] = (i + base_offset) % ThreadCount;
        }
        return Permute(V, indices);
    }
}
