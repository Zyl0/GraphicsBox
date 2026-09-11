#pragma once

#include "Types.h"
#include "Functions.h"
#include "Vector.h"
#include "Matrix.h"
#include "Math/Quaternion.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct Quaternion
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

        Quaternion() : x(0), y(0), z(0), w(1) {}
        Quaternion(const ScalarType& a, const ScalarType& b, const ScalarType& c, const ScalarType& s) : x(a), y(b), z(c), w(s) {}
        Quaternion(const Vector3<DataType, ThreadCount>& v, const ScalarType& s) : x(v.x), y(v.y), z(v.z), w(s) {}
        Quaternion(const QuaternionT<Type>& q) : x(q.x), y(q.y), z(q.z), w(q.w) {}
        Quaternion(const ScalarType& yaw, const ScalarType& pitch, const ScalarType& roll)
        {
            // Abbreviations for the various angular functions
            ScalarType cy = Cos(yaw * DataType(0.5));
            ScalarType sy = Sin(yaw * DataType(0.5));
            ScalarType cp = Cos(pitch * DataType(0.5));
            ScalarType sp = Sin(pitch * DataType(0.5));
            ScalarType cr = Cos(roll * DataType(0.5));
            ScalarType sr = Sin(roll * DataType(0.5));
        
            w = cy * cp * cr + sy * sp * sr;
            x = cy * cp * sr - sy * sp * cr;
            y = sy * cp * sr + cy * sp * cr;
            z = sy * cp * cr - cy * sp * sr;
        }

        INLINE const Vector3<DataType, ThreadCount>& GetVectorPart() const
        {
            return reinterpret_cast<const Vector3<DataType, ThreadCount>&>(*(this));
        }

        INLINE Matrix3<DataType, ThreadCount> GetRotationMatrix() const
        {
            ScalarType x2 = x * x;
            ScalarType y2 = y * y;
            ScalarType z2 = z * z;
            ScalarType xy = x * y;
            ScalarType xz = x * z;
            ScalarType yz = y * z;
            ScalarType wx = w * x;
            ScalarType wy = w * y;
            ScalarType wz = w * z;

            return Matrix3<DataType, ThreadCount>(
                ScalarType(1) - ScalarType(2) * (y2 + z2),
                ScalarType(2) * (xy - wz),
                ScalarType(2) * (xz + wy),

                ScalarType(2) * (xy + wz),
                ScalarType(1) - ScalarType(2) * (x2 + z2),
                ScalarType(2) * (yz - wx),

                ScalarType(2) * (xz - wy),
                ScalarType(2) * (yz + wx),
                ScalarType(1) - ScalarType(2) * (x2 + y2)
            );
        }

        INLINE Vector3<DataType, ThreadCount> GetAngles() const
        {
            Vector3<DataType, ThreadCount> retVector;

            retVector[2] = ATan2(DataType(2) * (y * z + w * x), w * w - x * x - y * y + z * z);
            retVector[1] = ASin(DataType(-2) * (x * z - w * y));
            retVector[0] = ATan2(DataType(2) * (x * y + w * z), w * w + x * x - y * y - z * z);

            return retVector;
        }

        INLINE void SetRotationMatrix(const Matrix3<DataType, ThreadCount> &m)
        {
            ScalarType m00 = m(0,0);
            ScalarType m11 = m(1,1);
            ScalarType m22 = m(2,2);
            ScalarType sum = m00 + m11 + m22;
            ScalarType f;
            typename ScalarType::MaskType mask, set{};

            mask = sum > ScalarType(0);
            f = Type(0.25) / w;
            w = Select(Sqrt(sum + Type(1)) * Type(0.5), w, mask);
            x = Select((m(2,1) - m(1,2)) * f, x, mask);
            y = Select((m(0,2) - m(2,0)) * f, y, mask);
            z = Select((m(1,0) - m(0,1)) * f, z, mask);
            set &= mask;
            
            mask = (m00 > m11) && (m00 > m22);
            f = Type(0.25) / x;
            x = Select(Sqrt(m00 - m11 - m22 + Type(1)) * Type(0.5), x, mask);
            y = Select((m(1,0) - m(0,1)) * f, y, mask);
            z = Select((m(0,2) - m(2,0)) * f, z, mask);
            w = Select((m(2,1) - m(1,2)) * f, w, mask);
            set &= mask;

            mask = m11 > m22;
            f = Type(0.25) / y;
            y = Select(Sqrt(m11 - m00 - m22 + Type(1)) * Type(0.5), y, mask);
            x = Select((m(1,0) - m(0,1)) * f, x, mask);
            z = Select((m(2,1) - m(1,2)) * f, z, mask);
            w = Select((m(0,2) - m(2,0)) * f, w, mask);
            set &= mask;

            set = ~set;
            f = Type(0.25) / z;
            z = Select(Sqrt(m22 - m00 - m11 + Type(1)) * Type(0.5), z, set);
            x = Select((m(0,2) - m(2,0)) * f, x, set);
            y = Select((m(2,1) - m(1,2)) * f, y, set);
            w = Select((m(1,0) - m(0,1)) * f, w, set);
        }

        INLINE QuaternionT<Type> Elt(size_t index) const
        {
            return {x[index], y[index], z[index], w[index]};
        }

        /**
         * \brief rotate a vector 
         * \tparam type real number type
         * \param v Vector to rotate
         * \return Rotated Vector
         */
        Vector3<DataType, ThreadCount> operator () (const Vector3<DataType, ThreadCount> &v) const
        {
            // todo verify math
            
            //const Vector3t<type> &b = GetVectorPart();
            //type b2 = b.x * b.x + b.y * b.y + b.z + b.z;
            //return (v * (w * w - b2) + b * (Dot(v,b) * static_cast<type>(2)) + Cross(b, v) * (w * static_cast<type>(2)));

            const Vector3<DataType, ThreadCount> &u = GetVectorPart();

            return   ScalarType(2) * Dot(u, v) * u
                   + (w * w - Dot(u, u)) * v
                   + ScalarType(2) * w * Cross(u, v);
        }
    };

    template <typename DataType, size_t ThreadCount> requires (std::is_arithmetic_v<DataType>)
    INLINE Quaternion<DataType, ThreadCount> Select(
        const Quaternion<DataType, ThreadCount>& A,
        const Quaternion<DataType, ThreadCount>& B,
        const typename Quaternion<DataType, ThreadCount>::MaskType& mask)
    {
        Quaternion<DataType, ThreadCount> r;
        r.x = Select(A.x, B.x, mask);
        r.y = Select(A.y, B.y, mask);
        r.z = Select(A.z, B.z, mask);
        r.w = Select(A.w, B.w, mask);
        return r;
    }
    
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Quaternion<DataType, ThreadCount> operator +(const Quaternion<DataType, ThreadCount> &q1, const Quaternion<DataType, ThreadCount> &q2)
    {
        return Quaternion<DataType, ThreadCount>( q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w );
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Quaternion<DataType, ThreadCount> operator -(const Quaternion<DataType, ThreadCount> &q1, const Quaternion<DataType, ThreadCount> &q2)
    {
        return Quaternion<DataType, ThreadCount>( q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w );
    }

    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    Quaternion<DataType, ThreadCount> operator *(const Quaternion<DataType, ThreadCount> &q1, const Quaternion<DataType, ThreadCount> &q2)
    {
        return Quaternion<DataType, ThreadCount>(
            q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
            q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
            q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
        );
    }
}
