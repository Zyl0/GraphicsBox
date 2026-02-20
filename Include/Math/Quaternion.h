#pragma once
#include "Matrix.h"
#include "Vector.h"

namespace Math
{
    template<typename type>
    struct QuaternionT
    {
        type x,y,z,w;

        QuaternionT() : x(0), y(0), z(0), w(1) {}

        QuaternionT(type a, type b, type c, type s) : x(a), y(b), z(c), w(s) {}

        QuaternionT(const Vector3t<type> &v, float s) : x(v.x), y(v.y), z(v.z), w(s) {}

        QuaternionT(type yaw, type pitch, type roll);

        const Vector3t<type> &GetVectorPart() const
        {
            return reinterpret_cast<const Vector3t<type>&>(x);
        }

        Matrix3t<type> GetRotationMatrix() const;

        const Vector3t<type> &GetAngles() const;

        void SetRotationMatrix(const Matrix3t<type> &m);

        /**
         * \brief rotate a vector 
         * \tparam type real number type
         * \param v Vector to rotate
         * \return Rotated Vector
         */
        Vector3t<type> operator () (const Vector3t<type> &v) const
        {
            // todo verify math
            
            //const Vector3t<type> &b = GetVectorPart();
            //type b2 = b.x * b.x + b.y * b.y + b.z + b.z;
            //return (v * (w * w - b2) + b * (Dot(v,b) * static_cast<type>(2)) + Cross(b, v) * (w * static_cast<type>(2)));

            const Vector3t<type> &u = GetVectorPart();
            const type s = w;

            return   static_cast<type>(2) * Dot(u, v) * u
                   + (s * s - Dot(u, u)) * v
                   + static_cast<type>(2) * s * Cross(u, v);
        }
    };

    template <typename type>
    QuaternionT<type> operator +(const QuaternionT<type> &q1, const QuaternionT<type> &q2)
    {
        return QuaternionT<type>( q1.x + q2.x, q1.y + q2.y, q1.z + q2.z, q1.w + q2.w );
    }

    template <typename type>
    QuaternionT<type> operator -(const QuaternionT<type> &q1, const QuaternionT<type> &q2)
    {
        return QuaternionT<type>( q1.x - q2.x, q1.y - q2.y, q1.z - q2.z, q1.w - q2.w );
    }

    template <typename type>
    QuaternionT<type> operator *(const QuaternionT<type> &q1, const QuaternionT<type> &q2)
    {
        return QuaternionT<type>(
            q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y,
            q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x,
            q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w,
            q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z
        );
    }

    template <typename type>
    QuaternionT<type>::QuaternionT(type yaw, type pitch, type roll)
    {
        // Abbreviations for the various angular functions
        double cy = cos(yaw * 0.5);
        double sy = sin(yaw * 0.5);
        double cp = cos(pitch * 0.5);
        double sp = sin(pitch * 0.5);
        double cr = cos(roll * 0.5);
        double sr = sin(roll * 0.5);
        
        w = cy * cp * cr + sy * sp * sr;
        x = cy * cp * sr - sy * sp * cr;
        y = sy * cp * sr + cy * sp * cr;
        z = sy * cp * cr - cy * sp * sr;
    }

    template <typename type>
    Matrix3t<type> QuaternionT<type>::GetRotationMatrix() const
    {
        type x2 = x * x;
        type y2 = y * y;
        type z2 = z * z;
        type xy = x * y;
        type xz = x * z;
        type yz = y * z;
        type wx = w * x;
        type wy = w * y;
        type wz = w * z;

        return Matrix3t<type>(
                static_cast<type>(1) - static_cast<type>(2) * (y2 + z2),
                static_cast<type>(2) * (xy - wz),
                static_cast<type>(2) * (xz + wy),

                static_cast<type>(2) * (xy + wz),
                static_cast<type>(1) - static_cast<type>(2) * (x2 + z2),
                static_cast<type>(2) * (yz - wx),

                static_cast<type>(2) * (xz - wy),
                static_cast<type>(2) * (yz + wx),
                static_cast<type>(1) - static_cast<type>(2) * (x2 + y2)
            );
    }

    template <typename type>
    const Vector3t<type>& QuaternionT<type>::GetAngles() const
    {
        Vector3t<type> retVector;

        retVector[2] = atan2(2.0 * (y * z + w * x), w * w - x * x - y * y + z * z);
        retVector[1] = asin(-2.0 * (x * z - w * y));
        retVector[0] = atan2(2.0 * (x * y + w * z), w * w + x * x - y * y - z * z);

        return retVector;
    }

    template <typename type>
    void QuaternionT<type>::SetRotationMatrix(const Matrix3t<type>& m)
    {
        type m00 = m(0,0);
        type m11 = m(1,1);
        type m22 = m(2,2);
        type sum = m00 + m11 + m22;

        if(sum > static_cast<type>(0))
        {
            w = sqrt(sum + static_cast<type>(1)) * static_cast<type>(0.5);
            type f = static_cast<type>(0.25) / w;

            x = (m(2,1) - m(1,2)) * f;
            y = (m(0,2) - m(2,0)) * f;
            z = (m(1,0) - m(0,1)) * f;
        }
        else if((m00 > m11) && (m00 > m22))
        {
            x = sqrt(m00 - m11 - m22 + static_cast<type>(1)) * static_cast<type>(0.5);
            type f = static_cast<type>(0.25) / x;
            
            y = (m(1,0) - m(0,1)) * f;
            z = (m(0,2) - m(2,0)) * f;
            w = (m(2,1) - m(1,2)) * f;
        }
        else if(m11 > m22)
        {
            y = sqrt(m11 - m00 - m22 + static_cast<type>(1)) * static_cast<type>(0.5);
            type f = static_cast<type>(0.25) / y;
            
            x = (m(1,0) - m(0,1)) * f;
            z = (m(2,1) - m(1,2)) * f;
            w = (m(0,2) - m(2,0)) * f;
        }
        else
        {
            z = sqrt(m22 - m00 - m11 + static_cast<type>(1)) * static_cast<type>(0.5);
            type f = static_cast<type>(0.25) / z;
            
            x = (m(0,2) - m(2,0)) * f;
            y = (m(2,1) - m(1,2)) * f;
            w = (m(1,0) - m(0,1)) * f;
        }
    }

    
    using QuaternionF = QuaternionT<float>;
    using QuaternionD = QuaternionT<double>;
}
