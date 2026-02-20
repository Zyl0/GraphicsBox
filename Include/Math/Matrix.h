#pragma once
#include "Vector.h"

namespace Math
{
    template<typename type>
    struct Matrix3t
    {
    private:
        type n[3][3]{};

    public:
        Matrix3t() = default;

        Matrix3t(type n00, type n01, type n02,
                 type n10, type n11, type n12,
                 type n20, type n21, type n22)
        {
            n[0][0] = n00; n[0][1] = n10; n[0][2] = n20;
            n[1][0] = n01; n[1][1] = n11; n[1][2] = n21;
            n[2][0] = n02; n[2][1] = n12; n[2][2] = n22;
        }

        Matrix3t(const Vector3t<type> &a, const Vector3t<type> &b, const Vector3t<type> &c)
        {
            n[0][0] = a.x; n[0][1] = a.y; n[0][2] = a.z;
            n[1][0] = b.x; n[1][1] = b.y; n[1][2] = b.z;
            n[2][0] = c.x; n[2][1] = c.y; n[2][2] = c.z;
        }

        type &operator ()(int i, int j)
        {
            return n[j][i];
        }

        const type &operator ()(int i, int j) const
        {
            return n[j][i];
        }

        bool operator == (const Matrix3t<type> &other) const
        {
            return
                n[0][0] == other.n[0][0] && n[0][1] == other.n[0][1] && n[0][2] == other.n[0][2] &&
                n[1][0] == other.n[1][0] && n[1][1] == other.n[1][1] && n[1][2] == other.n[1][2] &&
                n[2][0] == other.n[2][0] && n[2][1] == other.n[2][1] && n[2][2] == other.n[2][2];
        }

        bool operator != (const Matrix3t<type> &other) const
        {
            return !(this->operator==(other));
        }

        type &operator ()(size_t i, size_t j)
        {
            return n[j][i];
        }

        const type &operator ()(size_t i, size_t j) const
        {
            return n[j][i];
        }

        const type* data() const
            {return &n[0][0];}

        Vector3t<type> &operator [](int j)
        {
            return *reinterpret_cast<Vector3t<type>*>(n[j]);
        }

        Vector3t<type> &operator [](size_t j)
        {
            return *reinterpret_cast<Vector3t<type>*>(n[j]);
        }

        const Vector3t<type> &operator [](int j) const
        {
            return *reinterpret_cast<const Vector3t<type>*>(n[j]);
        }

        const Vector3t<type> &operator [](size_t j) const
        {
            return *reinterpret_cast<const Vector3t<type>*>(n[j]);
        }

        Matrix3t& operator *=(type s)
        {
            n[0][0] *= s; n[0][1] *= s; n[0][2] *= s;
            n[1][0] *= s; n[1][1] *= s; n[1][2] *= s;
            n[2][0] *= s; n[2][1] *= s; n[2][2] *= s;
            return *this;
        }

        Matrix3t& operator /=(type s)
        {
            n[0][0] /= s; n[0][1] /= s; n[0][2] /= s;
            n[1][0] /= s; n[1][1] /= s; n[1][2] /= s;
            n[2][0] /= s; n[2][1] /= s; n[2][2] /= s;
            return *this;
        }
    };
    
