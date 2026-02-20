#pragma once

#include "Line.h"
#include "Matrix.h"
#include "Plane.h"

namespace Math
{
    template<typename type>
    Matrix3t<type> MakeRotationX(type t)
    {
        type c = cos(t);
        type s = sin(t);

        return Matrix3t<type>(
            static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   c,                      -s,
            static_cast<type>(0),   -s,                     c
        );
    }

    template<typename type>
    Matrix3t<type> MakeRotationY(type t)
    {
        type c = cos(t);
        type s = sin(t);

        return Matrix3t<type>(
            c,                      static_cast<type>(0),   s,
            static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),
            -s,                     static_cast<type>(0),   c
        );
    }

    template<typename type>
    Matrix3t<type> MakeRotationZ(type t)
    {
        type c = cos(t);
        type s = sin(t);

        return Matrix3t<type>(
            c,                      -s,                     static_cast<type>(0),
            s,                      c,                      static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1)
        );
    }

    template<typename type>
    Matrix3t<type> MakeRotation(const Vector3t<type> &a, type t)
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

        return Matrix3t<type>(
            c +  x * a.x,           axay - s * a.z,         axaz + s * a.y,
            axay + s * a.z,         c + y * a.y,            ayaz - s * a.x,
            axaz - s * a.y,         ayaz + s * a.x,         c + z * a.x
        );
    }

    template<typename type>
    Matrix3t<type> MakeReflection(const Vector3t<type> &a)
    {
        type x = a.x * static_cast<type>(-2);
        type y = a.y * static_cast<type>(-2);
        type z = a.z * static_cast<type>(-2);

        type axay = x * a.y;
        type axaz = x * a.z;
        type ayaz = y * a.z; 
    
        return Matrix3t<type>(
            x * a.x + 1,            axay,                   axaz,
            axay,                   y * a.y + 1,            ayaz,
            axaz,                   ayaz,                   z * a.z + 1
        );
    }

    template<typename type>
    Matrix3t<type> MakeInvocation(const Vector3t<type> &a)
    {
        type x = a.x * static_cast<type>(2);
        type y = a.y * static_cast<type>(2);
        type z = a.z * static_cast<type>(2);

        type axay = x * a.y;
        type axaz = x * a.z;
        type ayaz = y * a.z; 
    
        return Matrix3t<type>(
            x * a.x - 1,            axay,                   axaz,
            axay,                   y * a.y - 1,            ayaz,
            axaz,                   ayaz,                   z * a.z - 1
        );
    }

    template<typename type>
    Matrix3t<type> MakeScale(type x, type y, type z)
    {
        return Matrix3t<type>(
            x,                      static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   y,                      static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(0),   z
        );
    }

    template<typename type>
    inline Matrix3t<type> MakeScale(const Vector3t<type> &a)
    {
        return MakeScale(a.x, a.y, a.z);
    }

    template<typename type>
    inline Matrix3t<type> MakeScale(type s)
    {
        return MakeScale(s, s, s);
    }

    template<typename type>
    inline Matrix4t<type> MakeFrustumProjection(type FOVy, type s, type n, type f)
    {
        type g = static_cast<type>(1) / tan(FOVy / static_cast<type>(2));
        type k = f / (f - n);

        return Matrix4t<type>(
            g / s,                      static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),       g,                      static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),       static_cast<type>(0),   k,                      -n * k,
            static_cast<type>(0),       static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0)
        );
    }

    template<typename type>
    inline Matrix4t<type> MakeInfiniteProjection(type FOVy, type s, type n, type e)
    {
        type g = static_cast<type>(1) / tan(FOVy / static_cast<type>(2));
        e = 1.0f - e;

        return Matrix4t<type>(
            g / s,                      static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),       g,                      static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),       static_cast<type>(0),   e,                      -n * e,
            static_cast<type>(0),       static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0)
        );
    }

    template<typename type>
