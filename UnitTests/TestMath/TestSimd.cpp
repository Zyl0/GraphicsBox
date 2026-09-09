#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

using namespace Math;

TEST_CASE("Simd - Aligned Types")
{
    AlignedVector2f av2;
    av2 = Vector2f(1.0f, 2.0f);
    REQUIRE(av2().x == 1.0f);
    REQUIRE(av2().y == 2.0f);
    
    AlignedVector3f av3;
    av3 = Vector3f(1.0f, 2.0f, 3.0f);
    REQUIRE(av3().x == 1.0f);
    REQUIRE(av3().y == 2.0f);
    REQUIRE(av3().z == 3.0f);
    
    AlignedVector4f av4;
    av4 = Vector4f(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(av4().w == 4.0f);
    
    AlignedQuaternionF aq;
    aq = QuaternionF(1.0f, 2.0f, 3.0f, 4.0f);
    REQUIRE(aq().w == 4.0f);
    
    AlignedMatrix4f am4;
    am4 = MakeMatrix4Identity<float>();
    REQUIRE(am4()(0, 0) == 1.0f);
    
    Transform4f t = MakeHomogeneousIdentity<float>();
    am4 = t;
    REQUIRE(am4()(0, 0) == 1.0f);
}
