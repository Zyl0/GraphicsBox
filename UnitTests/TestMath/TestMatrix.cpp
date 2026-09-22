#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

using namespace Math;

TEST_CASE("Matrix3 - Constructors and Elements")
{
    Matrix3f m1;
    Matrix3f m2(1.0f, 2.0f, 3.0f,
                4.0f, 5.0f, 6.0f,
                7.0f, 8.0f, 9.0f);

    REQUIRE(m2(0, 0) == 1.0f);
    REQUIRE(m2(0, 1) == 2.0f);
    REQUIRE(m2(0, 2) == 3.0f);
    REQUIRE(m2(1, 0) == 4.0f);
    REQUIRE(m2(2, 2) == 9.0f);

    Vector3f v0(1.0f, 2.0f, 3.0f);
    Vector3f v1(4.0f, 5.0f, 6.0f);
    Vector3f v2(7.0f, 8.0f, 9.0f);
    Matrix3f m3(v0, v1, v2);

    REQUIRE(m3 == Transpose(m2));
    REQUIRE(m3[0].x == 1.0f);
    
    Matrix3f ident = MakeIdentity<float>();
    REQUIRE(ident(0, 0) == 1.0f);
    REQUIRE(ident(1, 1) == 1.0f);
    REQUIRE(ident(2, 2) == 1.0f);
    REQUIRE(ident(0, 1) == 0.0f);
}

TEST_CASE("Matrix3 - Operators and Functions")
{
    Matrix3f m1 = MakeIdentity<float>();
    Matrix3f m2 = MakeIdentity<float>();
    m2(0, 1) = 2.0f;
    
    Matrix3f add = m1 + m2;
    REQUIRE(add(0, 0) == 2.0f);
    REQUIRE(add(0, 1) == 2.0f);
    
    Matrix3f sub = m1 - m2;
    REQUIRE(sub(0, 0) == 0.0f);
    REQUIRE(sub(0, 1) == -2.0f);
    
    Matrix3f mul = m1 * m2;
    REQUIRE(mul == m2);
    
    Vector3f v(1.0f, 2.0f, 3.0f);
    Vector3f vm = m2 * v;
    // m2 is:
    // 1 2 0
    // 0 1 0
    // 0 0 1
    // Wait, is it row major or col major? 
    // The header says "Column major 3 by 3 matrix".
    // If it's column major, m2(0, 1) might mean column 0, row 1? 
    // Usually (row, col) is standard, but if it's col-major storage, operator() might abstract it.
    // Let's just test properties without relying on exact memory layout if possible.

    Matrix3f trans = Transpose(m2);
    REQUIRE(trans(0, 0) == m2(0, 0));
    REQUIRE(trans(1, 0) == m2(0, 1));
    REQUIRE(trans(0, 1) == m2(1, 0));
    
    REQUIRE(Determinant(m1) == 1.0f);
    
    Matrix3f inv = Inverse(m2);
    Matrix3f ident = m2 * inv;
    REQUIRE(Catch::Approx(ident(0,0)) == 1.0f);
    REQUIRE(Catch::Approx(ident(1,1)) == 1.0f);
    REQUIRE(Catch::Approx(ident(2,2)) == 1.0f);
    REQUIRE(Catch::Approx(ident(0,1)).margin(0.0001f) == 0.0f);
}

TEST_CASE("Matrix4 - Constructors and Elements")
{
    Matrix4f m2(1.0f, 2.0f, 3.0f, 4.0f,
                5.0f, 6.0f, 7.0f, 8.0f,
                9.0f, 10.0f, 11.0f, 12.0f,
                13.0f, 14.0f, 15.0f, 16.0f);

    REQUIRE(m2(0, 0) == 1.0f);
    REQUIRE(m2(0, 3) == 4.0f);
    REQUIRE(m2(3, 3) == 16.0f);
    
    Matrix4f ident = MakeMatrix4Identity<float>();
    REQUIRE(ident(0, 0) == 1.0f);
    REQUIRE(ident(1, 1) == 1.0f);
    REQUIRE(ident(2, 2) == 1.0f);
    REQUIRE(ident(3, 3) == 1.0f);
    REQUIRE(ident(0, 1) == 0.0f);
}

TEST_CASE("Matrix4 - Operators and Functions")
{
    Matrix4f m1 = MakeMatrix4Identity<float>();
    Matrix4f m2 = MakeMatrix4Identity<float>();
    m2(0, 1) = 2.0f;
    
    Matrix4f add = m1 + m2;
    REQUIRE(add(0, 0) == 2.0f);
    REQUIRE(add(0, 1) == 2.0f);
    
    Matrix4f mul = m1 * m2;
    REQUIRE(mul == m2);
    
    Matrix4f trans = Transpose(m2);
    REQUIRE(trans(1, 0) == m2(0, 1));
    REQUIRE(trans(0, 1) == m2(1, 0));
    
    // Test inverse
    Matrix4f m3(2.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 2.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 2.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f);
    Matrix4f inv = Inverse(m3);
    REQUIRE(Catch::Approx(inv(0,0)) == 0.5f);
    REQUIRE(Catch::Approx(inv(1,1)) == 0.5f);
    REQUIRE(Catch::Approx(inv(2,2)) == 0.5f);
    REQUIRE(Catch::Approx(inv(3,3)) == 1.0f);
}
