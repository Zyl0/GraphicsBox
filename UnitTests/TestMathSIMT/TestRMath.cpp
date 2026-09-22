#include <catch2/catch_all.hpp>
#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Interpolation", "[Scalar]",
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
    using Point = Point3<T, N>;
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
    
    ScalarType val1 = LinearInterpolate(ScalarType(0.0), T(10.0), T(0.5));
    REQUIRE((val1 == 5.0).All() == true);
    
    ScalarType val2 = LinearInterpolate(ScalarType(0.0), T(10.0), T(0.25));
    REQUIRE((val2 == 2.5).All() == true);

    // BiLinearInterpolate: p00, p10, p01, p11, u, v
    ScalarType bival1 = BiLinearInterpolate(ScalarType(0.0), ScalarType(10.0), ScalarType(0.0), ScalarType(10.0), ScalarType(0.5), ScalarType(0.5));
    // (1-u)(1-v)*p00 + u(1-v)*p10 + (1-u)v*p01 + u*v*p11
    // (0.5)(0.5)*0 + (0.5)(0.5)*10 + (0.5)(0.5)*0 + (0.5)(0.5)*10 = 2.5 + 2.5 = 5.0
    REQUIRE((bival1 == 5.0).All() == true);
    
    ScalarType bival2 = BiLinearInterpolate(ScalarType(1.0), ScalarType(2.0), ScalarType(3.0), ScalarType(4.0), ScalarType(0.0), ScalarType(0.0));
    REQUIRE((bival2 == 1.0).All() == true);
    
    ScalarType bival3 = BiLinearInterpolate(ScalarType(1.0), ScalarType(2.0), ScalarType(3.0), ScalarType(4.0), ScalarType(1.0), ScalarType(1.0));
    REQUIRE((bival3 == 4.0).All() == true);
}