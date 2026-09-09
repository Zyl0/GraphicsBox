#include <catch2/catch_all.hpp>
#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("ComponentTransform", "[Scalar]",
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
    
    SECTION("ComponentTransform - WorldTransform")
    {
        WorldTransform wt;
        REQUIRE((wt.Position.x == 0.0f).All() == true);
        REQUIRE((wt.Scale.x == 1.0f).All() == true);
        REQUIRE((wt.Rotation.w == 1.0f).All() == true);
    
        wt.Position = Point3(1.0f, 2.0f, 3.0f);
        wt.Scale = Vector3(2.0f, 2.0f, 2.0f);
    
        Transform4 trans = wt.GetTransform();
    
        Point3 p(1.0f, 1.0f, 1.0f);
        Point3 pTrans = wt.TransformPosition(p);
    
        REQUIRE((pTrans.x == (1.0f * 2.0f + 1.0f)).All() == true);
        REQUIRE((pTrans.y == (1.0f * 2.0f + 2.0f)).All() == true);
        REQUIRE((pTrans.z == (1.0f * 2.0f + 3.0f)).All() == true);
    
        Point3 pOp = wt(p);
        REQUIRE((pTrans.x == pOp.x).All() == true);
        REQUIRE((pTrans.y == pOp.y).All() == true);
        REQUIRE((pTrans.z == pOp.z).All() == true);
    
        Vector3 v(1.0f, 0.0f, 0.0f);
        Vector3 vTrans = wt.TransformVector(v); // scaling and rotation
        REQUIRE((vTrans.x == (2.0f)).All() == true);
    
        Vector3 vOp = wt(v);
        REQUIRE((vTrans.x == vOp.x).All() == true);
    
        WorldTransform child;
        child.Position = Point3(1.0f, 0.0f, 0.0f);
    
        WorldTransform combined = wt * child;
        REQUIRE((combined.Position.x == 3.0f).All() == true); // parent pos (1) + scaled child pos (1*).All() == true2)
    }
}