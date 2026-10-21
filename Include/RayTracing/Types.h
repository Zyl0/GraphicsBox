#pragma once
#include "Math/Matrix.h"
#include "Math/Vector.h"

struct Ray
{
    alignas(16) Math::Vector3f origin;
    alignas(16) Math::Vector3f direction;
    float distance;
};

template <typename type>
INLINE Ray operator * (const Math::Matrix4t<type>& M, const Ray& ray)
{
    Ray r;
    Math::Vector4t<type> v;
    
    v = M * Math::Vector4t<type>(ray.origin, type(1));
    r.origin = v.xyz() / v.w;
    
    v = M * Math::Vector4t<type>(ray.origin + ray.distance * ray.direction, type(1));
    v.xyz() /= v.w;
    
    Math::Vector3t<type> d = v.xyz() - r.origin;
    r.direction = Normalize(d);
    r.distance = Magnitude(d);
    
    return r;
}