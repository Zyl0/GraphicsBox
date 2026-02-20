#pragma once
#include "Vector.h"

namespace Math
{
    template<typename type>
    struct LineT
    {
        Vector3t<type> direction;
        Vector3t<type> moment;

        LineT() = default;

        LineT(type vx, type vy, type vz, type mx, type my, type mz) :
            direction(vx, vy, vz), moment(mx, my, mz)
        {}

        LineT(const Vector3t<type> &v, const Vector3t<type> &m) :
            direction(v), moment(m)
        {}
    };

    using LineF = LineT<float>;
    using LineD = LineT<double>;
}
