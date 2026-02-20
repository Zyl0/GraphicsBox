#pragma once

#ifdef PLATFORM_LINUX
#include <stddef.h> 
#endif
#include <cmath>
#include <type_traits>
#include <algorithm>

#include "Shared/Annotations.h"


namespace Math
{
    template<typename type>
    struct Vector2t
    {
        using Type = type;
        
        type x, y;

        Vector2t() = default;

        Vector2t(const type a) : x(a), y(a) {}

        Vector2t(const type a, const type b) : x(a), y(b) {}

        type& operator[](const int i)
            {return (&x)[i];}
        
        const type& operator[](const int i) const
            {return (&x)[i];}

        type& operator[](const size_t i)
            {return (&x)[i];}
        
        const type& operator[](const size_t i) const
            {return (&x)[i];}

        const type* data() const
            {return &x;}

        Vector2t& operator *=(float s)
        {
            x *= s;
            y *= s;
            return *this;
        }

        Vector2t& operator /=(float s)
        {
            x /= s;
            y /= s;
            return *this;
        }

        Vector2t& operator +=(const Vector2t& v)
        {
            x += v.x;
            y += v.y;
            return *this;
        }

        Vector2t& operator -=(const Vector2t& v)
        {
            x -= v.x;
            y -= v.y;
            return *this;
        }

        Vector2t& operator *=(const Vector2t& v)
        {
            x *= v.x;
            y *= v.y;
            return *this;
        }

        Vector2t& operator /=(const Vector2t& v)
        {
            x /= v.x;
            y /= v.y;
            return *this;
        }
    };

    template<typename type>
    inline Vector2t<type> operator *(const Vector2t<type>& v, float s)
        {return Vector2t(v.x * s, v.y * s);}

    template<typename type>
    inline Vector2t<type> operator /(const Vector2t<type>& v, float s)
        {return Vector2t(v.x / s, v.y / s);}

    template<typename type>
    inline Vector2t<type> operator -(const Vector2t<type>& v)
        {return Vector2t(-v.x, -v.y);}

    template<typename type>
    inline type Magnitude(const Vector2t<type>& v)
        {return std::sqrt(v.x * v.x + v.y * v.y);}

    template<typename type>
    inline Vector2t<type> Normalize(const Vector2t<type>& v)
        {return v / Magnitude(v);}

    template<typename type>
    inline Vector2t<type> operator +(const Vector2t<type>& a, const Vector2t<type>& b)
        {return Vector2t(a.x + b.x, a.y + b.y);}

    template<typename type>
    inline Vector2t<type> operator -(const Vector2t<type>& a, const Vector2t<type>& b)
        {return Vector2t(a.x - b.x, a.y - b.y);}

    template<typename type>
    inline Vector2t<type> operator *(const Vector2t<type>& a, const Vector2t<type>& b)
        {return Vector2t(a.x * b.x, a.y * b.y);}

    template<typename type>
    inline Vector2t<type> operator /(const Vector2t<type>& a, const Vector2t<type>& b)
        {return Vector2t(a.x / b.x, a.y / b.y);}

    template<typename type>
    inline type Dot(const Vector2t<type> a, const Vector2t<type> b)
        {return a.x * b.x + a.y * b.y;}

    template<typename type>
    type CosTheta(const Vector2t<type> a, const Vector2t<type> b)
    {
        Vector2t<type> aNormalized = Normalize(a);
        Vector2t<type> bNormalized = Normalize(b);
        return Dot(aNormalized, bNormalized);
    }
    
    template<typename type>
    struct Vector3t
    {
        using Type = type;
        
        type x, y, z;

        Vector3t() = default;

        Vector3t(const type v) : x(v), y(v), z(v) {}

        Vector3t(const type a, const type b, const type c) : x(a), y(b), z(c) {}

        Vector3t(const Vector3t& Other) : x(Other.x), y(Other.y),  z(Other.z) {}

        Vector3t(Vector3t&& Other) noexcept : x(std::move(Other.x)),  y(std::move(Other.y)),  z(std::move(Other.z)) {}

        Vector3t& operator=(const Vector3t& Other)
        {
            if (this == &Other)
                return *this;
            x = Other.x;
            y = Other.y;
            z = Other.z;
            return *this;
        }

        Vector3t& operator=(Vector3t&& Other) noexcept
        {
            if (this == &Other)
                return *this;
            x = std::move(Other.x);
            y = std::move(Other.y);
            z = std::move(Other.z);
            return *this;
        }

        type& operator[](const int i)
            {return (&x)[i];}
        
        const type& operator[](const int i) const
            {return (&x)[i];}

        type& operator[](const size_t i)
            {return (&x)[i];}
        
        const type& operator[](const size_t i) const
            {return (&x)[i];}

