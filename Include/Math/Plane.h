#pragma once

#include "Vector.h"

namespace Math
{
    template<typename type>
    struct PlaneT
    {
        type x, y, z, w;

        PlaneT(type nx, type ny, type nz, type d) : x(nx), y(ny), z(nz), w(d) {}

        PlaneT(const Vector3t<type> &n, type d) : x(n.x), y(n.y), z(n.z), w(d) {}

        inline const Vector3t<type> &GetNormal() const
        {
            return (reinterpret_cast<const Vector3t<type> &>(x));
        }
    };

    template<typename type>
    type Dot(const PlaneT<type> &f, const Vector3t<type> &v)
    {
        return (f.x * v.x + f.y * v.y + f.z * v.z);
    }

    template<typename type>
    type Dot(const PlaneT<type> &f, const Point3t<type> &p)
    {
        return (f.x * p.x + f.y * p.y + f.z * p.z);
    }

    using PlaneF = PlaneT<float>;
    using PlaneD = PlaneT<double>;
}