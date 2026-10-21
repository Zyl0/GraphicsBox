#pragma once
#include "Math/Matrix.h"
#include "Math/Transforms.h"
#include "Math/Vector.h"

INLINE Math::Vector2f CalcProjSpaceCoords(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    return Math::Vector2f{
        (static_cast<float>(x) + .5f) / static_cast<float>(w),  
        (static_cast<float>(y) + .5f) / static_cast<float>(h)
    } * 2.f - 1.f;
}

Math::Vector3f CalcPrimaryRayPoint(const Math::Matrix4f& ProjToWorld, Math::Vector2f ProjSpaceCoordinates, float ZCoordinate, Math::Vector2f PixelOffset);

Math::Vector3f GramSchmidt(Math::Vector3f T, Math::Vector3f N);

template <uint32_t Base>
INLINE double Halton(uint32_t index) 
{
    double result = 0.0;
    constexpr double inv_base = 1.0 / static_cast<double>(Base); 
    double fraction = inv_base;
    
    while (index > 0) 
    {
        result += fraction * (index % Base);
        index /= Base; 
        fraction *= inv_base;
    }
    
    return result;
}
    
Math::Matrix4f JitterMatrix(float SubPixelOffsetX, float SubPixelOffsetY, float Width, float Height);

float PixelOffset(float SubPixelOffset, float PixelCount);