    template<typename type>
    Matrix3t<type> MakeIdentity()
    {
        return Matrix3t<type>(
            static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1)
        );
    }

    template<typename type>
    inline Matrix3t<type> operator +(const Matrix3t<type>& A, const Matrix3t<type>& B)
    {
        return Matrix3t<type>(
                        A(0,0) + B(0,0), A(0,1) + B(0,1), A(0,2) + B(0,2),
                        A(1,0) + B(1,0), A(1,1) + B(1,1), A(1,2) + B(1,2),
                        A(2,0) + B(2,0), A(2,1) + B(2,1), A(2,2) + B(2,2));
    }

    template<typename type>
    inline Matrix3t<type> operator -(const Matrix3t<type>& A, const Matrix3t<type>& B)
    {
        return Matrix3t<type>(
                        A(0,0) - B(0,0), A(0,1) - B(0,1), A(0,2) - B(0,2),
                        A(1,0) - B(1,0), A(1,1) - B(1,1), A(1,2) - B(1,2),
                        A(2,0) - B(2,0), A(2,1) - B(2,1), A(2,2) - B(2,2));
    }

    template<typename type>
    inline Matrix3t<type> operator *(const Matrix3t<type>& A, const Matrix3t<type>& B)
    {
        return Matrix3t<type>(
                        A(0,0) * B(0,0) + A(0,1) * B(1,0) + A(0,2) * B(2,0),
                        A(0,0) * B(0,1) + A(0,1) * B(1,1) + A(0,2) * B(2,1),
                        A(0,0) * B(0,2) + A(0,1) * B(1,2) + A(0,2) * B(2,2),
                        
                        A(1,0) * B(0,0) + A(1,1) * B(1,0) + A(1,2) * B(2,0),
                        A(1,0) * B(0,1) + A(1,1) * B(1,1) + A(1,2) * B(2,1),
                        A(1,0) * B(0,2) + A(1,1) * B(1,2) + A(1,2) * B(2,2),
                        
                        A(2,0) * B(0,0) + A(2,1) * B(1,0) + A(2,2) * B(2,0),
                        A(2,0) * B(0,1) + A(2,1) * B(1,1) + A(2,2) * B(2,1),
                        A(2,0) * B(0,2) + A(2,1) * B(1,2) + A(2,2) * B(2,2)
                        );
    }

    template<typename type>
    inline Vector3t<type> operator *(const Matrix3t<type> &M, const Vector3t<type>& v)
    {
        return Vector3t<type>(
                        M(0,0) * v.x + M(0,1) * v.y + M(0, 2) * v.z,
                        M(1,0) * v.x + M(1,1) * v.y + M(1, 2) * v.z,
                        M(2,0) * v.x + M(2,1) * v.y + M(2, 2) * v.z);
    }

    template<typename type>
    inline type Determinant(const Matrix3t<type>& M)
    {
        return  (M(0,0) * (M(1,1) * M(2,2) - M(1, 2) * M(2, 1)))
              + (M(0,1) * (M(1,2) * M(2,0) - M(1, 0) * M(2, 2)))
              + (M(0,2) * (M(1,0) * M(2,1) - M(1, 1) * M(2, 0)));
    }

    template<typename type>
    Matrix3t<type> Inverse(const Matrix3t<type>& M)
    {
        const Vector3t<type>& a = M[0];
        const Vector3t<type>& b = M[1];
        const Vector3t<type>& c = M[2];

        Vector3t<type> r0 = Cross(b, c);
        Vector3t<type> r1 = Cross(c, a);
        Vector3t<type> r2 = Cross(a,b);

        type invDet = static_cast<type>(1) / Dot(r2, c);

        return Matrix3t<type>(r0.x * invDet, r0.y * invDet, r0.z * invDet,
                        r1.x * invDet, r1.y * invDet, r1.z * invDet,
                        r2.x * invDet, r2.y * invDet, r2.z * invDet);
    }

    template<typename type>
    Matrix3t<type> Transpose(const Matrix3t<type>& M)
    {
        return Matrix3t<type>(
            M(0,0), M(1,0), M(2,0),
            M(0,1), M(1,1), M(2,1),
            M(0,2), M(1,2), M(2,2));
    }

    template<typename type>
    struct Matrix4t
    {
    protected:
        type n[4][4]{};

    public:
        Matrix4t() = default;

        Matrix4t(type n00, type n01, type n02, type n03,
                 type n10, type n11, type n12, type n13,
                 type n20, type n21, type n22, type n23,
                 type n30, type n31, type n32, type n33)
        {
            n[0][0] = n00; n[0][1] = n10; n[0][2] = n20; n[0][3] = n30;
            n[1][0] = n01; n[1][1] = n11; n[1][2] = n21; n[1][3] = n31;
            n[2][0] = n02; n[2][1] = n12; n[2][2] = n22; n[2][3] = n32;
            n[3][0] = n03; n[3][1] = n13; n[3][2] = n23; n[3][3] = n33;
        }
        
        Matrix4t( const Vector4t<type>& x, const Vector4t<type>& y, const Vector4t<type>& z, const Vector4t<type>& w )
        {
            n[0][0] = x.x;	n[0][1] = x.y;	n[0][2] = x.z;	n[0][3] = x.w;
            n[1][0] = y.x;	n[1][1] = y.y;	n[1][2] = y.z;	n[1][3] = y.w;
            n[2][0] = z.x;	n[2][1] = z.y;	n[2][2] = z.z;	n[2][3] = z.w;
            n[3][0] = w.x;  n[3][1] = w.y;	n[3][2] = w.z;	n[3][3] = w.w;
        }
        
        Matrix4t(const Vector3t<type>& a, const type x, const Vector3t<type>& b, const type y, const Vector3t<type>& c, const type z, const Vector3t<type>& d, const type w)
        {
            n[0][0] = a.x;	n[0][1] = a.y;	n[0][2] = a.z;	n[0][3] = x;
            n[1][0] = b.x;	n[1][1] = b.y;	n[1][2] = b.z;	n[1][3] = y;
            n[2][0] = c.x;	n[2][1] = c.y;	n[2][2] = c.z;	n[2][3] = z;
            n[3][0] = d.x;  n[3][1] = d.y;	n[3][2] = d.z;	n[3][3] = w;
        }

        type &operator ()(int i, int j)
        {
            return n[j][i];
        }

        const type &operator ()(int i, int j) const
        {
            return n[j][i];
        }

        type &operator ()(size_t i, size_t j)
        {
            return n[j][i];
        }

        const type &operator ()(size_t i, size_t j) const
        {
            return n[j][i];
        }

        bool operator == (const Matrix4t<type> &other)
        {
            return
                n[0][0] == other.n[0][0] && n[0][1] == other.n[0][1] && n[0][2] == other.n[0][2] && n[0][3] == other.n[0][3] &&
                n[1][0] == other.n[1][0] && n[1][1] == other.n[1][1] && n[1][2] == other.n[1][2] && n[1][3] == other.n[1][3] &&
                n[2][0] == other.n[2][0] && n[2][1] == other.n[2][1] && n[2][2] == other.n[2][2] && n[2][3] == other.n[2][3] &&
                n[3][0] == other.n[3][0] && n[3][1] == other.n[3][1] && n[3][2] == other.n[3][2] && n[3][3] == other.n[3][3];
        }

        bool operator != (const Matrix4t<type> &other)
        {
            return !(this->operator==(other));
        }

        const type* data() const
            {return &n[0][0];}

        Vector4t<type> &operator [](int j)
        {
            return *reinterpret_cast<Vector4t<type>*>(n[j]);
        }

        Vector4t<type> &operator [](size_t j)
        {
            return *reinterpret_cast<Vector4t<type>*>(n[j]);
        }

        const Vector4t<type> &operator [](int j) const
        {
            return *reinterpret_cast<const Vector4t<type>*>(n[j]);
        }

        const Vector4t<type> &operator [](size_t j) const
        {
            return *reinterpret_cast<const Vector4t<type>*>(n[j]);
        }

        Matrix4t& operator *=(type s)
        {
            n[0][0] *= s;	n[0][1] *= s;	n[0][2] *= s;	n[0][3] *= s;
            n[1][0] *= s;	n[1][1] *= s;	n[1][2] *= s;	n[1][3] *= s;
            n[2][0] *= s;	n[2][1] *= s;	n[2][2] *= s;	n[2][3] *= s;
            n[3][0] *= s;   n[3][1] *= s;	n[3][2] *= s;	n[3][3] *= s;
            return *this;
        }

        Matrix4t& operator /=(type s)
        {
            n[0][0] /= s;	n[0][1] /= s;	n[0][2] /= s;	n[0][3] /= s;
            n[1][0] /= s;	n[1][1] /= s;	n[1][2] /= s;	n[1][3] /= s;
            n[2][0] /= s;	n[2][1] /= s;	n[2][2] /= s;	n[2][3] /= s;
            n[3][0] /= s;   n[3][1] /= s;	n[3][2] /= s;	n[3][3] /= s;
            return *this;
        }
    };

    template<typename type>
    inline Matrix4t<type> operator +(const Matrix4t<type>& A, const Matrix4t<type>& B)
    {
        return Matrix4t<type>(
                        A(0,0) + B(0,0), A(0,1) + B(0,1), A(0,2) + B(0,2), A(0,3) + B(0,3),
                        A(1,0) + B(1,0), A(1,1) + B(1,1), A(1,2) + B(1,2), A(1,3) + B(1,3),
                        A(2,0) + B(2,0), A(2,1) + B(2,1), A(2,2) + B(2,2), A(2,3) + B(2,3),
                        A(3,0) + B(3,0), A(3,1) + B(3,1), A(3,2) + B(3,2), A(3,3) + B(3,3));
    }

    template<typename type>
    inline Matrix4t<type> operator -(const Matrix4t<type>& A, const Matrix4t<type>& B)
    {
        return Matrix4t<type>(
                        A(0,0) - B(0,0), A(0,1) - B(0,1), A(0,2) - B(0,2), A(0,3) - B(0,3),
                        A(1,0) - B(1,0), A(1,1) - B(1,1), A(1,2) - B(1,2), A(1,3) - B(1,3),
                        A(2,0) - B(2,0), A(2,1) - B(2,1), A(2,2) - B(2,2), A(2,3) - B(2,3),
                        A(3,0) - B(3,0), A(3,1) - B(3,1), A(3,2) - B(3,2), A(3,3) - B(3,3));
    }

    template<typename type>
    inline Matrix4t<type> operator *(const Matrix4t<type>& A, const Matrix4t<type>& B)
    {
        return Matrix4t<type>(
                        A(0,0) * B(0,0) + A(0,1) * B(1,0) + A(0,2) * B(2,0) + A(0,3) * B(3,0),
                        A(0,0) * B(0,1) + A(0,1) * B(1,1) + A(0,2) * B(2,1) + A(0,3) * B(3,1),
                        A(0,0) * B(0,2) + A(0,1) * B(1,2) + A(0,2) * B(2,2) + A(0,3) * B(3,2),
                        A(0,0) * B(0,3) + A(0,1) * B(1,3) + A(0,2) * B(2,3) + A(0,3) * B(3,3),
                        
                        A(1,0) * B(0,0) + A(1,1) * B(1,0) + A(1,2) * B(2,0) + A(1,3) * B(3,0),
                        A(1,0) * B(0,1) + A(1,1) * B(1,1) + A(1,2) * B(2,1) + A(1,3) * B(3,1),
                        A(1,0) * B(0,2) + A(1,1) * B(1,2) + A(1,2) * B(2,2) + A(1,3) * B(3,2),
                        A(1,0) * B(0,3) + A(1,1) * B(1,3) + A(1,2) * B(2,3) + A(1,3) * B(3,3),
                        
                        A(2,0) * B(0,0) + A(2,1) * B(1,0) + A(2,2) * B(2,0) + A(2,3) * B(3,0),
                        A(2,0) * B(0,1) + A(2,1) * B(1,1) + A(2,2) * B(2,1) + A(2,3) * B(3,1),
                        A(2,0) * B(0,2) + A(2,1) * B(1,2) + A(2,2) * B(2,2) + A(2,3) * B(3,2),
                        A(2,0) * B(0,3) + A(2,1) * B(1,3) + A(2,2) * B(2,3) + A(2,3) * B(3,3),
                        
                        A(3,0) * B(0,0) + A(3,1) * B(1,0) + A(3,2) * B(2,0) + A(3,3) * B(3,0),
                        A(3,0) * B(0,1) + A(3,1) * B(1,1) + A(3,2) * B(2,1) + A(3,3) * B(3,1),
                        A(3,0) * B(0,2) + A(3,1) * B(1,2) + A(3,2) * B(2,2) + A(3,3) * B(3,2),
                        A(3,0) * B(0,3) + A(3,1) * B(1,3) + A(3,2) * B(2,3) + A(3,3) * B(3,3)
                        );
    }

    template<typename type>
    inline Vector4t<type> operator *(const Matrix4t<type> &M, const Vector4t<type>& v)
    {
        return Vector4t<type>(
                        M(0,0) * v.x + M(0,1) * v.y + M(0, 2) * v.z + M(0,3) * v.w,
                        M(1,0) * v.x + M(1,1) * v.y + M(1, 2) * v.z + M(1,3) * v.w,
                        M(2,0) * v.x + M(2,1) * v.y + M(2, 2) * v.z + M(2,3) * v.w,
                        M(3,0) * v.x + M(3,1) * v.y + M(3, 2) * v.z + M(3,3) * v.w);
    }

    template<typename type>
    inline Vector4t<type> operator *(const Vector4t<type>& v, const Matrix4t<type> &M)
    {
        return Vector4t<type>(
                        M(0,0) * v.x + M(0,1) * v.x + M(0, 2) * v.x + M(0,3) * v.x,
                        M(1,0) * v.y + M(1,1) * v.y + M(1, 2) * v.y + M(1,3) * v.y,
                        M(2,0) * v.z + M(2,1) * v.z + M(2, 2) * v.z + M(2,3) * v.z,
                        M(3,0) * v.w + M(3,1) * v.w + M(3, 2) * v.w + M(3,3) * v.w);
    }

    template<typename type>
    Matrix4t<type> MakeMatrix4Identity()
    {
        return Matrix4t<type>(
            static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1),   static_cast<type>(0),
            static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(0),   static_cast<type>(1)
        );
    }

    template<typename type>
    Matrix4t<type> Inverse(const Matrix4t<type>& M)
    {
        const Vector3t<type>& a = M[0].xyz();
        const Vector3t<type>& b = M[1].xyz();
        const Vector3t<type>& c = M[2].xyz();
        const Vector3t<type>& d = M[3].xyz();

        type x = M(3,0);
        type y = M(3,1);
        type z = M(3,2);
        type w = M(3,3);

        Vector3t<type> s = Cross(a, b);
        Vector3t<type> t = Cross(c, d);
        Vector3t<type> u = a * y - b * x;
        Vector3t<type> v = c * w - d * z;

        type invDet = static_cast<type>(1) / (Dot(s, v) +  Dot(t, u));
        s *= invDet;
        t *= invDet;
        u *= invDet;
        v *= invDet;

        Vector3t<type> r0 = Cross(b, v) + t * y;
        Vector3t<type> r1 = Cross(v, a) - t * x;
        Vector3t<type> r2 = Cross(d, u) + s * w;
        Vector3t<type> r3 = Cross(u, c) - s * z;
        
        return Matrix4t<type>(
            r0.x, r0.y, r0.z, -Dot(b, t),
            r1.x, r1.y, r1.z,  Dot(a, t),
            r2.x, r2.y, r2.z, -Dot(d, s),
            r3.x, r3.y, r3.z,  Dot(c, s)
            );
    }

    template<typename type>
    Matrix4t<type> Transpose(const Matrix4t<type>& M)
    {
        return Matrix4t<type>(
            M(0,0), M(1,0), M(2,0), M(3,0),
            M(0,1), M(1,1), M(2,1), M(3,1),
            M(0,2), M(1,2), M(2,2), M(3,2),
            M(0,3), M(1,3), M(2,3), M(3,3));
    }

    template<typename type>
    Matrix4t<type> ToTransform4D(const Matrix3t<type>& Matrix)
    {
        return Matrix4t<type>(
            Matrix(0,0),    Matrix(1,0),    Matrix(2,0),    0,
            Matrix(0,1),    Matrix(1,1),    Matrix(2,1),    0,
            Matrix(0,2),    Matrix(1,2),    Matrix(2,2),    0,
            0,              0,              0,              1
        );
    }

    
    using Matrix3i = Matrix3t<int>;
    using Matrix3f = Matrix3t<float>;
    using Matrix3d = Matrix3t<double>;

    using Matrix4i = Matrix4t<int>;
    using Matrix4f = Matrix4t<float>;
    using Matrix4d = Matrix4t<double>;
}
