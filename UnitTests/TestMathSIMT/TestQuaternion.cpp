#include <catch2/catch_all.hpp>
#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Quaternion", "[Scalar]",
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
    
    SECTION("Quaternion - Constructors and Properties")
    {
        Quaternion q1;
        Quaternion q2(0.0f, 0.0f, 0.0f, 1.0f);
        Quaternion q3(Vector3(0.0f, 1.0f, 0.0f), (float)M_PI);
        Quaternion q4(0.0f, (float)M_PI / 2.0f, 0.0f); // yaw, pitch, roll
    
        REQUIRE((q2.x == 0.0f).All() == true);
        REQUIRE((q2.y == 0.0f).All() == true);
        REQUIRE((q2.z == 0.0f).All() == true);
        REQUIRE((q2.w == 1.0f).All() == true);
    
        Vector3 vec = q3.GetVectorPart();
        REQUIRE((vec.x == 0.0f).All() == true);
    
        Matrix3 mat = q2.GetRotationMatrix();
        REQUIRE((mat(0, 0) == 1.0f).All() == true);
        REQUIRE((mat(1, 1) == 1.0f).All() == true);
        REQUIRE((mat(2, 2) == 1.0f).All() == true);
    
        Vector3 angles = q4.GetAngles();
        // Assuming yaw/pitch/roll maps appropriately
    
        q1.SetRotationMatrix(mat);
        REQUIRE((q1.w == 1.0f).All() == true);
    
        Vector3 v(1.0f, 0.0f, 0.0f);
        Vector3 rot = q3(v); 
        // Rotate (1,0,0) by pi around y axis -> (-1, 0, 0)
        REQUIRE((rot.x == -1.0f).All() == true);
        REQUIRE((rot.y == 0.0f).All() == true);
        REQUIRE((rot.z == 0.0f).All() == true);
    }

    SECTION("Quaternion - Operators")
    {
        Quaternion q1(0.0f, 0.0f, 0.0f, 1.0f);
        Quaternion q2(1.0f, 0.0f, 0.0f, 0.0f);
    
        Quaternion q3 = q1 + q2;
        REQUIRE((q3.x == 1.0f).All() == true);
        REQUIRE((q3.w == 1.0f).All() == true);
    
        Quaternion q4 = q1 - q2;
        REQUIRE((q4.x == -1.0f).All() == true);
        REQUIRE((q4.w == 1.0f).All() == true);
    
        Quaternion q5 = q1 * q2;
        REQUIRE((q5.x == 1.0f).All() == true);
    }
}