#pragma once

#include "Line.h"
#include "Types.h"
#include "Matrix.h"
#include "Plane.h"
#include "Math/Transforms.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Transform4 : Matrix4<DataType, ThreadCount>
    {
        Transform4(): Matrix4<DataType, ThreadCount>() {}
        
        Transform4(
                DataType n00, DataType n01, DataType n02, DataType n03,
                DataType n10, DataType n11, DataType n12, DataType n13,
                DataType n20, DataType n21, DataType n22, DataType n23) :
            Matrix4<DataType, ThreadCount>(
                    n00,            n01,	        n02,	        n03,
                    n10,	        n11,	        n12,	        n13,
                    n20,	        n21,	        n22,	        n23,
                    DataType(0),	DataType(0),  	DataType(0),  	DataType(1)
                )
        {}
        
        Transform4(
                const Scalar<DataType, ThreadCount>& n00, const Scalar<DataType, ThreadCount>& n01, const Scalar<DataType, ThreadCount>& n02, const Scalar<DataType, ThreadCount>& n03,
                const Scalar<DataType, ThreadCount>& n10, const Scalar<DataType, ThreadCount>& n11, const Scalar<DataType, ThreadCount>& n12, const Scalar<DataType, ThreadCount>& n13,
                const Scalar<DataType, ThreadCount>& n20, const Scalar<DataType, ThreadCount>& n21, const Scalar<DataType, ThreadCount>& n22, const Scalar<DataType, ThreadCount>& n23) :
            Matrix4<DataType, ThreadCount>(
                    n00,            n01,	        n02,	        n03,
                    n10,	        n11,	        n12,	        n13,
                    n20,	        n21,	        n22,	        n23,
                    DataType(0),	DataType(0),  	DataType(0),  	DataType(1)
                )
        {}
        
        Transform4(const Matrix4t<DataType>& m) :
           Matrix4<DataType, ThreadCount>(
               m(0,0), m(0,1),	m(0,2),	m(0,3),
               m(1,0),	m(1,1),	m(1,2),	m(1,3),
               m(2,0),	m(2,1),	m(2,2),	m(2,3),
               m(3,0), m(3,1), m(3,2), m(3,3)
           )
        {}
        
        Transform4(const Matrix4<DataType, ThreadCount>& m) :
           Matrix4<DataType, ThreadCount>(
               m(0,0), m(0,1),	m(0,2),	m(0,3),
               m(1,0),	m(1,1),	m(1,2),	m(1,3),
               m(2,0),	m(2,1),	m(2,2),	m(2,3),
               m(3,0), m(3,1), m(3,2), m(3,3)
           )
        {}
        
        Transform4( const Vector3<DataType, ThreadCount>& x, const Vector3<DataType, ThreadCount>& y, const Vector3<DataType, ThreadCount>& z, const Vector3<DataType, ThreadCount>& t ) :
            Matrix4<DataType, ThreadCount>(
                x.x,            x.y,	        x.z,	        t.x,
                x.y,	        y.y,	        z.y,	        t.y,
                x.z,	        y.z,	        z.z,	        t.z,
                DataType(0),	DataType(0),  	DataType(0),  	DataType(1)
            )
        {}
        
        Transform4( const Vector3t<DataType>& x, const Vector3t<DataType>& y, const Vector3t<DataType>& z, const Vector3t<DataType>& t ) :
            Matrix4<DataType, ThreadCount>(
                x.x,            x.y,	        x.z,	        t.x,
                x.y,	        y.y,	        z.y,	        t.y,
                x.z,	        y.z,	        z.z,	        t.z,
                DataType(0),	DataType(0),  	DataType(0),  	DataType(1)
            )
        {}
        
        Transform4( const Matrix3<DataType, ThreadCount>& M, const Vector3<DataType, ThreadCount>& t ) :
            Matrix4<DataType, ThreadCount>(
                M(0,0),     M(0,1),	    M(0,2),	    t.x,
                M(1,0),	    M(1,1),	    M(1,2),	    t.y,
                M(2,0),	    M(2,1),	    M(2,2),	    t.z,
                DataType(0),	DataType(0),  	DataType(0),  	DataType(1)
            )
        {}
        
        Transform4( const Matrix3<DataType, ThreadCount>& M ) :
            Matrix4<DataType, ThreadCount>(
                M(0,0),     M(0,1),     M(0,2),     DataType(0),
                M(1,0),	    M(1,1),     M(1,2),     DataType(0),
                M(2,0),	    M(2,1),     M(2,2),     DataType(0),
                DataType(0),	DataType(0),  	DataType(0),  	DataType(1)
            )
        {}
        
        Transform4( const Matrix3t<DataType>& M, const Vector3t<DataType>& t ) :
            Matrix4<DataType, ThreadCount>(
                M(0,0),     M(0,1),	    M(0,2),	    t.x,
                M(1,0),	    M(1,1),	    M(1,2),	    t.y,
                M(2,0),	    M(2,1),	    M(2,2),	    t.z,
                DataType(0),	DataType(0),  	DataType(0),  	DataType(1)
            )
        {}
        
        Transform4( const Matrix3t<DataType>& M ) :
            Matrix4<DataType, ThreadCount>(
                M(0,0),     M(0,1),	    M(0,2),	    DataType(0),
                M(1,0),	    M(1,1),	    M(1,2),	    DataType(0),
                M(2,0),	    M(2,1),	    M(2,2),	    DataType(0),
                DataType(0),	DataType(0),  	DataType(0),  	DataType(1)
            )
        {}
        
        Transform4(const Transform4t<DataType>& m) : Transform4(reinterpret_cast<Matrix4t<DataType>>(m)) {}
        
        INLINE Vector3<DataType, ThreadCount>& operator [](int j)
        {
            return *reinterpret_cast<Vector3<DataType, ThreadCount>*>(this->n[j]);
        }

        INLINE const Vector3<DataType, ThreadCount>& operator [](int j) const
        {
            return *reinterpret_cast<const Vector3<DataType, ThreadCount>*>(this->n[j]);
        }

        INLINE const Point3<DataType, ThreadCount>& GetTranslation()
        {
            return *reinterpret_cast<const Point3<DataType, ThreadCount>>(this->n[3]);
        }

        INLINE void SetTranslation(const Point3<DataType, ThreadCount>&p)
        {
            this->n[3][0] = p.x;
            this->n[3][1] = p.y;
            this->n[3][2] = p.z;
        }
        
        INLINE static Transform4 Translation(const Scalar<DataType, ThreadCount>& tx, const Scalar<DataType, ThreadCount>& ty, const Scalar<DataType, ThreadCount>& tz)
        {            
            return Transform4(
                DataType(1),    DataType(0),    DataType(0),    tx,
                DataType(0),    DataType(1),    DataType(0),    ty,
                DataType(0),    DataType(0),    DataType(1),    tz
            );
        }
        
        INLINE static Transform4 Translation(const Vector3<DataType, ThreadCount>& t)
        {            
            return Transform4(
                1,              DataType(0),    DataType(0),    t.x,
                DataType(0),    1,              DataType(0),    t.y,
                DataType(0),    DataType(0),    1,              t.z
            );
        }
        
        INLINE static Transform4 RotationX(const Scalar<DataType, ThreadCount>& t)
        {
            Scalar<DataType, ThreadCount> c = Cos(t);
            Scalar<DataType, ThreadCount> s = Sin(t);
            
            return Transform4(
                1,              DataType(0),    DataType(0),    DataType(0),
                DataType(0),    c,              -s,             DataType(0),
                DataType(0),    -s,             c,              DataType(0)
            );
        }
        
        INLINE static Transform4 RotationY(const Scalar<DataType, ThreadCount>& t)
        {
            Scalar<DataType, ThreadCount> c = Cos(t);
            Scalar<DataType, ThreadCount> s = Sin(t);
            
            return Transform4(
                c,              DataType(0),    s,              DataType(0),
                DataType(0),    1,              DataType(0),    DataType(0),
                -s,             DataType(0),    c,              DataType(0)
            );
        }
        
        INLINE static Transform4 RotationZ(const Scalar<DataType, ThreadCount>& t)
        {
            Scalar<DataType, ThreadCount> c = Cos(t);
            Scalar<DataType, ThreadCount> s = Sin(t);
            
            return Transform4(
                c,              -s,             DataType(0),    DataType(0),
                -s,             c,              DataType(0),    DataType(0),
                DataType(0),    DataType(0),    1,              DataType(0)
            );
        }
        
        INLINE static Transform4 Rotation(const Vector3<DataType, ThreadCount>& axis, const Scalar<DataType, ThreadCount>& t)
        {        
            Scalar<DataType, ThreadCount> c = Cos(t);
            Scalar<DataType, ThreadCount> s = Sin(t);
            Scalar<DataType, ThreadCount> d = DataType(1) - c;

            Scalar<DataType, ThreadCount> x = axis.x * d;
            Scalar<DataType, ThreadCount> y = axis.y * d;
            Scalar<DataType, ThreadCount> z = axis.z * d;

            Scalar<DataType, ThreadCount> axay = x * axis.y;
            Scalar<DataType, ThreadCount> axaz = x * axis.z;
            Scalar<DataType, ThreadCount> ayaz = y * axis.z;
            
            return Transform4(
                c +  x * axis.x,    axay - s * axis.z,  axaz + s * axis.y,  DataType(0),
                axay + s * axis.z,  c + y * axis.y,     ayaz - s * axis.x,  DataType(0),
                axaz - s * axis.y,  ayaz + s * axis.x,  c + z * axis.x,     DataType(0)
            );
        }
        
        INLINE static Transform4 Reflection(const Vector3<DataType, ThreadCount>& axis)
        {
            Scalar<DataType, ThreadCount> x = axis.x * DataType(-2);
            Scalar<DataType, ThreadCount> y = axis.y * DataType(-2);
            Scalar<DataType, ThreadCount> z = axis.z * DataType(-2);
            
            Scalar<DataType, ThreadCount> axay = x * axis.y;
            Scalar<DataType, ThreadCount> axaz = x * axis.z;
            Scalar<DataType, ThreadCount> ayaz = y * axis.z; 
            
            return Transform4(
                x * axis.x + DataType(1),   axay,                       axaz,                       DataType(0),
                axay,                       y * axis.y + DataType(1),   ayaz,                       DataType(0),
                axaz,                       ayaz,                       z * axis.z + DataType(1),   DataType(0)
            );
        }
        
        INLINE static Transform4 Reflection(const Plane<DataType, ThreadCount>& f)
        {
            Scalar<DataType, ThreadCount> x = f.x * DataType(-2);
            Scalar<DataType, ThreadCount> y = f.y * DataType(-2);
            Scalar<DataType, ThreadCount> z = f.z * DataType(-2);

            Scalar<DataType, ThreadCount> nxny = x * f.y;
            Scalar<DataType, ThreadCount> nxnz = x * f.z;
            Scalar<DataType, ThreadCount> nynz = y * f.z;
            
            return Transform4(
                x * f.x + DataType(1),  nxny,                   nxnz,                   x * f.w,
                nxny,                   y * f.y + DataType(1),  nynz,                   y * f.w,
                nxnz,                   nynz,                   z * f.z + DataType(1),  z * f.w
            );
        }
        
        INLINE static Transform4 Invocation(const Scalar<DataType, ThreadCount>& axis)
        {
            Scalar<DataType, ThreadCount> x = axis.x * DataType(2);
            Scalar<DataType, ThreadCount> y = axis.y * DataType(2);
            Scalar<DataType, ThreadCount> z = axis.z * DataType(2);
            
            Scalar<DataType, ThreadCount> axay = x * axis.y;
            Scalar<DataType, ThreadCount> axaz = x * axis.z;
            Scalar<DataType, ThreadCount> ayaz = y * axis.z; 
            
            return Transform4(
                x * axis.x - DataType(1),   axay,                       axaz,                       DataType(0),
                axay,                       y * axis.y - DataType(1),   ayaz,                       DataType(0),
                axaz,                       ayaz,                       z * axis.z - DataType(1),   DataType(0)
            );
        }
        
        INLINE static Transform4 Perspective(const Scalar<DataType, ThreadCount>& fieldOfView, const Scalar<DataType, ThreadCount>& aspectRatio, const Scalar<DataType, ThreadCount>& zNear, const Scalar<DataType, ThreadCount>& zFar)
        {
            Scalar<DataType, ThreadCount> itan = DataType(1) / Tan(fieldOfView * DataType(0.5));
            Scalar<DataType, ThreadCount> id = DataType(1) / (zNear - zFar);
            
            return Transform4(
                Matrix4<DataType, ThreadCount>(
                    itan/aspectRatio,   DataType(0),    DataType(0),        DataType(0),
                    DataType(0),        itan,           DataType(0),        DataType(0),
                    DataType(0),        DataType(0),    (zFar+zNear)*id,    DataType(2)*zFar*zNear*id,
                    DataType(0),        DataType(0),    DataType(-1),       DataType(0)
                ));
        }
    };

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Transform4<DataType, ThreadCount> Select(
        const Transform4<DataType, ThreadCount>& A,
        const Transform4<DataType, ThreadCount>& B,
        const typename Transform4<DataType, ThreadCount>::MaskType& mask)
    {
        Transform4<DataType, ThreadCount> r;

        for (size_t i = 0; i < Transform4<DataType, ThreadCount>::kRowCount; ++i)
            for (size_t j = 0; j < Transform4<DataType, ThreadCount>::kColumnCount - 1; ++j)
                r(j, i) = Select(A(j, i), B(j, i), mask);

        return r;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Transform4<DataType, ThreadCount> Inverse(const Transform4<DataType, ThreadCount>& H)
    {
        const Vector3<DataType, ThreadCount> &a = H[0];
        const Vector3<DataType, ThreadCount> &b = H[1];
        const Vector3<DataType, ThreadCount> &c = H[2];
        const Vector3<DataType, ThreadCount> &d = H[3];

        Vector3<DataType, ThreadCount> s = Cross(a, b);
        Vector3<DataType, ThreadCount> t = Cross(c, d);

        Scalar<DataType, ThreadCount> Det = Dot(s, c);
        Scalar<DataType, ThreadCount> invDet = DataType(1) / Det;

        s *= invDet;
        t *= invDet;
        Vector3<DataType, ThreadCount> v = c * invDet;
        
        Vector3<DataType, ThreadCount> r0 = Cross(b, v);
        Vector3<DataType, ThreadCount> r1 = Cross(v, a);

        return Transform4<DataType, ThreadCount>(
            r0.x,   r0.y,   r0.z,   -Dot(b, t),
            r1.x,   r1.y,   r1.z,   Dot(a, t),
            s.x,    s.y,    s.z,    -Dot(d,s)
        );
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Transform4<DataType, ThreadCount> operator *(const Transform4<DataType, ThreadCount> &A, const Transform4<DataType, ThreadCount> &B)
    {
        /*
        return Transform4<DataType, ThreadCount>(
            A(0,0) * B(0,0) + A(0,1) * B(1,0) + A(0,2) * B(2,0),
            A(0,0) * B(0,1) + A(0,1) * B(1,1) + A(0,2) * B(2,1),
            A(0,0) * B(0,2) + A(0,1) * B(1,2) + A(0,2) * B(2,2),
            A(0,0) * B(0,3) + A(0,1) * B(1,3) + A(0,2) * B(2,3) + A(0,3),
                        
            A(1,0) * B(0,0) + A(1,1) * B(1,0) + A(1,2) * B(2,0),
            A(1,0) * B(0,1) + A(1,1) * B(1,1) + A(1,2) * B(2,1),
            A(1,0) * B(0,2) + A(1,1) * B(1,2) + A(1,2) * B(2,2),
            A(1,0) * B(0,3) + A(1,1) * B(1,3) + A(1,2) * B(2,3) + A(1,3),
                        
            A(2,0) * B(0,0) + A(2,1) * B(1,0) + A(2,2) * B(2,0),
            A(2,0) * B(0,1) + A(2,1) * B(1,1) + A(2,2) * B(2,1),
            A(2,0) * B(0,2) + A(2,1) * B(1,2) + A(2,2) * B(2,2),
            A(2,0) * B(0,3) + A(2,1) * B(1,3) + A(2,2) * B(2,3) + A(2,3)
        );
        */
        
        Transform4<DataType, ThreadCount> res{};
        
        // Only apply loop interchange optimization since matrix is too small for tiling and beyond
        // Loop interchange is done to account for the column major nature of the matrix versus the preference of row majors for cpu's memory (first index is row)
        // Collapse the last level for
        for (int j = 0; j < 4; j++)
        for (int k = 0; k < 3; k++)
        {
            res(0, j) += A(0, k) * B(k, j);
            res(1, j) += A(1, k) * B(k, j);
            res(2, j) += A(2, k) * B(k, j);
            res(3, j) += A(3, k) * B(k, j);
        }
        
        res(0, 3) += A(0,3);
        res(1, 3) += A(1,3);
        res(2, 3) += A(2,3);
        
        return res;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Transform4<DataType, ThreadCount> operator *(const Transform4<DataType, ThreadCount> &A, const Matrix4<DataType, ThreadCount> &B)
    {
        return static_cast<Matrix4<DataType, ThreadCount>>(A) * B;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Transform4<DataType, ThreadCount> operator *(const Matrix4<DataType, ThreadCount> &A, const Transform4<DataType, ThreadCount> &B)
    {
        return A * static_cast<Matrix4<DataType, ThreadCount>>(B);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator *(const Transform4<DataType, ThreadCount> &H, const Vector3<DataType, ThreadCount> &v)
    {
        return Vector3<DataType, ThreadCount>(
            H(0,0) * v.x + H(0,1) * v.y + H(0,2) * v.z,
            H(1,0) * v.x + H(1,1) * v.y + H(1,2) * v.z,
            H(2,0) * v.x + H(2,1) * v.y + H(2,2) * v.z
        );
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Point3<DataType, ThreadCount> operator *(const Transform4<DataType, ThreadCount> &H, const Point3<DataType, ThreadCount> &t)
    {
        return Point3<DataType, ThreadCount>(
            H(0,0) * t.x + H(0,1) * t.y + H(0,2) * t.z + H(0,3),
            H(1,0) * t.x + H(1,1) * t.y + H(1,2) * t.z + H(1,3),
            H(2,0) * t.x + H(2,1) * t.y + H(2,2) * t.z + H(2,3)
        );
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Plane<DataType, ThreadCount> operator * (const Plane<DataType, ThreadCount>& f, const Transform4<DataType, ThreadCount>& H)
    {
        return Plane<DataType, ThreadCount>(
            f.x * H(0, 0) + f.y * H(1, 0) + f.z * H(2, 0),
            f.x * H(0, 1) + f.y * H(1, 1) + f.z * H(2, 1),
            f.x * H(0, 2) + f.y * H(1, 2) + f.z * H(2, 2),
            f.x * H(0, 3) + f.y * H(1, 3) + f.z * H(2, 3) + f.w
            );    
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Line<DataType, ThreadCount> Transform(const Line<DataType, ThreadCount> &line, const Transform4<DataType, ThreadCount> &H)
    {
        Matrix3<DataType, ThreadCount> adj(Cross(H[1], H[2]), Cross(H[2], H[0]), Cross(H[0], H[1]));
        const Point3<DataType, ThreadCount> &t = H.GetTranslation();

        Vector3<DataType, ThreadCount> v = H * line.direction;
        Vector3<DataType, ThreadCount> m = adj * line.moment + Cross(t, v);
        return Line<DataType, ThreadCount>(v, m);
    }
}
