#pragma once

#include "Vector.h"

namespace Math
{
    namespace Box3
    {
        extern const size_t Edge[24];
        extern const Vector3d Normal[24];
    }
    
    template<typename type>
    struct Box3T
    {
        using Type = type;
        using Vector = Vector3t<Type>;
        using Point = Point3t<Type>;
        
        static constexpr Type Epsilon = std::numeric_limits<type>::epsilon();
        
        Point a, b;
        
        Box3T() : a(0), b(0) {}
        Box3T(Type HalfRadius) : a(-HalfRadius), b(HalfRadius) {}
        Box3T(Type X, Type Y, Type Z) : a(Point(-X/2, -Y/2, -Z/2)), b(Point(X/2, Y/2, Z/2)) {}
        Box3T(Point Center, Type HalfRadius) : a(Center - HalfRadius), b(Center + HalfRadius) {}
        Box3T(Point A, Point B) : a(A), b(B) {}
        Box3T(const Box3T& A, const Box3T& B) : a(Min(A.a, B.a)), b(Max(A.b, B.b)) {}
        
        INLINE Point& operator[] (size_t Index)
        {
            return (Index == 0) ? a : b;
        }
        
        INLINE Point operator[] (size_t Index) const
        {
            return (Index == 0) ? a : b;
        }
        
        INLINE Point Center() const {return 0.5 * (a + b);}
        INLINE Vector Diagonal() const {return b - a;}
        INLINE Vector Size() const {return Diagonal();}
        INLINE Type Radius() const {return Magnitude(Diagonal());}
        
        INLINE Point Vertex(size_t k) const {return  ((k & 1) ? b[0] : a[0], (k & 2) ? b[1] : a[1], (k & 4) ? b[2] : a[2]);}
        INLINE Type Volume() const {Vector Size = Size(); return Size.x * Size.y * Size.z;}
        
        INLINE bool Inside(const Box3T& other) const {return ((a < other.a) && (b > other.b));}
        INLINE bool Inside(const Vector& point) const {return (b < point.a) && (a > point.b);}
        
        void Translate(const Vector& t)
        {
            a += t;
            b += t;
        }
        
        void Scale(double s)
        {
            a *= s;
            b *= s;
            
            // Swap coordinates for negative coefficients 
            if (s < 0.0)
            {
                std::swap(a, b);
            }
        }

        /**
         * Computes the sub-box in the n-th octant.
         * @param Index Octant index.
         * @return sub box
         */
        Box3T Sub(size_t Index) const
        {
            Point c = Center();
            return Box3T(
                Point((Index & 1) ? c[0] : a[0], (Index & 2) ? c[1] : a[1], (Index & 4) ? c[2] : a[2]),
                Point((Index & 1) ? b[0] : c[0], (Index & 2) ? b[1] : c[1], (Index & 4) ? b[2] : c[2])
                );
        }
    };
    
    template<typename type>
    bool operator==(const Box3T<type>& A, const Box3T<type>& B)
    {
        return (A.a == B.a) && (A.b == B.b);
    }
    
    template<typename type>
    bool operator!=(const Box3T<type>& A, const Box3T<type>& B)
    {
        return !(A == B);
    }
    
    using Box3f = Box3T<float>;
    using Box3d = Box3T<double>;
}
