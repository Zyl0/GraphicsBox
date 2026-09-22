#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

using namespace Math;

TEST_CASE("ComponentTransform - WorldTransform")
{
    WorldTransformF wt;
    REQUIRE(wt.Position.x == 0.0f);
    REQUIRE(wt.Scale.x == 1.0f);
    REQUIRE(wt.Rotation.w == 1.0f);
    
    wt.Position = Point3f(1.0f, 2.0f, 3.0f);
    wt.Scale = Vector3f(2.0f, 2.0f, 2.0f);
    
    Transform4f trans = wt.GetTransform();
    
    Point3f p(1.0f, 1.0f, 1.0f);
    Point3f pTrans = wt.TransformPosition(p);
    
    REQUIRE(pTrans.x == Catch::Approx(1.0f * 2.0f + 1.0f));
    REQUIRE(pTrans.y == Catch::Approx(1.0f * 2.0f + 2.0f));
    REQUIRE(pTrans.z == Catch::Approx(1.0f * 2.0f + 3.0f));
    
    Point3f pOp = wt(p);
    REQUIRE(pTrans.x == pOp.x);
    REQUIRE(pTrans.y == pOp.y);
    REQUIRE(pTrans.z == pOp.z);
    
    Vector3f v(1.0f, 0.0f, 0.0f);
    Vector3f vTrans = wt.TransformVector(v); // scaling and rotation
    REQUIRE(vTrans.x == Catch::Approx(2.0f));
    
    Vector3f vOp = wt(v);
    REQUIRE(vTrans.x == vOp.x);
    
    WorldTransformF child;
    child.Position = Point3f(1.0f, 0.0f, 0.0f);
    
    WorldTransformF combined = wt * child;
    REQUIRE(combined.Position.x == Catch::Approx(3.0f)); // parent pos (1) + scaled child pos (1*2)
}
