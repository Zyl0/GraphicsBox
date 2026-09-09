#include <catch2/catch_all.hpp>
#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Functions", "[Scalar]",
    ((typename T, size_t N), T, N),
    (double, 1), (float, 1),
    (double, 2), (float, 2),
    (double, 4), (float, 4),
    (double, 8), (float, 8),
    (double, 16), (float, 16),
    (double, 32), (float, 32),
    (double, 64), (float, 64))
{
    using ScalarType = Scalar<T, N>;
    using MaskType = ScalarType::MaskType;
    using Box = Box3<T, N>;
    using Point3 = Point3<T, N>;
    using Vector2 = Vector2<T, N>;
    using Vector3 = Vector3<T, N>;
    using Vector4 = Vector4<T, N>;
    using Matrix3 = Matrix3<T, N>;
    using Matrix4 = Matrix4<T, N>;
    using Transform4 = Transform4<T, N>;
    using Line = Line<T, N>;
    using Plane = Plane<T, N>;
    using WorldTransform = WorldTransform<T, N>;
    using Quaternion = Quaternion<T, N>;
    constexpr size_t ThreadCount = N;
    
    SECTION("Functons - Abs")
    {
        REQUIRE((Abs(ScalarType(-5)) == 5).All() == true);
        REQUIRE((Abs(ScalarType(5)) == 5).All() == true);
    }

    SECTION("Functons - Radians & Degrees")
    {
        REQUIRE((Radians(ScalarType(180.0)) == M_PI).All() == true);
        REQUIRE((Degrees(ScalarType(M_PI)) == 180.0).All() == true);
    }

    SECTION("Functons - Clamp")
    {
        REQUIRE((Clamp(ScalarType(5.0),  T(0.0), T(10.0)) == T(5.0)).All() == true);
        REQUIRE((Clamp(ScalarType(-5.0), T(0.0), T(10.0)) == T(0.0)).All() == true);
        REQUIRE((Clamp(ScalarType(15.0), T(0.0), T(10.0)) == T(10.0)).All() == true);
    }

    SECTION("Functons - SmoothStep")
    {
        REQUIRE((SmoothStep(ScalarType(0.0f)) == 0.0f).All() == true);
        REQUIRE((SmoothStep(ScalarType(1.0f)) == 1.0f).All() == true);
        REQUIRE((SmoothStep(ScalarType(0.5f)) == 0.5f).All() == true);
    }

    SECTION("Functons - InverseLerp")
    {
        REQUIRE((InverseLinearInterpolate(ScalarType(0.0), T(10.0), T(5.0)) == 0.5).All() == true);
    }

    SECTION("Functons - Saturate")
    {
        REQUIRE((Saturate(ScalarType(0.5)) == 0.5).All() == true);
        REQUIRE((Saturate(ScalarType(-0.5)) == 0.0).All() == true);
        REQUIRE((Saturate(ScalarType(1.5)) == 1.0).All() == true);
    }
}