        const type* data() const
            {return &x;}

        type* data()
            {return &x;}

        Vector3t& operator *=(float s)
        {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }

        Vector3t& operator /=(float s)
        {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }

        Vector3t& operator +=(const Vector3t& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        Vector3t& operator -=(const Vector3t& v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }

        Vector3t& operator *=(const Vector3t& v)
        {
            x *= v.x;
            y *= v.y;
            z *= v.z;
            return *this;
        }

        Vector3t& operator /=(const Vector3t& v)
        {
            x /= v.x;
            y /= v.y;
            z /= v.z;
            return *this;
        }

        bool operator==(const Vector3t& other) const
        {
            return (x == other.x && y == other.y && z == other.z);
        }
    };

    template<typename type>
    inline Vector3t<type> operator *(const Vector3t<type>& v, type s)
    {return Vector3t(v.x * s, v.y * s, v.z * s);}

    template<typename type>
    inline Vector3t<type> operator *(type s, const Vector3t<type>& v)
    {return Vector3t(v.x * s, v.y * s, v.z * s);}

    template<typename type>
    inline Vector3t<type> operator /(const Vector3t<type>& v, float s)
        {return Vector3t(v.x / s, v.y / s, v.z / s);}
    
    template<typename type>
    inline Vector3t<type> operator -(const Vector3t<type>& v)
        {return Vector3t(-v.x, -v.y, -v.z);}

    template<typename type>
    inline type Magnitude(const Vector3t<type>& v)
        {return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);}

    template<typename type>
    inline type SquareMagnitude(const Vector3t<type>& v)
        {return v.x * v.x + v.y * v.y + v.z * v.z;}

    template<typename type>
    inline Vector3t<type> Normalize(const Vector3t<type>& v)
        {return v / Magnitude(v);}

    template<typename type>
    inline Vector3t<type> operator +(const Vector3t<type>& a,  type s)
        {return Vector3t(a.x + s, a.y + s, a.z + s);}

    template<typename type>
    inline Vector3t<type> operator -(const Vector3t<type>& a,  type s)
        {return Vector3t(a.x - s, a.y - s, a.z - s);}

    template<typename type>
    inline Vector3t<type> operator -(type s, const Vector3t<type>& a)
        {return Vector3t(s - a.x, s - a.y, s - a.z);}

    template<typename type>
    inline Vector3t<type> operator +(const Vector3t<type>& a, const Vector3t<type>& b)
        {return Vector3t(a.x + b.x, a.y + b.y, a.z + b.z);}

    template<typename type>
    inline Vector3t<type> operator -(const Vector3t<type>& a, const Vector3t<type>& b)
        {return Vector3t(a.x - b.x, a.y - b.y, a.z - b.z);}

    template<typename type>
    inline Vector3t<type> operator *(const Vector3t<type>& a, const Vector3t<type>& b)
        {return Vector3t(a.x * b.x, a.y * b.y, a.z * b.z);}

    template<typename type>
    inline Vector3t<type> operator /(const Vector3t<type>& a, const Vector3t<type>& b)
        {return Vector3t(a.x / b.x, a.y / b.y, a.z / b.z);}


    template<typename type>
    inline type Dot(const Vector3t<type> a, const Vector3t<type> b)
        {return a.x * b.x + a.y * b.y + a.z * b.z;}

    template<typename type>
    type CosTheta(const Vector3t<type> a, const Vector3t<type> b)
    {
        Vector3t<type> aNormalized = Normalize(a);
        Vector3t<type> bNormalized = Normalize(b);
        return Dot(aNormalized, bNormalized);
    }

    template<typename type>
    inline Vector3t<type> Cross(const Vector3t<type>& a, const Vector3t<type>& b)
    {
        return Vector3t(   a.y * b.z - a.z * b.y,
                            a.z * b.x - a.x * b.z,
                            a.x * b.y - a.y * b.x);
    }

    template<typename type>
    inline Vector3t<type> Project(const Vector3t<type>& a, const Vector3t<type>& b)
    {
        return b * (Dot(a, b) / Dot(b,b));
    }

    template<typename type>
    inline Vector3t<type> Reflect(const Vector3t<type>& a, const Vector3t<type>& b)
    {
        return a - b * (Dot(a, b) / Dot(b,b));
    }

    template<typename type>
    struct Point3t : Vector3t<type>
    {
        using Type = type;
        
        Point3t() : Vector3t<type>(0) {}

        Point3t(type a, type b, type c) : Vector3t<type>(a, b, c) {}

        Point3t(const Point3t& Other)
            : Vector3t<type>(Other)
        {
        }

        Point3t(Point3t&& Other) noexcept
            : Vector3t<type>(std::move(Other))
        {
        }

