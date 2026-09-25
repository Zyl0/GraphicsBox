#pragma once

#include "Line.h"
#include "Matrix.h"
#include "Plane.h"

namespace Math
{
    template<typename type>
    struct Transform4t : Matrix4t<type>
    {
        Transform4t()  :
            Matrix4t<type>(
                static_cast<type>(1),       static_cast<type>(0),       static_cast<type>(0),       static_cast<type>(0),
                static_cast<type>(0),       static_cast<type>(1),       static_cast<type>(0),       static_cast<type>(0),
                static_cast<type>(0),       static_cast<type>(0),       static_cast<type>(1),       static_cast<type>(0),
                static_cast<type>(0),       static_cast<type>(0),       static_cast<type>(0),       static_cast<type>(1)
            )
        {}

        Transform4t(
                type n00, type n01, type n02, type n03,
                type n10, type n11, type n12, type n13,
                type n20, type n21, type n22, type n23) :
            Matrix4t<type>(
                    n00,    n01,	n02,	n03,
                    n10,	n11,	n12,	n13,
                    n20,	n21,	n22,	n23,
                    0,	    0,  	0,  	1
                )
        {}

        Transform4t(const Matrix4t<type> &m) :
            Matrix4t<type>(
                    m(0,0), m(0,1),	m(0,2),	m(0,3),
                    m(1,0),	m(1,1),	m(1,2),	m(1,3),
                    m(2,0),	m(2,1),	m(2,2),	m(2,3),
                    m(3,0), m(3,1), m(3,2), m(3,3)
                )
        {}

        Transform4t( const Vector3t<type>& x, const Vector3t<type>& y, const Vector3t<type>& z, const Vector3t<type>& t ) :
            Matrix4t<type>(
                    x.x,    x.y,	x.z,	t.x,
                    x.y,	y.y,	z.y,	t.y,
                    x.z,	y.z,	z.z,	t.z,
                    0,	    0,  	0,  	1
                )
        {}

        Transform4t( const Matrix3t<type>& M, const Vector3t<type>& t ) :
            Matrix4t<type>(
                    M(0,0), M(0,1),	M(0,2),	t.x,
                    M(1,0),	M(1,1),	M(1,2),	t.y,
                    M(2,0),	M(2,1),	M(2,2),	t.z,
                    0,	    0,  	0,  	1
                )
        {
        }

        Transform4t( const Matrix3t<type>& M) :
            Matrix4t<type>(
                    M(0,0), M(0,1),	M(0,2),	0,
                    M(1,0),	M(1,1),	M(1,2),	0,
                    M(2,0),	M(2,1),	M(2,2),	0,
                    0,	    0,  	0,  	1
                )
        {
        }

        Vector3t<type> &operator [](int j)
        {
            return *reinterpret_cast<Vector3t<type>*>(this->n[j]);
        }

        const Vector3t<type> &operator [](int j) const
        {
            return *reinterpret_cast<const Vector3t<type>*>(this->n[j]);
        }

        const Point3t<type> &GetTranslation() const
        {
            return *reinterpret_cast<const Point3t<type>*>(this->n[3]);
        }

        void SetTranslation(const Point3t<type> &p)
        {
            this->n[3][0] = p.x;
            this->n[3][1] = p.y;
            this->n[3][2] = p.z;
        }
        
