#include "Modeling/Primitives.h"

double SDFSphere::Value(const Math::Vector3d& v) const
{
    return Math::Magnitude(v - center) - radius;
}

double SDFCapsule::Value(const Math::Vector3d& v) const
{
    double deltaPA = Magnitude(v - A);
    double deltaPB = Magnitude(v - B);
    double deltaAB = Magnitude(B - A);

    Math::Vector3d vPA = v - A;
    Math::Vector3d vPB = v - B;

    Math::Vector3d u = (B - A) / deltaAB;
    double l = Dot(vPA, u);

    double dist = 0;

    if (l < 0)
    {
        dist = SquareMagnitude(vPA);
    }
    else if (l < deltaAB)
    {
        dist = sqrt(SquareMagnitude(vPA) - l*l);
    }
    else
    {
        dist = SquareMagnitude(vPB);
    }
    

    return dist - radius;
}

double SDFBox::Value(const Math::Vector3d& v) const
{
    Math::Point3d q = Abs(v) - D;
    double dist = std::min(std::max(std::max(q[0], q[1]), q[2]), 0.) + Math::Magnitude(Math::Max(q, Math::Point3d(0)));
    return dist - radius;
}

double SDFTore::Value(const Math::Vector3d& v) const
{
    Math::Vector3d v2 = v - center;
    const double x = Magnitude(Math::Vector3d(v2[0], 0, v2[2])) - outerRadius;
    return Magnitude(Math::Vector3d(x, v2[1], 0)) - inerRadius;
}