inline Matrix4t<type> MakeRevFrustumProjection(type FOVy, type s, type n, type f)
    {
        type g = static_cast<type>(1) / tan(FOVy / static_cast<type>(2));
        type k = f / (n - f);

        return Matrix4t<type>(
            g / s,                      static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),       g,                      static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),       static_cast<type>(0),   k,                      -f * k,
            static_cast<type>(0),       static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0)
        );
    }

    template<typename type>
    inline Matrix4t<type> MakeRevInfiniteProjection(type FOVy, type s, type n, type e)
    {
        type g = static_cast<type>(1) / tan(FOVy / static_cast<type>(2));

        return Matrix4t<type>(
            g / s,                      static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),       g,                      static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),       static_cast<type>(0),   e,                      n * (static_cast<type>(1) - e),
            static_cast<type>(0),       static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0)
        );
    }

    template<typename type>
    inline Matrix4t<type> MakeOrthoProjection(type l, type r, type t, type b, type n, type f)
    {
        type wInv = static_cast<type>(1) / (r - l);
        type hInv = static_cast<type>(1) / (b - t);
        type dInv = static_cast<type>(1) / (f - n);

        return Matrix4t<type>(
            static_cast<type>(2) * wInv,static_cast<type>(0),       static_cast<type>(0),   -(r + l) * wInv,
            static_cast<type>(0),       static_cast<type>(2)*hInv,  static_cast<type>(0),   -(b + t) * hInv,
            static_cast<type>(0),       static_cast<type>(0),       dInv,                   -n * dInv,
            static_cast<type>(0),       static_cast<type>(0),       static_cast<type>(0),   static_cast<type>(1)
        );
    }


    template<typename type>
    inline Matrix4t<type> MakeLookAtView(Vector3t<type> from, Vector3t<type> at, Vector3t<type> up)
    {;
        Vector3t<type> right = Normalize( Cross(at, Normalize(up) ) );
        Vector3t<type> newUp = Normalize( Cross(right, at) );

        return Inverse(Matrix4t<type>(
            right.x,                    newUp.x,                -at.x,                  from.x,
            right.y,                    newUp.y,                -at.y,                  from.y,
            right.z,                    newUp.z,                -at.z,                  from.z,
            static_cast<type>(0),       static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1)));
    }

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

        const Point3t<type> &GetTranslation()
        {
            return *reinterpret_cast<const Point3t<type>*>(this->n[3]);
        }

        void SetTranslation(const Point3t<type> &p)
        {
            this->n[3][0] = p.x;
            this->n[3][1] = p.y;
            this->n[3][2] = p.z;
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
    Transform4t<type> MakeHomogeneousIdentity()
    {
        return Transform4t<type>(
            static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0)
        );
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousTranslation(type tx, type ty, type tz)
    {
        return Transform4t<type>(
            static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),   tx,
            static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),   ty,
            static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1),   tz
        );
    }

    template<typename type>
    inline Transform4t<type> MakeHomogeneousTranslation(Vector3t<type> t)
    {
        return MakeHomogeneousTranslation(t.x, t.y, t.z);   
    }
    
    template<typename type>
    Transform4t<type> MakeHomogeneousRotationX(type t)
    {
        type c = cos(t);
        type s = sin(t);

        return Transform4t<type>(
            static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   c,                      -s,                     static_cast<type>(0),
            static_cast<type>(0),   -s,                     c,                      static_cast<type>(0)
        );
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousRotationY(type t)
    {
        type c = cos(t);
        type s = sin(t);

        return Transform4t<type>(
            c,                      static_cast<type>(0),   s,                      static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),
            -s,                     static_cast<type>(0),   c,                      static_cast<type>(0)
        );
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousRotationZ(type t)
    {
        type c = cos(t);
        type s = sin(t);

        return Transform4t<type>(
            c,                      -s,                     static_cast<type>(0),   static_cast<type>(0),
            s,                      c,                      static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0)
        );
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousRotation(const Vector3t<type> &a, type t)
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

        return Transform4t<type>(
            c +  x * a.x,           axay - s * a.z,         axaz + s * a.y,         static_cast<type>(0),
            axay + s * a.z,         c + y * a.y,            ayaz - s * a.x,         static_cast<type>(0),
            axaz - s * a.y,         ayaz + s * a.x,         c + z * a.x,            static_cast<type>(0)
        );
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousReflection(const Vector3t<type> &a)
    {
        type x = a.x * static_cast<type>(-2);
        type y = a.y * static_cast<type>(-2);
        type z = a.z * static_cast<type>(-2);

        type axay = x * a.y;
        type axaz = x * a.z;
        type ayaz = y * a.z; 
    
        return Transform4t<type>(
            x * a.x + 1,            axay,                   axaz,                   static_cast<type>(0),
            axay,                   y * a.y + 1,            ayaz,                   static_cast<type>(0),
            axaz,                   ayaz,                   z * a.z + 1,            static_cast<type>(0)
        );
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousInvocation(const Vector3t<type> &a)
    {
        type x = a.x * static_cast<type>(2);
        type y = a.y * static_cast<type>(2);
        type z = a.z * static_cast<type>(2);

        type axay = x * a.y;
        type axaz = x * a.z;
        type ayaz = y * a.z; 
    
        return Transform4t<type>(
            x * a.x - 1,            axay,                   axaz,                   static_cast<type>(0),
            axay,                   y * a.y - 1,            ayaz,                   static_cast<type>(0),
            axaz,                   ayaz,                   z * a.z - 1,            static_cast<type>(0)
        );
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousScale(type x, type y, type z)
    {
        return Transform4t<type>(
            x,                      static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   y,                      static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(0),   z,                      static_cast<type>(0)
        );
    }

    template<typename type>
    inline Transform4t<type> MakeHomogeneousScale(const Vector3t<type> &a)
    {
        return MakeHomogeneousScale(a.x, a.y, a.z);
    }

    template<typename type>
    inline Transform4t<type> MakeHomogeneousScale(type s)
    {
        return MakeHomogeneousScale(s, s, s);
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousPerspective(type fieldOfView, type aspectRatio, type zNear, type zFar)
    {
        float itan = 1 / tan(fieldOfView * 0.5f);
        float id = 1 / (zNear - zFar);
        
        return static_cast<Transform4t<type>>(
            Matrix4t<type>(
                itan/aspectRatio,   0,      0,                  0,
                0,                  itan,   0,                  0,
                0,                  0,      (zFar+zNear)*id,    2.f*zFar*zNear*id,
                0,                  0,      -1,                 0
            )
        );
    }

    template<typename type>
    Transform4t<type> MakeHomogeneousReflection(const PlaneT<type> &f)
    {
        type x = f.x * static_cast<type>(-2);
        type y = f.y * static_cast<type>(-2);
        type z = f.z * static_cast<type>(-2);

        type nxny = x * f.y;
        type nxnz = x * f.z;
        type nynz = y * f.z;
        
        return static_cast<Transform4t<type>>(
            x * f.x + static_cast<type>(1), nxny, nxnz, x * f.w,
            nxny, y * f.y + static_cast<type>(1), nynz, y * f.w,
            nxnz, nynz, z * f.z + static_cast<type>(1), z * f.w
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