        INLINE static Transform4t Identity()
        {
            return Transform4t<type>(
                static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
                static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),
                static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0)
            );
        }

        INLINE static Transform4t Translation(type tx, type ty, type tz)
        {
            return Transform4t<type>(
                static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),   tx,
                static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),   ty,
                static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1),   tz
            );
        }

        INLINE static Transform4t Translation(Vector3t<type> t)
        {
            return Translation(t.x, t.y, t.z);   
        }
        
        INLINE static Transform4t RotationX(type t)
        {
            type c = cos(t);
            type s = sin(t);

            return Transform4t<type>(
                static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
                static_cast<type>(0),   c,                      -s,                     static_cast<type>(0),
                static_cast<type>(0),   -s,                     c,                      static_cast<type>(0)
            );
        }

        INLINE static Transform4t RotationY(type t)
        {
            type c = cos(t);
            type s = sin(t);

            return Transform4t<type>(
                c,                      static_cast<type>(0),   s,                      static_cast<type>(0),
                static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),
                -s,                     static_cast<type>(0),   c,                      static_cast<type>(0)
            );
        }

        INLINE static Transform4t RotationZ(type t)
        {
            type c = cos(t);
            type s = sin(t);

            return Transform4t(
                c,                      -s,                     static_cast<type>(0),   static_cast<type>(0),
                s,                      c,                      static_cast<type>(0),   static_cast<type>(0),
                static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0)
            );
        }

        INLINE static Transform4t Rotation(const Vector3t<type> &a, type t)
        {
            type c = cos(t);
            type s = sin(t);
            type d = static_cast<type>(1) - c;

            type x = a.x * d;
            type y = a.y * d;
            type z = a.z * d;

            type axay = x * a.y;
            type axaz = x * a.z;
            type ayaz = y * a.z;

            return Transform4t(
                c +  x * a.x,           axay - s * a.z,         axaz + s * a.y,         static_cast<type>(0),
                axay + s * a.z,         c + y * a.y,            ayaz - s * a.x,         static_cast<type>(0),
                axaz - s * a.y,         ayaz + s * a.x,         c + z * a.x,            static_cast<type>(0)
            );
        }

        INLINE static Transform4t Reflection(const Vector3t<type> &a)
        {
            type x = a.x * static_cast<type>(-2);
            type y = a.y * static_cast<type>(-2);
            type z = a.z * static_cast<type>(-2);

            type axay = x * a.y;
            type axaz = x * a.z;
            type ayaz = y * a.z; 
        
            return Transform4t(
                x * a.x + 1,            axay,                   axaz,                   static_cast<type>(0),
                axay,                   y * a.y + 1,            ayaz,                   static_cast<type>(0),
                axaz,                   ayaz,                   z * a.z + 1,            static_cast<type>(0)
            );
        }

        INLINE static Transform4t Invocation(const Vector3t<type> &a)
        {
            type x = a.x * static_cast<type>(2);
            type y = a.y * static_cast<type>(2);
            type z = a.z * static_cast<type>(2);

            type axay = x * a.y;
            type axaz = x * a.z;
            type ayaz = y * a.z; 
        
            return Transform4t(
                x * a.x - 1,            axay,                   axaz,                   static_cast<type>(0),
                axay,                   y * a.y - 1,            ayaz,                   static_cast<type>(0),
                axaz,                   ayaz,                   z * a.z - 1,            static_cast<type>(0)
            );
        }

        INLINE static Transform4t Scale(type x, type y, type z)
        {
            return Transform4t(
                x,                      static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
                static_cast<type>(0),   y,                      static_cast<type>(0),   static_cast<type>(0),
                static_cast<type>(0),   static_cast<type>(0),   z,                      static_cast<type>(0)
            );
        }

        INLINE static Transform4t Scale(const Vector3t<type> &a)
        {
            return Scale(a.x, a.y, a.z);
        }

        INLINE static Transform4t Scale(type s)
        {
            return Scale(s, s, s);
        }

        INLINE static Transform4t Perspective(type fieldOfView, type aspectRatio, type zNear, type zFar)
        {
            float itan = 1 / tan(fieldOfView * 0.5f);
            float id = 1 / (zNear - zFar);
            
            return Transform4t(
                Matrix4t<type>(
                    itan/aspectRatio,   0,      0,                  0,
                    0,                  itan,   0,                  0,
                    0,                  0,      (zFar+zNear)*id,    2.f*zFar*zNear*id,
                    0,                  0,      -1,                 0
                )
            );
        }

        INLINE static Transform4t Reflection(const PlaneT<type> &f)
        {
            type x = f.x * static_cast<type>(-2);
            type y = f.y * static_cast<type>(-2);
            type z = f.z * static_cast<type>(-2);

            type nxny = x * f.y;
            type nxnz = x * f.z;
            type nynz = y * f.z;
            
            return Transform4t(
                x * f.x + static_cast<type>(1), nxny, nxnz, x * f.w,
                nxny, y * f.y + static_cast<type>(1), nynz, y * f.w,
                nxnz, nynz, z * f.z + static_cast<type>(1), z * f.w
            );
        }
    };

    template<typename type>
    Transform4t<type> Inverse(const Transform4t<type> &H)
    {
        const Vector3t<type> &a = H[0];
        const Vector3t<type> &b = H[1];
        const Vector3t<type> &c = H[2];
        const Vector3t<type> &d = H[3];

        Vector3t<type> s = Cross(a, b);
        Vector3t<type> t = Cross(c, d);

        type Det = Dot(s, c);
        type invDet = static_cast<type>(1) / Det;

        s *= invDet;
        t *= invDet;
        Vector3t<type> v = c * invDet;
        
        Vector3t<type> r0 = Cross(b, v);
        Vector3t<type> r1 = Cross(v, a);

        return Transform4t<type>(
            r0.x,   r0.y,   r0.z,   -Dot(b, t),
            r1.x,   r1.y,   r1.z,   Dot(a, t),
            s.x,    s.y,    s.z,    -Dot(d,s)
        );
    }

    template<typename type>
    Transform4t<type> operator *(const Transform4t<type> &A, const Transform4t<type> &B)
    {
        return Transform4t<type>(
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
    }

    template<typename type>
    Transform4t<type> operator *(const Transform4t<type> &A, const Matrix4t<type> &B)
    {
        return static_cast<Matrix4t<type>>(A) * B;
    }

    template<typename type>
    Transform4t<type> operator *(const Matrix4t<type> &A, const Transform4t<type> &B)
    {
        return A * static_cast<Matrix4t<type>>(B);
    }

    template<typename type>
    Vector3t<type> operator *(const Transform4t<type> &H, const Vector3t<type> &v)
    {
        return Vector3t<type>(
            H(0,0) * v.x + H(0,1) * v.y + H(0,2) * v.z,
            H(1,0) * v.x + H(1,1) * v.y + H(1,2) * v.z,
            H(2,0) * v.x + H(2,1) * v.y + H(2,2) * v.z
        );
    }

    template<typename type>
    Point3t<type> operator *(const Transform4t<type> &H, const Point3t<type> &t)
    {
        return Point3t<type>(
            H(0,0) * t.x + H(0,1) * t.y + H(0,2) * t.z + H(0,3),
            H(1,0) * t.x + H(1,1) * t.y + H(1,2) * t.z + H(1,3),
            H(2,0) * t.x + H(2,1) * t.y + H(2,2) * t.z + H(2,3)
        );
    }

    template<typename type>
    PlaneT<type> operator * (const PlaneT<type> &f, const Transform4t<type> &H)
    {
        return PlaneT<type>(
            f.x * H(0, 0) + f.y * H(1, 0) + f.z * H(2, 0),
            f.x * H(0, 1) + f.y * H(1, 1) + f.z * H(2, 1),
            f.x * H(0, 2) + f.y * H(1, 2) + f.z * H(2, 2),
            f.x * H(0, 3) + f.y * H(1, 3) + f.z * H(2, 3) + f.w
            );    
    }

    template<typename type>
    LineT<type> Transform(const LineT<type> &line, const Transform4t<type> &H)
    {
        Matrix3t<type> adj(Cross(H[1], H[2]), Cross(H[2], H[0]), Cross(H[0], H[1]));
        const Point3t<type> &t = H.GetTranslation();

        Vector3t<type> v = H * line.direction;
        Vector3t<type> m = adj * line.moment + Cross(t, v);
        return LineT<type>(v, m);
    }

    using Transform4f = Transform4t<float>;
    using Transform4d = Transform4t<double>;
}