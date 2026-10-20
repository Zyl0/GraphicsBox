#pragma once
#include "Math/Matrix.h"
#include "Math/Vector.h"

INLINE Math::Vector2f CalcProjSpaceCoords(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    return Math::Vector2f{
        (static_cast<float>(x) + .5f) / static_cast<float>(w),  
        (static_cast<float>(y) + .5f) / static_cast<float>(h)
    } * 2.f - 1.f;
}

Math::Vector3f CalcPrimaryRayPoint(const Math::Matrix4f& ProjToWorld, Math::Vector2f ProjSpaceCoordinates, float ZCoordinate, Math::Vector2f PixelOffset);
