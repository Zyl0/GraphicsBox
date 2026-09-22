#pragma once

#include "Types.h"
#include "Vector.h"

#include "Math/Matrix.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Matrix3
    {
        using Type = DataType;
        using ScalarType =  Scalar<DataType, ThreadCount>;
        using MaskType = typename Scalar<DataType, ThreadCount>::MaskType;
        using IndexerType = typename Scalar<DataType, ThreadCount>::IndexerType;

        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;
        static constexpr size_t kRowCount = 3;
        static constexpr size_t kColumnCount = 3;

        static consteval size_t Size() {return ThreadCount;}
        
    private:
        ScalarType n[3][3]{};

    public:
        Matrix3() = default;

        Matrix3(Type n00, Type n01, Type n02,
                 Type n10, Type n11, Type n12,
                 Type n20, Type n21, Type n22)
        {
            n[0][0] = n00; n[0][1] = n10; n[0][2] = n20;
            n[1][0] = n01; n[1][1] = n11; n[1][2] = n21;
            n[2][0] = n02; n[2][1] = n12; n[2][2] = n22;
        }
        
        Matrix3(const ScalarType& n00, const ScalarType& n01, const ScalarType& n02,
                 const ScalarType& n10, const ScalarType& n11, const ScalarType& n12,
                 const ScalarType& n20, const ScalarType& n21, const ScalarType& n22)
        {
            n[0][0] = n00; n[0][1] = n10; n[0][2] = n20;
            n[1][0] = n01; n[1][1] = n11; n[1][2] = n21;
            n[2][0] = n02; n[2][1] = n12; n[2][2] = n22;
        }
        
        Matrix3(const Matrix3t<DataType>& Matrix)
        {
            n[0][0] = Matrix[0][0]; n[0][1] = Matrix[0][1]; n[0][2] = Matrix[0][2];
            n[1][0] = Matrix[1][0]; n[1][1] = Matrix[1][1]; n[1][2] = Matrix[1][2];
            n[2][0] = Matrix[2][0]; n[2][1] = Matrix[2][1]; n[2][2] = Matrix[2][2];
        }

        Matrix3(const Vector3t<DataType> &a, const Vector3t<DataType> &b, const Vector3t<DataType> &c)
        {
            n[0][0] = a.x; n[0][1] = a.y; n[0][2] = a.z;
            n[1][0] = b.x; n[1][1] = b.y; n[1][2] = b.z;
            n[2][0] = c.x; n[2][1] = c.y; n[2][2] = c.z;
        }
        
        Matrix3(const Vector3<DataType, ThreadCount> &a, const Vector3<DataType, ThreadCount> &b, const Vector3<DataType, ThreadCount> &c)
        {
            n[0][0] = a.x; n[0][1] = a.y; n[0][2] = a.z;
            n[1][0] = b.x; n[1][1] = b.y; n[1][2] = b.z;
            n[2][0] = c.x; n[2][1] = c.y; n[2][2] = c.z;
        }

        ScalarType& operator ()(int i, int j)
        {
            return n[j][i];
        }

        const ScalarType& operator ()(int i, int j) const
        {
            return n[j][i];
        }

        MaskType operator == (const Matrix3<DataType, ThreadCount> &other) const
        {
            return
                n[0][0] == other.n[0][0] && n[0][1] == other.n[0][1] && n[0][2] == other.n[0][2] &&
                n[1][0] == other.n[1][0] && n[1][1] == other.n[1][1] && n[1][2] == other.n[1][2] &&
                n[2][0] == other.n[2][0] && n[2][1] == other.n[2][1] && n[2][2] == other.n[2][2];
        }

        MaskType operator != (const Matrix3<DataType, ThreadCount> &other) const
        {
            return !(this->operator==(other));
        }

        ScalarType &operator ()(size_t i, size_t j)
        {
            return n[j][i];
        }

        const ScalarType &operator ()(size_t i, size_t j) const
        {
            return n[j][i];
        }

        const ScalarType* data() const {return &n[0][0];}

        Matrix3t<Type> Elt(size_t index) const
        {
            return {
                this->operator()(0, 0), this->operator()(0, 1), this->operator()(0, 2),
                this->operator()(1, 0), this->operator()(1, 1), this->operator()(1, 2),
                this->operator()(2, 0), this->operator()(2, 1), this->operator()(2, 2)
            };
        }

        Vector3<DataType, ThreadCount>& operator [](int j)
        {
            return *reinterpret_cast<Vector3<DataType, ThreadCount>*>(n[j]);
        }

        Vector3<DataType, ThreadCount>& operator [](size_t j)
        {
            return *reinterpret_cast<Vector3<DataType, ThreadCount>*>(n[j]);
        }

        const Vector3<DataType, ThreadCount> &operator [](int j) const
        {
            return *reinterpret_cast<const Vector3<DataType, ThreadCount>*>(n[j]);
        }

        const Vector3<DataType, ThreadCount> &operator [](size_t j) const
        {
            return *reinterpret_cast<const Vector3<DataType, ThreadCount>*>(n[j]);
        }

        Matrix3& operator *=(const ScalarType& s)
        {
            n[0][0] *= s; n[0][1] *= s; n[0][2] *= s;
            n[1][0] *= s; n[1][1] *= s; n[1][2] *= s;
            n[2][0] *= s; n[2][1] *= s; n[2][2] *= s;
            return *this;
        }

        Matrix3& operator /=(const ScalarType& s)
        {
            n[0][0] /= s; n[0][1] /= s; n[0][2] /= s;
            n[1][0] /= s; n[1][1] /= s; n[1][2] /= s;
            n[2][0] /= s; n[2][1] /= s; n[2][2] /= s;
            return *this;
        }
        
        static Matrix3 Identity()
        {
            return Matrix3(
                static_cast<DataType>(1),   static_cast<DataType>(0),   static_cast<DataType>(0),
                static_cast<DataType>(0),   static_cast<DataType>(1),   static_cast<DataType>(0),
                static_cast<DataType>(0),   static_cast<DataType>(0),   static_cast<DataType>(1)
            );
        }
        
        static Matrix3 RotationX(const ScalarType& t)
        {
            ScalarType c = Coss(t);
            Scalar s = Sin(t);
            
            return Matrix3(
                ScalarType(1),  ScalarType(0),  ScalarType(0),
                ScalarType(0),  c,              -s,
                ScalarType(0),  -s,             c
            );
        }
        
        static Matrix3 RotationY(const ScalarType& t)
        {
            ScalarType c = Coss(t);
            Scalar s = Sin(t);
            
            return Matrix3(
                c,              ScalarType(0),  s,
                ScalarType(0),  ScalarType(1),  ScalarType(0),
                -s,             ScalarType(0),  c
            );
        }
        
        static Matrix3 RotationZ(const ScalarType& t)
        {
            ScalarType c = Coss(t);
            Scalar s = Sin(t);
            
            return Matrix3(
                c,              -s,             ScalarType(0),
                s,              c,              ScalarType(0),
                ScalarType(0),  ScalarType(0),  ScalarType(1)
            );
        }
        
        static Matrix3 Rotation(const Vector3<DataType, ThreadCount>& axis, const ScalarType& t)
        {
            ScalarType c = cos(t);
            ScalarType s = sin(t);
            ScalarType d = 1 - c;

            ScalarType x = axis.x * d;
            ScalarType y = axis.y * d;
            ScalarType z = axis.z * d;

            ScalarType axay = x * axis.y;
            ScalarType axaz = x * axis.z;
            ScalarType ayaz = y * axis.z;
            
            
            return Matrix3(
                c +  x * axis.x,    axay - s * axis.z,  axaz + s * axis.y,
                axay + s * axis.z,  c + y * axis.y,     ayaz - s * axis.x,
                axaz - s * axis.y,  ayaz + s * axis.x,  c + z * axis.x
            );
        }
        
        static Matrix3 Reflection(const Vector3<DataType, ThreadCount>& axis)
        {
            ScalarType x = axis.x * -2;
            ScalarType y = axis.y * -2;
            ScalarType z = axis.z * -2;

            ScalarType axay = x * axis.y;
            ScalarType axaz = x * axis.z;
            ScalarType ayaz = y * axis.z; 
            
            return Matrix3(
                x * axis.x + 1,     axay,          axaz,
                axay,            y * axis.y + 1,   ayaz,
                axaz,            ayaz,          z * axis.z + 1
            );
        }
        
        static Matrix3 Invocation(const Vector3<DataType, ThreadCount>& axis)
        {
            ScalarType x = axis.x * 2;
            ScalarType y = axis.y * 2;
            ScalarType z = axis.z * 2;

            ScalarType axay = x * axis.y;
            ScalarType axaz = x * axis.z;
            ScalarType ayaz = y * axis.z; 
            
            return Matrix3(
                x * axis.x - 1,    axay,           axaz,
                axay,           y * axis.y - 1,    ayaz,
                axaz,           ayaz,           z * axis.z - 1
            );
        }
        
        static Matrix3 Scale(const ScalarType& x, const ScalarType& y, const ScalarType& z)
        {            
            return Matrix3(
                x,              ScalarType(0),  ScalarType(0),
                ScalarType(0),  y,              ScalarType(0),
                ScalarType(0),  ScalarType(0),  z
            );
        }
        
        static Matrix3 Scale(const Vector3<DataType, ThreadCount>& s)
        {            
            return Matrix3(
                s.x,            ScalarType(0),  ScalarType(0),
                ScalarType(0),  s.y,            ScalarType(0),
                ScalarType(0),  ScalarType(0),  s.z
            );
        }
        
        static Matrix3 Scale(const ScalarType& s)
        {
            return Matrix3(
                s,              ScalarType(0),  ScalarType(0),
                ScalarType(0),  s,              ScalarType(0),
                ScalarType(0),  ScalarType(0),  s
            );
        }
        /*
        static Matrix3 Scale(ScalarType x, ScalarType y, ScalarType z)
        {            
            return Matrix3(
                ScalarType(1),  ScalarType(0),  ScalarType(0),
                ScalarType(0),  ScalarType(1),  ScalarType(0),
                ScalarType(0),  ScalarType(0),  ScalarType(1)
            );
        }
        */
    };

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Matrix3<DataType, ThreadCount> Select(
        const Matrix3<DataType, ThreadCount>& A,
        const Matrix3<DataType, ThreadCount>& B,
        const typename Matrix3<DataType, ThreadCount>::MaskType& mask)
    {
        Matrix3<DataType, ThreadCount> r;

        for (size_t i = 0; i < Matrix3<DataType, ThreadCount>::kRowCount; ++i)
        for (size_t j = 0; j < Matrix3<DataType, ThreadCount>::kColumnCount; ++j)
            r(j, i) = Select(A(j, i), B(j, i), mask);

        return r;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Matrix3<DataType, ThreadCount> operator +(const Matrix3<DataType, ThreadCount>& A, const Matrix3<DataType, ThreadCount>& B)
    {
        return Matrix3<DataType, ThreadCount>(
            A(0,0) + B(0,0), A(0,1) + B(0,1), A(0,2) + B(0,2),
            A(1,0) + B(1,0), A(1,1) + B(1,1), A(1,2) + B(1,2),
            A(2,0) + B(2,0), A(2,1) + B(2,1), A(2,2) + B(2,2)
            );
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Matrix3<DataType, ThreadCount> operator -(const Matrix3<DataType, ThreadCount>& A, const Matrix3<DataType, ThreadCount>& B)
    {
        return Matrix3<DataType, ThreadCount>(
            A(0,0) - B(0,0), A(0,1) - B(0,1), A(0,2) - B(0,2),
            A(1,0) - B(1,0), A(1,1) - B(1,1), A(1,2) - B(1,2),
            A(2,0) - B(2,0), A(2,1) - B(2,1), A(2,2) - B(2,2)
            );
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Matrix3<DataType, ThreadCount> operator *(const Matrix3<DataType, ThreadCount>& A, const Matrix3<DataType, ThreadCount>& B)
    {
        Matrix3<DataType, ThreadCount> res{};
        
        // Only apply loop interchange optimization since matrix is too small for tiling and beyond
        // Loop interchange is done to account for the column major nature of the matrix versus the preference of row majors for cpu's memory (first index is row)
        // Collapse the last level for
        for (int j = 0; j < 3; j++)
        for (int k = 0; k < 3; k++)
        {
            res(0, j) += A(0, k) * B(k, j);
            res(1, j) += A(1, k) * B(k, j);
            res(2, j) += A(2, k) * B(k, j);
        }
        
        return res;
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator *(const Matrix3<DataType, ThreadCount> &M, const Vector3<DataType, ThreadCount>& v)
    {
        return Vector3<DataType, ThreadCount>(
            M(0,0) * v.x + M(0,1) * v.y + M(0, 2) * v.z,
            M(1,0) * v.x + M(1,1) * v.y + M(1, 2) * v.z,
            M(2,0) * v.x + M(2,1) * v.y + M(2, 2) * v.z
            );
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector3<DataType, ThreadCount> operator *(const Matrix3<DataType, ThreadCount> &M, const Vector3t<DataType>& v)
    {
        return Vector3<DataType, ThreadCount>(
            M(0,0) * v.x + M(0,1) * v.y + M(0, 2) * v.z,
            M(1,0) * v.x + M(1,1) * v.y + M(1, 2) * v.z,
            M(2,0) * v.x + M(2,1) * v.y + M(2, 2) * v.z
            );
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Matrix3<DataType, ThreadCount>::ScalarType Determinant(const Matrix3<DataType, ThreadCount>& M)
    {
        return  (M(0,0) * (M(1,1) * M(2,2) - M(1, 2) * M(2, 1)))
              + (M(0,1) * (M(1,2) * M(2,0) - M(1, 0) * M(2, 2)))
              + (M(0,2) * (M(1,0) * M(2,1) - M(1, 1) * M(2, 0)));
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Matrix3<DataType, ThreadCount> Inverse(const Matrix3<DataType, ThreadCount>& M)
    {
        const Vector3<DataType, ThreadCount>& a = M[0];
        const Vector3<DataType, ThreadCount>& b = M[1];
        const Vector3<DataType, ThreadCount>& c = M[2];

        Vector3<DataType, ThreadCount> r0 = Cross(b, c);
        Vector3<DataType, ThreadCount> r1 = Cross(c, a);
        Vector3<DataType, ThreadCount> r2 = Cross(a,b);

        typename Matrix3<DataType, ThreadCount>::ScalarType invDet = Matrix3<DataType, ThreadCount>::ScalarType(1) / Dot(r2, c);

        return Matrix3<DataType, ThreadCount>(
            r0.x * invDet, r0.y * invDet, r0.z * invDet,
            r1.x * invDet, r1.y * invDet, r1.z * invDet,
            r2.x * invDet, r2.y * invDet, r2.z * invDet
            );
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Matrix3<DataType, ThreadCount> Transpose(const Matrix3<DataType, ThreadCount>& M)
    {
        return Matrix3<DataType, ThreadCount>(
            M(0,0), M(1,0), M(2,0),
            M(0,1), M(1,1), M(2,1),
            M(0,2), M(1,2), M(2,2));
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Matrix4
    {
        using Type = DataType;
        using ScalarType =  Scalar<DataType, ThreadCount>;
        using MaskType = typename Scalar<DataType, ThreadCount>::MaskType;
        using IndexerType = typename Scalar<DataType, ThreadCount>::IndexerType;

        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;
        static constexpr size_t kRowCount = 4;
        static constexpr size_t kColumnCount = 4;

        static consteval size_t Size() {return ThreadCount;}
        
    protected:
        ScalarType n[4][4]{};
        
    public:
        Matrix4() = default;
        
        Matrix4(Type n00, Type n01, Type n02, Type n03,
                 Type n10, Type n11, Type n12, Type n13,
                 Type n20, Type n21, Type n22, Type n23,
                 Type n30, Type n31, Type n32, Type n33)
        {
            n[0][0] = n00; n[0][1] = n10; n[0][2] = n20; n[0][3] = n30;
            n[1][0] = n01; n[1][1] = n11; n[1][2] = n21; n[1][3] = n31;
            n[2][0] = n02; n[2][1] = n12; n[2][2] = n22; n[2][3] = n32;
            n[3][0] = n03; n[3][1] = n13; n[3][2] = n23; n[3][3] = n33;
        }

        Matrix4(const ScalarType& n00, const ScalarType& n01, const ScalarType& n02, const ScalarType& n03,
                 const ScalarType& n10, const ScalarType& n11, const ScalarType& n12, const ScalarType& n13,
                 const ScalarType& n20, const ScalarType& n21, const ScalarType& n22, const ScalarType& n23,
                 const ScalarType& n30, const ScalarType& n31, const ScalarType& n32, const ScalarType& n33)
        {
            n[0][0] = n00; n[0][1] = n10; n[0][2] = n20; n[0][3] = n30;
            n[1][0] = n01; n[1][1] = n11; n[1][2] = n21; n[1][3] = n31;
            n[2][0] = n02; n[2][1] = n12; n[2][2] = n22; n[2][3] = n32;
            n[3][0] = n03; n[3][1] = n13; n[3][2] = n23; n[3][3] = n33;
        }
        
        Matrix4(const Matrix4t<Type>& Matrix)
        {
            n[0][0] = Matrix[0][0]; n[0][1] = Matrix[0][1]; n[0][2] = Matrix[0][2]; n[0][3] = Matrix[0][3];
            n[1][0] = Matrix[1][0]; n[1][1] = Matrix[1][1]; n[1][2] = Matrix[1][2]; n[1][3] = Matrix[1][3];
            n[2][0] = Matrix[2][0]; n[2][1] = Matrix[2][1]; n[2][2] = Matrix[2][2]; n[2][3] = Matrix[2][3];
            n[3][0] = Matrix[3][0]; n[3][1] = Matrix[3][1]; n[3][2] = Matrix[3][2]; n[3][3] = Matrix[3][3];
        }
        
        Matrix4( const Vector4t<DataType>& x, const Vector4t<DataType>& y, const Vector4t<DataType>& z, const Vector4t<DataType>& w )
        {
            n[0][0] = x.x;	n[0][1] = x.y;	n[0][2] = x.z;	n[0][3] = x.w;
            n[1][0] = y.x;	n[1][1] = y.y;	n[1][2] = y.z;	n[1][3] = y.w;
            n[2][0] = z.x;	n[2][1] = z.y;	n[2][2] = z.z;	n[2][3] = z.w;
            n[3][0] = w.x;  n[3][1] = w.y;	n[3][2] = w.z;	n[3][3] = w.w;
        }
        
        Matrix4( const Vector4<DataType, ThreadCount>& x, const Vector4<DataType, ThreadCount>& y, const Vector4<DataType, ThreadCount>& z, const Vector4<DataType, ThreadCount>& w )
        {
            n[0][0] = x.x;	n[0][1] = x.y;	n[0][2] = x.z;	n[0][3] = x.w;
            n[1][0] = y.x;	n[1][1] = y.y;	n[1][2] = y.z;	n[1][3] = y.w;
            n[2][0] = z.x;	n[2][1] = z.y;	n[2][2] = z.z;	n[2][3] = z.w;
            n[3][0] = w.x;  n[3][1] = w.y;	n[3][2] = w.z;	n[3][3] = w.w;
        }
        
        Matrix4(const Vector3<DataType, ThreadCount>& a, const ScalarType& x, const Vector3<DataType, ThreadCount>& b, const ScalarType& y, const Vector3<DataType, ThreadCount>& c, const ScalarType& z, const Vector3<DataType, ThreadCount>& d, const ScalarType& w)
        {
            n[0][0] = a.x;	n[0][1] = a.y;	n[0][2] = a.z;	n[0][3] = x;
            n[1][0] = b.x;	n[1][1] = b.y;	n[1][2] = b.z;	n[1][3] = y;
            n[2][0] = c.x;	n[2][1] = c.y;	n[2][2] = c.z;	n[2][3] = z;
            n[3][0] = d.x;  n[3][1] = d.y;	n[3][2] = d.z;	n[3][3] = w;
        }

        ScalarType &operator ()(int i, int j)
        {
            return n[j][i];
        }

        const ScalarType& operator ()(int i, int j) const
        {
            return n[j][i];
        }

        ScalarType& operator ()(size_t i, size_t j)
        {
            return n[j][i];
        }

        const ScalarType& operator ()(size_t i, size_t j) const
        {
            return n[j][i];
        }

        MaskType operator == (const Matrix4<DataType, ThreadCount> &other)
        {
            return
                n[0][0] == other.n[0][0] && n[0][1] == other.n[0][1] && n[0][2] == other.n[0][2] && n[0][3] == other.n[0][3] &&
                n[1][0] == other.n[1][0] && n[1][1] == other.n[1][1] && n[1][2] == other.n[1][2] && n[1][3] == other.n[1][3] &&
                n[2][0] == other.n[2][0] && n[2][1] == other.n[2][1] && n[2][2] == other.n[2][2] && n[2][3] == other.n[2][3] &&
                n[3][0] == other.n[3][0] && n[3][1] == other.n[3][1] && n[3][2] == other.n[3][2] && n[3][3] == other.n[3][3];
        }

        MaskType operator != (const Matrix4<DataType, ThreadCount> &other)
        {
            return !(this->operator==(other));
        }

        const ScalarType* data() const {return &n[0][0];}

        Matrix4t<Type> Elt(size_t index) const
        {
            return {
                this->operator()(0, 0), this->operator()(0, 1), this->operator()(0, 1), this->operator()(0, 3),
                this->operator()(1, 0), this->operator()(1, 1), this->operator()(1, 1), this->operator()(1, 3),
                this->operator()(2, 0), this->operator()(2, 1), this->operator()(2, 1), this->operator()(2, 3),
                this->operator()(3, 0), this->operator()(3, 1), this->operator()(3, 1), this->operator()(3, 3)
            };
        }

        Vector4<DataType, ThreadCount> &operator [](int j)
        {
            return *reinterpret_cast<Vector4<DataType, ThreadCount>*>(n[j]);
        }

        Vector4<DataType, ThreadCount> &operator [](size_t j)
        {
            return *reinterpret_cast<Vector4<DataType, ThreadCount>*>(n[j]);
        }

        const Vector4<DataType, ThreadCount> &operator [](int j) const
        {
            return *reinterpret_cast<const Vector4<DataType, ThreadCount>*>(n[j]);
        }

        const Vector4<DataType, ThreadCount> &operator [](size_t j) const
        {
            return *reinterpret_cast<const Vector4<DataType, ThreadCount>*>(n[j]);
        }

        Matrix4& operator *=(const ScalarType& s)
        {
            n[0][0] *= s;	n[0][1] *= s;	n[0][2] *= s;	n[0][3] *= s;
            n[1][0] *= s;	n[1][1] *= s;	n[1][2] *= s;	n[1][3] *= s;
            n[2][0] *= s;	n[2][1] *= s;	n[2][2] *= s;	n[2][3] *= s;
            n[3][0] *= s;   n[3][1] *= s;	n[3][2] *= s;	n[3][3] *= s;
            return *this;
        }

        Matrix4& operator /=(const ScalarType& s)
        {
            n[0][0] /= s;	n[0][1] /= s;	n[0][2] /= s;	n[0][3] /= s;
            n[1][0] /= s;	n[1][1] /= s;	n[1][2] /= s;	n[1][3] /= s;
            n[2][0] /= s;	n[2][1] /= s;	n[2][2] /= s;	n[2][3] /= s;
            n[3][0] /= s;   n[3][1] /= s;	n[3][2] /= s;	n[3][3] /= s;
            return *this;
        }
        
        static Matrix4 Identity()
        {
            return Matrix4(
                DataType(1),   DataType(0),   DataType(0),   DataType(0),
                DataType(0),   DataType(1),   DataType(0),   DataType(0),
                DataType(0),   DataType(0),   DataType(1),   DataType(0),
                DataType(0),   DataType(0),   DataType(0),   DataType(1)
            );
        }
        
        static Matrix4 RotationX(const ScalarType& t);

        static Matrix4 RotationY(const ScalarType& t);

        static Matrix4 RotationZ(const ScalarType& t);

        static Matrix4 Rotation(const Vector3<DataType, ThreadCount>& axis, const ScalarType& t);

        static Matrix4 Reflection(const Vector3<DataType, ThreadCount>& a);

        static Matrix4 Invocation(const Vector3<DataType, ThreadCount>& a);

        static Matrix4 Scale(const ScalarType& x, const ScalarType& y, const ScalarType& z);

        static Matrix4 Scale(const Vector3<DataType, ThreadCount>& s);

        static Matrix4 Scale(const ScalarType& s);
        
        static Matrix4 FrustumProjection(const ScalarType& FOVy, const ScalarType& s, const ScalarType& n, const ScalarType& f)
        {
            ScalarType g = DataType(1) / Tan(FOVy / DataType(2));
            ScalarType k = f / (f - n);
            
            return Matrix4(
                g / s,          ScalarType(0),  ScalarType(0),  ScalarType(0),
                ScalarType(0),  g,              ScalarType(0),  ScalarType(0),
                ScalarType(0),  ScalarType(0),  k,              -n * k,
                ScalarType(0),  ScalarType(0),  ScalarType(1),  ScalarType(0)
            );
        }
        
        static Matrix4 InfiniteProjection(const ScalarType& FOVy, const ScalarType& s, const ScalarType& n, const ScalarType& e)
        {
            ScalarType g = DataType(1) / Tan(FOVy / DataType(2));
            e = 1 - e;
            
            return Matrix4(
                g / s,          ScalarType(0),  ScalarType(0),  ScalarType(0),
                ScalarType(0),  g,              ScalarType(0),  ScalarType(0),
                ScalarType(0),  ScalarType(0),  e,              -n * e,
                ScalarType(0),  ScalarType(0),  ScalarType(1),  ScalarType(0)
            );
        }
        
        static Matrix4 RevFrustumProjection(const ScalarType& FOVy, const ScalarType& s, const ScalarType& n, const ScalarType& f)
        {
            ScalarType g = DataType(1) / Tan(FOVy / DataType(2));
            ScalarType k = f / (n - f);
            
            return Matrix4(
                g / s,          ScalarType(0),  ScalarType(0),  ScalarType(0),
                ScalarType(0),  g,              ScalarType(0),  ScalarType(0),
                ScalarType(0),  ScalarType(0),  k,              -f * k,
                ScalarType(0),  ScalarType(0),  ScalarType(1),  ScalarType(0)
            );
        }
        
        static Matrix4 RevInfiniteProjection(const ScalarType& FOVy, const ScalarType& s, const ScalarType& n, const ScalarType& e)
        {
            ScalarType g = DataType(1) / Tan(FOVy / DataType(2));

            return Matrix4(
                g / s,          ScalarType(0),  ScalarType(0),  ScalarType(0),
                ScalarType(0),  g,              ScalarType(0),  ScalarType(0),
                ScalarType(0),  ScalarType(0),  e,              n * (1 - e),
                ScalarType(0),  ScalarType(0),  ScalarType(1),  ScalarType(0)
            );
        }

        static Matrix4 OrthoProjection(const ScalarType& l, const ScalarType& r, const ScalarType& t, const ScalarType& b, const ScalarType& n, const ScalarType& f)
        {
            ScalarType wInv = DataType(1) / (r - l);
            ScalarType hInv = DataType(1) / (b - t);
            ScalarType dInv = DataType(1) / (f - n);
            
            return Matrix4(
                DataType(2) * wInv,     ScalarType(0),      ScalarType(0),  -(r + l) * wInv,
                ScalarType(0),          DataType(2) * hInv, ScalarType(0),  -(b + t) * hInv,
                ScalarType(0),          ScalarType(0),      dInv,           -n * dInv,
                ScalarType(0),          ScalarType(0),      ScalarType(0),  ScalarType(1)
            );
        }
        
        static Matrix4 LookAtView(const Vector3<DataType, ThreadCount>& from, const Vector3<DataType, ThreadCount>& at, const Vector3<DataType, ThreadCount>& up)
        {
            Vector3<DataType, ThreadCount> right = Normalize( Cross(at, Normalize(up) ) );
            Vector3<DataType, ThreadCount> newUp = Normalize( Cross(right, at) );
            
            return Inverse(Matrix4(
                right.x,        newUp.x,        -at.x,          from.x,
                right.y,        newUp.y,        -at.y,          from.y,
                right.z,        newUp.z,        -at.z,          from.z,
                ScalarType(0),  ScalarType(0),  ScalarType(0),  ScalarType(1)
            ));
        }
    };

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Matrix4<DataType, ThreadCount> Select(
        const Matrix4<DataType, ThreadCount>& A,
        const Matrix4<DataType, ThreadCount>& B,
        const typename Matrix4<DataType, ThreadCount>::MaskType& mask)
    {
        Matrix4<DataType, ThreadCount> r;

        for (size_t i = 0; i < Matrix4<DataType, ThreadCount>::kRowCount; ++i)
        for (size_t j = 0; j < Matrix4<DataType, ThreadCount>::kColumnCount; ++j)
            r(j, i) = Select(A(j, i), B(j, i), mask);

        return r;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Matrix4<DataType, ThreadCount> operator +(const Matrix4<DataType, ThreadCount>& A, const Matrix4<DataType, ThreadCount>& B)
    {
        return Matrix4<DataType, ThreadCount>(
            A(0,0) + B(0,0), A(0,1) + B(0,1), A(0,2) + B(0,2), A(0,3) + B(0,3),
            A(1,0) + B(1,0), A(1,1) + B(1,1), A(1,2) + B(1,2), A(1,3) + B(1,3),
            A(2,0) + B(2,0), A(2,1) + B(2,1), A(2,2) + B(2,2), A(2,3) + B(2,3),
            A(3,0) + B(3,0), A(3,1) + B(3,1), A(3,2) + B(3,2), A(3,3) + B(3,3));
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Matrix4<DataType, ThreadCount> operator -(const Matrix4<DataType, ThreadCount>& A, const Matrix4<DataType, ThreadCount>& B)
    {
        return Matrix4<DataType, ThreadCount>(
            A(0,0) - B(0,0), A(0,1) - B(0,1), A(0,2) - B(0,2), A(0,3) - B(0,3),
            A(1,0) - B(1,0), A(1,1) - B(1,1), A(1,2) - B(1,2), A(1,3) - B(1,3),
            A(2,0) - B(2,0), A(2,1) - B(2,1), A(2,2) - B(2,2), A(2,3) - B(2,3),
            A(3,0) - B(3,0), A(3,1) - B(3,1), A(3,2) - B(3,2), A(3,3) - B(3,3));
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Matrix4<DataType, ThreadCount> operator *(const Matrix4<DataType, ThreadCount>& A, const Matrix4<DataType, ThreadCount>& B)
    {
        Matrix4<DataType, ThreadCount> res{};
        
        // Only apply loop interchange optimization since matrix is too small for tiling and beyond
        // Loop interchange is done to account for the column major nature of the matrix versus the preference of row majors for cpu's memory (first index is row)
        // Collapse the last level for
        for (int j = 0; j < 4; j++)
        for (int k = 0; k < 4; k++)
        {
            res(0, j) += A(0, k) * B(k, j);
            res(1, j) += A(1, k) * B(k, j);
            res(2, j) += A(2, k) * B(k, j);
            res(3, j) += A(3, k) * B(k, j);
        }
        
        return res;
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator *(const Matrix4<DataType, ThreadCount> &M, const Vector4<DataType, ThreadCount>& v)
    {
        return Vector4<DataType, ThreadCount>(
            M(0,0) * v.x + M(0,1) * v.y + M(0, 2) * v.z + M(0,3) * v.w,
            M(1,0) * v.x + M(1,1) * v.y + M(1, 2) * v.z + M(1,3) * v.w,
            M(2,0) * v.x + M(2,1) * v.y + M(2, 2) * v.z + M(2,3) * v.w,
            M(3,0) * v.x + M(3,1) * v.y + M(3, 2) * v.z + M(3,3) * v.w);
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    INLINE Vector4<DataType, ThreadCount> operator *(const Vector4<DataType, ThreadCount>& v, const Matrix4<DataType, ThreadCount> &M)
    {
        return Vector4<DataType, ThreadCount>(
            M(0,0) * v.x + M(0,1) * v.x + M(0, 2) * v.x + M(0,3) * v.x,
            M(1,0) * v.y + M(1,1) * v.y + M(1, 2) * v.y + M(1,3) * v.y,
            M(2,0) * v.z + M(2,1) * v.z + M(2, 2) * v.z + M(2,3) * v.z,
            M(3,0) * v.w + M(3,1) * v.w + M(3, 2) * v.w + M(3,3) * v.w);
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Inverse(const Matrix4<DataType, ThreadCount>& M)
    {
        const Vector3<DataType, ThreadCount>& a = M[0].xyz();
        const Vector3<DataType, ThreadCount>& b = M[1].xyz();
        const Vector3<DataType, ThreadCount>& c = M[2].xyz();
        const Vector3<DataType, ThreadCount>& d = M[3].xyz();

        const typename Matrix4<DataType, ThreadCount>::ScalarType& x = M(3,0);
        const typename Matrix4<DataType, ThreadCount>::ScalarType& y = M(3,1);
        const typename Matrix4<DataType, ThreadCount>::ScalarType& z = M(3,2);
        const typename Matrix4<DataType, ThreadCount>::ScalarType& w = M(3,3);

        Vector3<DataType, ThreadCount> s = Cross(a, b);
        Vector3<DataType, ThreadCount> t = Cross(c, d);
        Vector3<DataType, ThreadCount> u = a * y - b * x;
        Vector3<DataType, ThreadCount> v = c * w - d * z;

        typename Matrix4<DataType, ThreadCount>::ScalarType invDet = Matrix4<DataType, ThreadCount>::ScalarType(1) / (Dot(s, v) +  Dot(t, u));
        s *= invDet;
        t *= invDet;
        u *= invDet;
        v *= invDet;

        Vector3<DataType, ThreadCount> r0 = Cross(b, v) + t * y;
        Vector3<DataType, ThreadCount> r1 = Cross(v, a) - t * x;
        Vector3<DataType, ThreadCount> r2 = Cross(d, u) + s * w;
        Vector3<DataType, ThreadCount> r3 = Cross(u, c) - s * z;
        
        return Matrix4<DataType, ThreadCount>(
            r0.x, r0.y, r0.z, -Dot(b, t),
            r1.x, r1.y, r1.z,  Dot(a, t),
            r2.x, r2.y, r2.z, -Dot(d, s),
            r3.x, r3.y, r3.z,  Dot(c, s)
            );
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Transpose(const Matrix4<DataType, ThreadCount>& M)
    {
        return Matrix4<DataType, ThreadCount>(
            M(0,0), M(1,0), M(2,0), M(3,0),
            M(0,1), M(1,1), M(2,1), M(3,1),
            M(0,2), M(1,2), M(2,2), M(3,2),
            M(0,3), M(1,3), M(2,3), M(3,3));
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> ToTransform4D(const Matrix3<DataType, ThreadCount>& Matrix)
    {
        return Matrix4<DataType, ThreadCount>(
            Matrix(0,0),    Matrix(1,0),    Matrix(2,0),    0,
            Matrix(0,1),    Matrix(1,1),    Matrix(2,1),    0,
            Matrix(0,2),    Matrix(1,2),    Matrix(2,2),    0,
            0,                  0,                  0,                  1
        );
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::RotationX(const ScalarType& t)
    {
        return ToTransform4D(Matrix3<DataType, ThreadCount>::RotationX(t));
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::RotationY(const ScalarType& t)
    {
        return ToTransform4D(Matrix3<DataType, ThreadCount>::RotationY(t));
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::RotationZ(const ScalarType& t)
    {
        return ToTransform4D(Matrix3<DataType, ThreadCount>::RotationZ(t));
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::Rotation(const Vector3<DataType, ThreadCount>& axis, const ScalarType& t)
    {
        return ToTransform4D(Matrix3<DataType, ThreadCount>::Rotation(axis, t));
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::Reflection(const Vector3<DataType, ThreadCount>& axis)
    {
        return ToTransform4D(Matrix3<DataType, ThreadCount>::Reflection(axis));
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::Invocation(const Vector3<DataType, ThreadCount>& axis)
    {
        return ToTransform4D(Matrix3<DataType, ThreadCount>::Invocation(axis));
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::Scale(const ScalarType& x, const ScalarType& y, const ScalarType& z)
    {
        return ToTransform4D(Matrix3<DataType, ThreadCount>::Scale(x, y, z)); 
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::Scale(const Vector3<DataType, ThreadCount>& s)
    {    
        return ToTransform4D(Matrix3<DataType, ThreadCount>::Scale(s));
    }

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    Matrix4<DataType, ThreadCount> Matrix4<DataType, ThreadCount>::Scale(const ScalarType& s)
    {
        return ToTransform4D(Matrix3<DataType, ThreadCount>::Scale(s));
    }
}
