#pragma once
#include "Math/Vector.h"

struct Ray
{
    alignas(16) Math::Vector3f origin;
    alignas(16) Math::Vector3f direction;
    float distance;
};

template <typename T>
struct BVH
{
    
};