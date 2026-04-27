#pragma once

#include "Math/Matrix.h"

namespace Rendering
{
    bool frustumCullingTest(const Math::Matrix4f &ViewProj, const Math::Matrix4f &Model, Math::Point3f boundMin, Math::Point3f boundMax);
}
