#pragma once

#include "Types.h"
#include "Functions.h"
#include "ctti/detail/pretty_function.hpp"
#include "Math/Vector.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Vector2
    {
        using Type = DataType;
        using ScalarType =  Scalar<DataType, ThreadCount>;
        using MaskType = typename Scalar<DataType, ThreadCount>::MaskType;
        using IndexerType = typename Scalar<DataType, ThreadCount>::IndexerType;

        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;
        static constexpr size_t kComponentCount = 2;

        static consteval size_t Size() {return ThreadCount;}
        
        ScalarType x, y;

        constexpr Vector2() = default;
        constexpr Vector2(ScalarType a) : x(a), y(a) {}
        constexpr Vector2(ScalarType a, ScalarType b) : x(a), y(b) {}
        constexpr Vector2(const Vector2& rhs) = default;
        constexpr Vector2(const Vector2 a, const Vector2 b) : x(b.x - a.x), y(b.y - a.y) {}

        ScalarType& operator[](const int i) {return (&x)[i];}
        const ScalarType& operator[](const int i) const {return (&x)[i];}
        ScalarType& operator[](const size_t i) {return (&x)[i];}
        const ScalarType& operator[](const size_t i) const {return (&x)[i];}

        const ScalarType* data() const {return &x;}

        Vector2& operator = (const Vector2& rhs)
        {
            x = rhs.x;
            y = rhs.y;
            return *this;
        }

        Vector2& operator *=(ScalarType s)
        {
            x *= s;
            y *= s;
            return *this;
        }

        Vector2& operator /=(ScalarType s)
        {
            x /= s;
            y /= s;
            return *this;
        }

        Vector2& operator +=(ScalarType s)
        {
            x += s;
            y += s;
            return *this;
        }

        Vector2& operator -=(ScalarType s)
        {
            x -= s;
            y -= s;
            return *this;
        }

        Vector2& operator +=(const Vector2& v)
        {
            x += v.x;
            y += v.y;
            return *this;
        }

        Vector2& operator -=(const Vector2& v)
        {
            x -= v.x;
            y -= v.y;
            return *this;
        }

        Vector2& operator *=(const Vector2& v)
        {
            x *= v.x;
            y *= v.y;
            return *this;
        }

        Vector2& operator /=(const Vector2& v)
        {
            x /= v.x;
            y /= v.y;
            return *this;
        }
    };

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator +(const Vector2<DataType, ThreadCount>& v, float s)
    {
        return Vector2(v.x + s, v.y + s);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator -(const Vector2<DataType, ThreadCount>& v, float s)
    {
        return Vector2(v.x - s, v.y - s);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator *(const Vector2<DataType, ThreadCount>& v, float s)
    {
        return Vector2(v.x * s, v.y * s);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator /(const Vector2<DataType, ThreadCount>& v, float s)
    {
        return Vector2(v.x / s, v.y / s);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator +(DataType s, const Vector2<DataType, ThreadCount>& a)
    {
        return Vector2(s + a.x, s + a.y);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator -(DataType s, const Vector2<DataType, ThreadCount>& a)
    {
        return Vector2(s - a.x, s - a.y);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator *(DataType s, const Vector2<DataType, ThreadCount>& a)
    {
        return Vector2(s * a.x, s * a.y);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator /(DataType s, const Vector2<DataType, ThreadCount>& a)
    {
        return Vector2(s / a.x, s / a.y);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator -(const Vector2<DataType, ThreadCount>& v)
    {
        return Vector2(-v.x, -v.y);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector2<DataType, ThreadCount>::ScalarType SquareMagnitude(const Vector2<DataType, ThreadCount>& v)
    {
        return v.x * v.x + v.y * v.y;
    }


    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector2<DataType, ThreadCount>::ScalarType Magnitude(const Vector2<DataType, ThreadCount>& v)
    {
        typename Vector2<DataType, ThreadCount>::ScalarType r = SquareMagnitude(v);
        
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(r.m, Vector2<DataType, ThreadCount>::kAlignment)
        for (size_t i = 0; i < Vector2<DataType, ThreadCount>::kThreadCount; i++)
        {
            r.m[i] = std::sqrt(r.m[i]);
        }
        return r;
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> Normalize(const Vector2<DataType, ThreadCount>& v)
    {
        return v / Magnitude(v);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator +(const Vector2<DataType, ThreadCount>& a, const Vector2<DataType, ThreadCount>& b)
    {
        return Vector2(a.x + b.x, a.y + b.y);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator -(const Vector2<DataType, ThreadCount>& a, const Vector2<DataType, ThreadCount>& b)
    {
        return Vector2(a.x - b.x, a.y - b.y);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator *(const Vector2<DataType, ThreadCount>& a, const Vector2<DataType, ThreadCount>& b)
    {
        return Vector2(a.x * b.x, a.y * b.y);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector2<DataType, ThreadCount> operator /(const Vector2<DataType, ThreadCount>& a, const Vector2<DataType, ThreadCount>& b)
    {
        return Vector2(a.x / b.x, a.y / b.y);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector2<DataType, ThreadCount>::ScalarType Dot(const Vector2<DataType, ThreadCount> a, const Vector2<DataType, ThreadCount> b)
    {
        return a.x * b.x + a.y * b.y;
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    typename Vector2<DataType, ThreadCount>::ScalarType CosTheta(const Vector2<DataType, ThreadCount> a, const Vector2<DataType, ThreadCount> b)
    {
        Vector2<DataType, ThreadCount> aNormalized = Normalize(a);
        Vector2<DataType, ThreadCount> bNormalized = Normalize(b);
        return Dot(aNormalized, bNormalized);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Vector3
    {
        using Type = DataType;
        using ScalarType =  Scalar<DataType, ThreadCount>;
        using MaskType = typename Scalar<DataType, ThreadCount>::MaskType;
        using IndexerType = typename Scalar<DataType, ThreadCount>::IndexerType;

        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;
        static constexpr size_t kComponentCount = 3;
        
        static consteval size_t Size() {return ThreadCount;}
        
        ScalarType x, y, z;

        constexpr Vector3() = default;
        constexpr Vector3(ScalarType a) : x(a), y(a), z(a) {}
        constexpr Vector3(ScalarType a, ScalarType b, ScalarType c) : x(a), y(b), z(c) {}
        constexpr Vector3(const Vector3& rhs) = default;
        constexpr Vector3(const Vector3 a, const Vector3 b) : x(b.x - a.x), y(b.y - a.y), z(b.z - a.z) {}

        ScalarType& operator[](const int i) {return (&x)[i];}
        const ScalarType& operator[](const int i) const {return (&x)[i];}
        ScalarType& operator[](const size_t i) {return (&x)[i];}
        const ScalarType& operator[](const size_t i) const {return (&x)[i];}

        const ScalarType* data() const {return &x;}

        Vector3& operator = (const Vector3& rhs)
        {
            x = rhs.x;
            y = rhs.y;
            z = rhs.z;
            return *this;
        }

        Vector3& operator *=(ScalarType s)
        {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }

        Vector3& operator /=(ScalarType s)
        {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }

        Vector3& operator +=(ScalarType s)
        {
            x += s;
            y += s;
            z += s;
            return *this;
        }

        Vector3& operator -=(ScalarType s)
        {
            x -= s;
            y -= s;
            z -= s;
            return *this;
        }

        Vector3& operator +=(const Vector3& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        Vector3& operator -=(const Vector3& v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }

        Vector3& operator *=(const Vector3& v)
        {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            return *this;
        }

        Vector3& operator /=(const Vector3& v)
        {
            x /= v.x;
            y /= v.y;
            z /= v.z;
            return *this;
        }
    };

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator *(const Vector3<DataType, ThreadCount>& v, typename Vector3<DataType, ThreadCount>::ScalarType s)
    {
        return Vector3(v.x * s, v.y * s, v.z * s);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator *(typename Vector3<DataType, ThreadCount>::ScalarType s, const Vector3<DataType, ThreadCount>& v)
    {
        return Vector3(v.x * s, v.y * s, v.z * s);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator /(const Vector3<DataType, ThreadCount>& v, typename Vector3<DataType, ThreadCount>::ScalarType s)
    {
        return Vector3(v.x / s, v.y / s, v.z / s);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator /(typename Vector3<DataType, ThreadCount>::ScalarType s, const Vector3<DataType, ThreadCount>& a)
    {
        return Vector3(s / a.x, s / a.y, s / a.z);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator -(const Vector3<DataType, ThreadCount>& v)
    {
        return Vector3(-v.x, -v.y, -v.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector3<DataType, ThreadCount>::ScalarType SquareMagnitude(const Vector3<DataType, ThreadCount>& v)
    {
        return v.x * v.x + v.y * v.y + v.z * v.z;
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector3<DataType, ThreadCount>::ScalarType Magnitude(const Vector3<DataType, ThreadCount>& v)
    {
        typename Vector3<DataType, ThreadCount>::ScalarType r = SquareMagnitude(v);
        
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(r.m, Vector4<DataType, ThreadCount>::kAlignment)
        for (size_t i = 0; i < Vector3<DataType, ThreadCount>::kThreadCount; i++)
        {
            r.m[i] = std::sqrt(r.m[i]);
        }
        return r;
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> Normalize(const Vector3<DataType, ThreadCount>& v)
    {
        return v / Magnitude(v);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> Abs(const Vector3<DataType, ThreadCount>& v)
    {
        return Vector3<DataType, ThreadCount>(Abs(v.x), Abs(v.y), Abs(v.z));
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator +(const Vector3<DataType, ThreadCount>& a,  typename Vector3<DataType, ThreadCount>::ScalarType s)
    {
        return Vector3(a.x + s, a.y + s, a.z + s);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator -(const Vector3<DataType, ThreadCount>& a,  typename Vector3<DataType, ThreadCount>::ScalarType s)
    {
        return Vector3(a.x - s, a.y - s, a.z - s);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator -(typename Vector3<DataType, ThreadCount>::ScalarType s, const Vector3<DataType, ThreadCount>& a)
    {
        return Vector3(s - a.x, s - a.y, s - a.z);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator +(typename Vector3<DataType, ThreadCount>::ScalarType s, const Vector3<DataType, ThreadCount>& a)
    {
        return Vector3(s + a.x, s + a.y, s + a.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator +(const Vector3<DataType, ThreadCount>& a, const Vector3<DataType, ThreadCount>& b)
    {
        return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator -(const Vector3<DataType, ThreadCount>& a, const Vector3<DataType, ThreadCount>& b)
    {
        return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator *(const Vector3<DataType, ThreadCount>& a, const Vector3<DataType, ThreadCount>& b)
    {
        return Vector3(a.x * b.x, a.y * b.y, a.z * b.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator /(const Vector3<DataType, ThreadCount>& a, const Vector3<DataType, ThreadCount>& b)
    {
        return Vector3(a.x / b.x, a.y / b.y, a.z / b.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector3<DataType, ThreadCount>::ScalarType Dot(const Vector3<DataType, ThreadCount> a, const Vector3<DataType, ThreadCount> b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    typename Vector3<DataType, ThreadCount>::ScalarType CosTheta(const Vector3<DataType, ThreadCount> a, const Vector3<DataType, ThreadCount> b)
    {
        Vector3<DataType, ThreadCount> aNormalized = Normalize(a);
        Vector3<DataType, ThreadCount> bNormalized = Normalize(b);
        return Dot(aNormalized, bNormalized);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> Cross(const Vector3<DataType, ThreadCount>& a, const Vector3<DataType, ThreadCount>& b)
    {
        return Vector3(   a.y * b.z - a.z * b.y,
                            a.z * b.x - a.x * b.z,
                            a.x * b.y - a.y * b.x);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> Project(const Vector3<DataType, ThreadCount>& a, const Vector3<DataType, ThreadCount>& b)
    {
        return b * (Dot(a, b) / Dot(b,b));
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> Reflect(const Vector3<DataType, ThreadCount>& a, const Vector3<DataType, ThreadCount>& b)
    {
        return a - b * (Dot(a, b) / Dot(b,b));
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Point3 : Vector3<DataType, ThreadCount>
    {        
        constexpr Point3() = default;
        constexpr Point3(typename Vector3<DataType, ThreadCount>::ScalarType a) :
            Vector3<DataType, ThreadCount>::Vector3(a)
        {}
        constexpr Point3(typename Vector3<DataType, ThreadCount>::ScalarType a, typename Vector3<DataType, ThreadCount>::ScalarType b, typename Vector3<DataType, ThreadCount>::ScalarType c):
            Vector3<DataType, ThreadCount>::Vector3(a, b, c)
        {}
        constexpr Point3(const Point3& rhs) = default;
        constexpr Point3(const Point3 a, const Point3 b):
            Vector3<DataType, ThreadCount>::Vector3(a, b)
        {}

        Point3& operator=(const Point3& Other)
        {
            if (this == &Other)
                return *this;
            Vector3<DataType, ThreadCount>::operator =(Other);
            return *this;
        }

        Point3(const Vector3<DataType, ThreadCount>& Other):
            Vector3<DataType, ThreadCount>(Other)
        {
        }

        Point3& operator=(const Vector3<DataType, ThreadCount>& Other)
        {
            if (this == &Other)
                return *this;
            Vector3<DataType, ThreadCount>::operator =(Other);
            return *this;
        }
    };

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Point3<DataType, ThreadCount> operator +(const Point3<DataType, ThreadCount> &a, const Point3<DataType, ThreadCount> &b)
    {
        return  Point3<DataType, ThreadCount>(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Point3<DataType, ThreadCount> operator +(const Point3<DataType, ThreadCount> &p, const Vector3<DataType, ThreadCount> &v)
    {
        return  Point3<DataType, ThreadCount>(p.x + v.x, p.y + v.y, p.z + v.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Point3<DataType, ThreadCount> operator -(const Point3<DataType, ThreadCount> &a, const Point3<DataType, ThreadCount> &b)
    {
        return  Point3<DataType, ThreadCount>(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Point3<DataType, ThreadCount> operator -(const Point3<DataType, ThreadCount> &p, const Vector3<DataType, ThreadCount> &v)
    {
        return  Point3<DataType, ThreadCount>(p.x - v.x, p.y - v.y, p.z - v.z);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    float Distance( const Point3<DataType, ThreadCount>& a, const Point3<DataType, ThreadCount>& b )
    {
        return Magnitude(a - b);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    float SquareDistance( const Point3<DataType, ThreadCount>& a, const Point3<DataType, ThreadCount>& b )
    {
        return SquareMagnitude(a - b);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Point3<DataType, ThreadCount> Center( const Point3<DataType, ThreadCount>& a, const Point3<DataType, ThreadCount>& b )
    {
        return Point3<DataType, ThreadCount>((a.x + b.x) / 2, (a.y + b.y) / 2, (a.z + b.z) / 2);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Point3<DataType, ThreadCount> Min( const Point3<DataType, ThreadCount>& a, const Point3<DataType, ThreadCount>& b )
    { 
        return Point3<DataType, ThreadCount>( Min(a.x, b.x), Min(a.y, b.y), Min(a.z, b.z) ); 
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Point3<DataType, ThreadCount> Max( const Point3<DataType, ThreadCount>& a, const Point3<DataType, ThreadCount>& b ) 
    { 
        return Point3<DataType, ThreadCount>( Max(a.x, b.x), Max(a.y, b.y), Max(a.z, b.z) ); 
    }

    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Vector4
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

        Vector4() = default;

        Vector4(const ScalarType s) : x(s), y(s), z(s), w(s) {}

        Vector4(const Vector3<DataType, ThreadCount> &v3, const ScalarType d) : x(v3.x), y(v3.y), z(v3.z), w(d) {}

        Vector4(const ScalarType a, const ScalarType b, const ScalarType c, const ScalarType d) : x(a), y(b), z(c), w(d) {}
        
        Vector4(const Vector4& a, const Vector4 b) : x(b.x - a.x), y(b.y - a.y), z(b.z - a.z), w(b.w - a.w) {}
        
        Vector3<DataType, ThreadCount>& xyz()
        {
            return *reinterpret_cast<Vector3<DataType, ThreadCount>*>(this);
        }
        
        const Vector3<DataType, ThreadCount>& xyz() const
        {
            return *reinterpret_cast<const Vector3<DataType, ThreadCount>*>(this);
        }

        DataType& operator[](const int i) {return (&x)[i];}
        const DataType& operator[](const int i) const {return (&x)[i];}
        DataType& operator[](const size_t i) {return (&x)[i];}
        const DataType& operator[](const size_t i) const {return (&x)[i];}
        const DataType* data() const  {return &x;}
        DataType* data() {return &x;}

        Vector4& operator *=(ScalarType s)
        {
            x *= s;
            y *= s;
            z *= s;
            w *= s;
            return *this;
        }

        Vector4& operator /=(ScalarType s)
        {
            x /= s;
            y /= s;
            z /= s;
            w /= s;
            return *this;
        }

        Vector4& operator +=(ScalarType s)
        {
            x += s;
            y += s;
            z += s;
            w += s;
            return *this;
        }

        Vector4& operator -=(ScalarType s)
        {
            x -= s;
            y -= s;
            z -= s;
            w -= s;
            return *this;
        }

        Vector4& operator +=(const Vector4& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
            return *this;
        }

        Vector4& operator -=(const Vector4& v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            w -= v.w;
            return *this;
        }

        Vector4& operator *=(const Vector4& v)
        {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            w *= v.w;
            return *this;
        }

        Vector4& operator /=(const Vector4& v)
        {
            x /= v.x;
            y /= v.y;
            z /= v.z;
            w /= v.w;
            return *this;
        }


        INLINE Vector3<DataType, ThreadCount> XXX() const {return Vector3<DataType, ThreadCount>(x, x, x);}
        INLINE Vector3<DataType, ThreadCount> YXX() const {return Vector3<DataType, ThreadCount>(y, x, x);}
        INLINE Vector3<DataType, ThreadCount> ZXX() const {return Vector3<DataType, ThreadCount>(z, x, x);}
        INLINE Vector3<DataType, ThreadCount> XYX() const {return Vector3<DataType, ThreadCount>(x, y, x);}
        INLINE Vector3<DataType, ThreadCount> YYX() const {return Vector3<DataType, ThreadCount>(y, y, x);}
        INLINE Vector3<DataType, ThreadCount> ZYX() const {return Vector3<DataType, ThreadCount>(z, y, x);}
        INLINE Vector3<DataType, ThreadCount> XZX() const {return Vector3<DataType, ThreadCount>(x, z, x);}
        INLINE Vector3<DataType, ThreadCount> YZX() const {return Vector3<DataType, ThreadCount>(y, z, x);}
        INLINE Vector3<DataType, ThreadCount> ZZX() const {return Vector3<DataType, ThreadCount>(z, z, x);}
        
        INLINE Vector3<DataType, ThreadCount> XXY() const {return Vector3<DataType, ThreadCount>(x, x, y);}
        INLINE Vector3<DataType, ThreadCount> YXY() const {return Vector3<DataType, ThreadCount>(y, x, y);}
        INLINE Vector3<DataType, ThreadCount> ZXY() const {return Vector3<DataType, ThreadCount>(z, x, y);}
        INLINE Vector3<DataType, ThreadCount> XYY() const {return Vector3<DataType, ThreadCount>(x, y, y);}
        INLINE Vector3<DataType, ThreadCount> YYY() const {return Vector3<DataType, ThreadCount>(y, y, y);}
        INLINE Vector3<DataType, ThreadCount> ZYY() const {return Vector3<DataType, ThreadCount>(z, y, y);}
        INLINE Vector3<DataType, ThreadCount> XZY() const {return Vector3<DataType, ThreadCount>(x, z, y);}
        INLINE Vector3<DataType, ThreadCount> YZY() const {return Vector3<DataType, ThreadCount>(y, z, y);}
        INLINE Vector3<DataType, ThreadCount> ZZY() const {return Vector3<DataType, ThreadCount>(z, z, y);}

        INLINE Vector3<DataType, ThreadCount> XXZ() const {return Vector3<DataType, ThreadCount>(x, x, z);}
        INLINE Vector3<DataType, ThreadCount> YXZ() const {return Vector3<DataType, ThreadCount>(y, x, z);}
        INLINE Vector3<DataType, ThreadCount> ZXZ() const {return Vector3<DataType, ThreadCount>(z, x, z);}
        INLINE Vector3<DataType, ThreadCount> XYZ() const {return Vector3<DataType, ThreadCount>(x, y, z);}
        INLINE Vector3<DataType, ThreadCount> YYZ() const {return Vector3<DataType, ThreadCount>(y, y, z);}
        INLINE Vector3<DataType, ThreadCount> ZYZ() const {return Vector3<DataType, ThreadCount>(z, y, z);}
        INLINE Vector3<DataType, ThreadCount> XZZ() const {return Vector3<DataType, ThreadCount>(x, z, z);}
        INLINE Vector3<DataType, ThreadCount> YZZ() const {return Vector3<DataType, ThreadCount>(y, z, z);}
        INLINE Vector3<DataType, ThreadCount> ZZZ() const {return Vector3<DataType, ThreadCount>(z, z, z);}
    };
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator +(const Vector4<DataType, ThreadCount>& v, DataType s)
    {
        return Vector4(v.x + s, v.y + s, v.z + s, v.w + s);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator +(DataType s, const Vector4<DataType, ThreadCount>& v)
    {
        return Vector4(v.x + s, v.y + s, v.z + s, v.w + s);
    }    
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator -(const Vector4<DataType, ThreadCount>& v, DataType s)
    {
        return Vector4(v.x - s, v.y - s, v.z - s, v.w - s);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator -(typename Vector4<DataType, ThreadCount>::ScalarType s, const Vector4<DataType, ThreadCount>& v)
    {
        return Vector4(s - v.x,  s - v.y,   s - v.z, s - v.w);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator *(const Vector4<DataType, ThreadCount>& v, typename Vector4<DataType, ThreadCount>::ScalarType s)
    {
        return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator *(typename Vector4<DataType, ThreadCount>::ScalarType s, const Vector4<DataType, ThreadCount>& v)
    {
        return Vector4(v.x * s, v.y * s, v.z * s, v.w * s);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator /(const Vector4<DataType, ThreadCount>& v, typename Vector4<DataType, ThreadCount>::ScalarType s)
    {
        return Vector4(v.x / s, v.y / s, v.z / s, v.w / s);
    }    
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator /(typename Vector4<DataType, ThreadCount>::ScalarType s, const Vector4<DataType, ThreadCount>& v)
    {
        return Vector4( s / v.x,  s / v.y,   s / v.z, s / v.w);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator -(const Vector4<DataType, ThreadCount>& v)
    {
        return Vector4(-v.x, -v.y, -v.z, -v.w);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector4<DataType, ThreadCount>::ScalarType SquareMagnitude(const Vector4<DataType, ThreadCount>& v)
    {
        return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector4<DataType, ThreadCount>::ScalarType Magnitude(const Vector4<DataType, ThreadCount>& v)
    {
        typename Vector4<DataType, ThreadCount>::ScalarType r = SquareMagnitude(v);
        
        MATH_SIMT_SIMDIFY_FOR MATH_SIMT_SIMDIFY_ALIGNED(r.m, Vector4<DataType, ThreadCount>::kAlignment)
        for (size_t i = 0; i < Vector3<DataType, ThreadCount>::kThreadCount; i++)
        {
            r.m[i] = std::sqrt(r.m[i]);
        }
        return r;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> Normalize(const Vector4<DataType, ThreadCount>& v)
    {
        return v / Magnitude(v);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator +(const Vector4<DataType, ThreadCount>& a, const Vector4<DataType, ThreadCount>& b)
    {
        return Vector4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator -(const Vector4<DataType, ThreadCount>& a, const Vector4<DataType, ThreadCount>& b)
    {
        return Vector4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator *(const Vector4<DataType, ThreadCount>& a, const Vector4<DataType, ThreadCount>& b)
    {
        return Vector4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator /(const Vector4<DataType, ThreadCount>& a, const Vector4<DataType, ThreadCount>& b)
    {
        return Vector4(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector4<DataType, ThreadCount>::ScalarType Dot(const Vector4<DataType, ThreadCount> a, const Vector4<DataType, ThreadCount> b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE typename Vector4<DataType, ThreadCount>::ScalarType CosTheta(const Vector4<DataType, ThreadCount> a, const Vector4<DataType, ThreadCount> b)
    {
        Vector4<DataType, ThreadCount> aNormalized = Normalize(a);
        Vector4<DataType, ThreadCount> bNormalized = Normalize(b);
        return Dot(aNormalized, bNormalized);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> Project(const Vector4<DataType, ThreadCount>& a, const Vector4<DataType, ThreadCount>& b)
    {
        return b * (Dot(a, b) / Dot(b,b));
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> Reflect(const Vector3<DataType, ThreadCount>& a, const Vector4<DataType, ThreadCount>& b)
    {
        return a - b * (Dot(a, b) / Dot(b,b));
    }
}
