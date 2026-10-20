#include "RayTracersCommon.h"

using namespace Math;

Math::Vector3f CalcPrimaryRayPoint(const Math::Matrix4f& ProjToWorld, Math::Vector2f ProjSpaceCoordinates, float ZCoordinate, Math::Vector2f PixelOffset)
{
    Vector4f p = ProjToWorld * Vector4f(
                         PixelOffset.x + ProjSpaceCoordinates.x,
                         -(PixelOffset.y + ProjSpaceCoordinates.y),
                        ZCoordinate, 1.f);
    return p.xyz() / p.w;
}
