#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

using namespace Math;

TEST_CASE("Quaternion - Constructors and Properties")
{
    QuaternionF q1;
    QuaternionF q2(0.0f, 0.0f, 0.0f, 1.0f);
    QuaternionF q3(Vector3f(0.0f, 1.0f, 0.0f), (float)M_PI);
    QuaternionF q4(0.0f, (float)M_PI / 2.0f, 0.0f); // yaw, pitch, roll
    
    REQUIRE(q2.x == 0.0f);
    REQUIRE(q2.y == 0.0f);
    REQUIRE(q2.z == 0.0f);
    REQUIRE(q2.w == 1.0f);
    
    Vector3f vec = q3.GetVectorPart();
    REQUIRE(vec.x == 0.0f);
    
    Matrix3f mat = q2.GetRotationMatrix();
    REQUIRE(mat(0, 0) == 1.0f);
    REQUIRE(mat(1, 1) == 1.0f);
    REQUIRE(mat(2, 2) == 1.0f);
    
    Vector3f angles = q4.GetAngles();
    // Assuming yaw/pitch/roll maps appropriately
    
    q1.SetRotationMatrix(mat);
    REQUIRE(q1.w == 1.0f);
    
    Vector3f v(1.0f, 0.0f, 0.0f);
    Vector3f rot = q3(v); 
    // Rotate (1,0,0) by pi around y axis -> (-1, 0, 0)
    REQUIRE(Catch::Approx(rot.x).margin(0.0001f) == -1.0f);
    REQUIRE(Catch::Approx(rot.y).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(rot.z).margin(0.0001f) == 0.0f);
}

TEST_CASE("Quaternion - Operators")
{
    QuaternionF q1(0.0f, 0.0f, 0.0f, 1.0f);
    QuaternionF q2(1.0f, 0.0f, 0.0f, 0.0f);
    
    QuaternionF q3 = q1 + q2;
    REQUIRE(q3.x == 1.0f);
    REQUIRE(q3.w == 1.0f);
    
    QuaternionF q4 = q1 - q2;
    REQUIRE(q4.x == -1.0f);
    REQUIRE(q4.w == 1.0f);
    
    QuaternionF q5 = q1 * q2;
    REQUIRE(q5.x == 1.0f);
}
