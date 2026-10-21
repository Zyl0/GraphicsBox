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

Math::Vector3f GramSchmidt(Math::Vector3f T, Math::Vector3f N)
{
    return Normalize(T - Dot(T, N) * N);
}

Math::Matrix4f JitterMatrix(float SubPixelOffsetX, float SubPixelOffsetY, float Width, float Height)
{
    return Math::Transform4f::Translation((SubPixelOffsetX / std::max(Width, 1.f) * 2.f), (SubPixelOffsetY / std::max(Height, 1.f) * 2.f), 0.f);
}

float PixelOffset(float SubPixelOffset, float PixelCount)
{
    return SubPixelOffset / std::max(PixelCount, 1.f) * 2.f;
}
