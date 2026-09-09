#pragma once

#include "Vector.h"
#include "Math/Box.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Box3
    {
        using Type = DataType;
        using ScalarType =  Scalar<DataType, ThreadCount>;
        using MaskType = typename Scalar<DataType, ThreadCount>::MaskType;
        using IndexerType = typename Scalar<DataType, ThreadCount>::IndexerType;

        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;
        using Vector = Vector3<DataType, ThreadCount>;
        using Point = Point3<DataType, ThreadCount>;
        
        Point a, b;

        Box3() : a(0), b(0) {}
        Box3(const ScalarType& HalfRadius) : a(-HalfRadius), b(HalfRadius) {}
        Box3(const ScalarType& X, const ScalarType& Y, const ScalarType& Z) : a(Point(-X/2, -Y/2, -Z/2)), b(Point(X/2, Y/2, Z/2)) {}
        Box3(const Point& Center, const ScalarType& HalfRadius) : a(Center - HalfRadius), b(Center + HalfRadius) {}
        Box3(const Point& A, const Point& B) : a(A), b(B) {}
        Box3(const Box3T<Type>& box) : a(box.a), b(box.b) {}
        Box3(const Box3& A, const Box3& B) : a(Min(A.a, B.a)), b(Max(A.b, B.b)) {}

        Box3T<Type> Elt(size_t index) const
        {
            return {a[index], b[index]};
        }

        INLINE Point& operator[] (size_t Index)
        {
            return (Index == 0) ? a : b;
        }
        
        INLINE Point operator[] (size_t Index) const
        {
            return (Index == 0) ? a : b;
        }
        
        INLINE Point Center() const {return (a + b) * 0.5f;}
        INLINE ScalarType Center(size_t Index) const {return (a[Index] + b[Index]) * 0.5;}
        INLINE Vector Diagonal() const {return b - a;}
        INLINE Vector Size() const {return Diagonal();}
        INLINE ScalarType Radius() const {return Magnitude(Diagonal());}
        
        INLINE Point Vertex(size_t k) const {return (Select( b[0], a[0], k & 1), Select(b[1], a[1], k & 2), Select(b[2],a[2], k & 4));}
        INLINE ScalarType Volume() const {Vector size = Size(); return size.x * size.y * size.z;}
        
        INLINE MaskType Inside(const Box3& other) const
        {
            return ((b.x < other.b.x) && (b.y < other.b.y) && (b.z < other.b.z)) && 
                    (a.y > other.a.x) && (a.y > other.a.y) && (a.z > other.a.z);
        }
        
        INLINE MaskType Inside(const Point& point) const
        {
            return ((b.x < point.x) && (b.y < point.y) && (b.z < point.z)) && 
                    (a.y > point.x) && (a.y > point.y) && (a.z > point.z);
        }
        
        Box3& Insert( const Point& p )
        {
            a.x = Min(a.x, p.x); 
            a.y = Min(a.y, p.y); 
            a.z = Min(a.z, p.z); 
            
            b.x = Max(b.x, p.x); 
            b.y = Max(b.y, p.y); 
            b.z = Max(b.z, p.z); 
            
            return *this;
        }
        
        Box3& Insert( const Box3& box )
        {
            a.x = Min(a.x, box.a.x); 
            a.y = Min(a.y, box.a.y); 
            a.z = Min(a.z, box.a.z); 
            
            b.x = Max(b.x, box.b.x); 
            b.y = Max(b.y, box.b.y); 
            b.z = Max(b.z, box.b.z); 
            
            return *this;
        }
        
        void Translate(const Vector& t)
        {
            a += t;
            b += t;
        }
        
        void Scale(const ScalarType& s)
        {
            a *= s;
            b *= s;
            
            // Swap coordinates for negative coefficients
            Point c = a;
            MaskType mask = s < 0;
            a.x = Select(b.x, a.x, mask);
            a.y = Select(b.y, a.y, mask);
            a.z = Select(b.z, a.z, mask);
            b.x = Select(c.x, b.x, mask);
            b.y = Select(c.y, b.y, mask);
            b.z = Select(c.z, b.z, mask);
        }

        /**
         * Computes the sub-box in the n-th octant.
         * @param Index Octant index.
         * @return sub box
         */
        Box3 Sub(size_t Index) const
        {
            Point c = Center();
            return Box3(
                Point(Select(c[0], a[0], Index & 1), Select(c[1], a[1], Index & 2), Select(c[2], a[2], Index & 4)),
                Point(Select(b[0], c[0], Index & 1), Select(b[1], c[1], Index & 2), Select(b[2], c[2], Index & 4))
                );
        }
    };

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Box3<DataType, ThreadCount>::MaskType operator==(const Box3<DataType, ThreadCount>& A, const Box3<DataType, ThreadCount>& B)
    {
        return (A.a == B.a) && (A.b == B.b);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Box3<DataType, ThreadCount>::MaskType operator!=(const Box3<DataType, ThreadCount>& A, const Box3<DataType, ThreadCount>& B)
    {
        return !(A == B);
    }
}