        Point3t& operator=(const Point3t& Other)
        {
            if (this == &Other)
                return *this;
            Vector3t<type>::operator =(Other);
            return *this;
        }

        Point3t& operator=(Point3t&& Other) noexcept
        {
            if (this == &Other)
                return *this;
            Vector3t<type>::operator =(std::move(Other));
            return *this;
        }

        Point3t(const Vector3t<type>& Other)
        : Vector3t<type>(Other)
        {
        }

        Point3t(Vector3t<type>&& Other) noexcept
            : Vector3t<type>(std::move(Other))
        {
        }

        Point3t& operator=(const Vector3t<type>& Other)
        {
            if (this == &Other)
                return *this;
            Vector3t<type>::operator =(Other);
            return *this;
        }

        Point3t& operator=(Vector3t<type>&& Other) noexcept
        {
            if (this == &Other)
                return *this;
            Vector3t<type>::operator =(std::move(Other));
            return *this;
        }
    };

    template<typename type>
    inline Point3t<type> operator +(const Point3t<type> &a, const Point3t<type> &b)
    {
        return  Point3t<type>(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    template<typename type>
    inline Point3t<type> operator +(const Point3t<type> &p, const Vector3t<type> &v)
    {
        return  Point3t<type>(p.x + v.x, p.y + v.y, p.z + v.z);
    }

    template<typename type>
    inline Point3t<type> operator -(const Point3t<type> &a, const Point3t<type> &b)
    {
        return  Point3t<type>(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    template<typename type>
    inline Point3t<type> operator -(const Point3t<type> &p, const Vector3t<type> &v)
    {
        return  Point3t<type>(p.x - v.x, p.y - v.y, p.z - v.z);
    }

    template<typename type>
    float Distance( const Point3t<type>& a, const Point3t<type>& b )
    {
        return Magnitude(a - b);
    }

    template<typename type>
    float SquareDistance( const Point3t<type>& a, const Point3t<type>& b )
    {
        return SquareMagnitude(a - b);
    }

    template<typename type>
    Point3t<type> Center( const Point3t<type>& a, const Point3t<type>& b )
    {
        return Point3t<type>((a.x + b.x) / 2, (a.y + b.y) / 2, (a.z + b.z) / 2);
    }

    template<typename type>
    Point3t<type> Min( const Point3t<type>& a, Point3t<type>& b )
    { 
        return Point3t<type>( std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z) ); 
    }

    template<typename type>
    Point3t<type> Max( const Point3t<type>& a, const Point3t<type>& b ) 
    { 
        return Point3t<type>( std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z) ); 
    }

    template<typename type>
    struct Vector4t
    {
        using Type = type;
        
        type x, y, z, w;

        Vector4t() = default;

        Vector4t(const type s) : x(s), y(s), z(s), w(s) {}

        Vector4t(const Vector3t<type> &v3, const type d) : x(v3.x), y(v3.y), z(v3.z), w(d) {}

        Vector4t(const type a, const type b, const type c, const type d) : x(a), y(b), z(c), w(d) {}
        
        Vector3t<type>& xyz()
            {return *reinterpret_cast<Vector3t<type>*>(this);}
        
        const Vector3t<type>& xyz() const
            {return *reinterpret_cast<const Vector3t<type>*>(this);}

        type& operator[](const int i)
            {return (&x)[i];}
        
        const type& operator[](const int i) const
            {return (&x)[i];}

        type& operator[](const size_t i)
            {return (&x)[i];}
        
        const type& operator[](const size_t i) const
            {return (&x)[i];}

        const type* data() const
            {return &x;}

        type* data()        
            {return &x;}

        Vector4t& operator *=(float s)
        {
            x *= s;
            y *= s;
            z *= s;
            w *= s;
            return *this;
        }

        Vector4t& operator /=(float s)
        {
            x /= s;
            y /= s;
            z /= s;
            w /= s;
            return *this;
        }

        Vector4t& operator +=(const Vector4t& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            w += v.w;
            return *this;
        }

        Vector4t& operator -=(const Vector4t& v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            w -= v.w;
            return *this;
        }

        INLINE Vector3t<type> XXX() const {return Vector3t<type>(x, x, x);}
        INLINE Vector3t<type> YXX() const {return Vector3t<type>(y, x, x);}
        INLINE Vector3t<type> ZXX() const {return Vector3t<type>(z, x, x);}
        INLINE Vector3t<type> XYX() const {return Vector3t<type>(x, y, x);}
        INLINE Vector3t<type> YYX() const {return Vector3t<type>(y, y, x);}
        INLINE Vector3t<type> ZYX() const {return Vector3t<type>(z, y, x);}
        INLINE Vector3t<type> XZX() const {return Vector3t<type>(x, z, x);}
        INLINE Vector3t<type> YZX() const {return Vector3t<type>(y, z, x);}
        INLINE Vector3t<type> ZZX() const {return Vector3t<type>(z, z, x);}
        
        INLINE Vector3t<type> XXY() const {return Vector3t<type>(x, x, y);}
        INLINE Vector3t<type> YXY() const {return Vector3t<type>(y, x, y);}
        INLINE Vector3t<type> ZXY() const {return Vector3t<type>(z, x, y);}
        INLINE Vector3t<type> XYY() const {return Vector3t<type>(x, y, y);}
        INLINE Vector3t<type> YYY() const {return Vector3t<type>(y, y, y);}
        INLINE Vector3t<type> ZYY() const {return Vector3t<type>(z, y, y);}
        INLINE Vector3t<type> XZY() const {return Vector3t<type>(x, z, y);}
        INLINE Vector3t<type> YZY() const {return Vector3t<type>(y, z, y);}
        INLINE Vector3t<type> ZZY() const {return Vector3t<type>(z, z, y);}

        INLINE Vector3t<type> XXZ() const {return Vector3t<type>(x, x, z);}
        INLINE Vector3t<type> YXZ() const {return Vector3t<type>(y, x, z);}
        INLINE Vector3t<type> ZXZ() const {return Vector3t<type>(z, x, z);}
        INLINE Vector3t<type> XYZ() const {return Vector3t<type>(x, y, z);}
        INLINE Vector3t<type> YYZ() const {return Vector3t<type>(y, y, z);}
        INLINE Vector3t<type> ZYZ() const {return Vector3t<type>(z, y, z);}
        INLINE Vector3t<type> XZZ() const {return Vector3t<type>(x, z, z);}
        INLINE Vector3t<type> YZZ() const {return Vector3t<type>(y, z, z);}
        INLINE Vector3t<type> ZZZ() const {return Vector3t<type>(z, z, z);}
    };

    template<typename type>
    inline Vector4t<type> operator *(const Vector4t<type>& v, type s)
        {return Vector4t(v.x * s, v.y * s, v.z * s, v.w * s);}

    template<typename type>
    inline Vector4t<type> operator *(type s, const Vector4t<type>& v)
    {return Vector4t(v.x * s, v.y * s, v.z * s, v.w * s);}

    template<typename type>
    inline Vector4t<type> operator /(const Vector4t<type>& v, float s)
        {return Vector4t(v.x / s, v.y / s, v.z / s, v.w / s);}

    template<typename type>
    inline Vector4t<type> operator -(const Vector4t<type>& v)
        {return Vector4t(-v.x, -v.y, -v.z, -v.w);}

    template<typename type>
    inline type Magnitude(const Vector4t<type>& v)
        {return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);}

    template<typename type>
    inline Vector4t<type> Normalize(const Vector4t<type>& v)
        {return v / Magnitude(v);}

    template<typename type>
    inline Vector4t<type> operator +(const Vector4t<type>& a, const Vector4t<type>& b)
        {return Vector4t(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);}

    template<typename type>
    inline Vector4t<type> operator -(const Vector4t<type>& a, const Vector4t<type>& b)
        {return Vector4t(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);}

    template<typename type>
    inline Vector4t<type> operator *(const Vector4t<type>& a, const Vector4t<type>& b)
        {return Vector4t(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);}

    template<typename type>
    inline Vector4t<type> operator /(const Vector4t<type>& a, const Vector4t<type>& b)
        {return Vector4t(a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w);}

    template<typename type>
    inline type Dot(const Vector4t<type> a, const Vector4t<type> b)
        {return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;}

    template<typename type>
    type CosTheta(const Vector4t<type> a, const Vector4t<type> b)
    {
        Vector4t<type> aNormalized = Normalize(a);
        Vector4t<type> bNormalized = Normalize(b);
        return Dot(aNormalized, bNormalized);
    }

    template<typename type>
    inline Vector4t<type> Project(const Vector4t<type>& a, const Vector4t<type>& b)
    {
        return b * (Dot(a, b) / Dot(b,b));
    }

    template<typename type>
    inline Vector4t<type> Reflect(const Vector3t<type>& a, const Vector4t<type>& b)
    {
        return a - b * (Dot(a, b) / Dot(b,b));
    }

    using Vector2i = Vector2t<int>;
    using Vector2f = Vector2t<float>;
    using Vector2d = Vector2t<double>;

    using Vector3i = Vector3t<int>;
    using Vector3f = Vector3t<float>;
    using Vector3d = Vector3t<double>;

    using Point3i = Point3t<int>;
    using Point3f = Point3t<float>;
    using Point3d = Point3t<double>;

    using Vector4i = Vector4t<int>;
    using Vector4f = Vector4t<float>;
    using Vector4d = Vector4t<double>;
}


