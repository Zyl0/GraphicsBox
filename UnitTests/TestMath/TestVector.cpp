#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

using namespace Math;

TEST_CASE("Vector2 - Constructors and Element Access")
{
    Vector2f v1;
    Vector2f v2(1.0f);
    Vector2f v3(1.0f, 2.0f);
    Vector2f v4(v3, v2); // v = b - a, so v2 - v3 = (1-1, 1-2) = (0, -1)

    REQUIRE(v4.x == 0.0f);
    REQUIRE(v4.y == -1.0f);

    REQUIRE(v2.x == 1.0f);
    REQUIRE(v2.y == 1.0f);

    REQUIRE(v3.x == 1.0f);
    REQUIRE(v3.y == 2.0f);

    REQUIRE(v3[0] == 1.0f);
    REQUIRE(v3[1] == 2.0f);

    v1[0] = 3.0f;
    v1[1] = 4.0f;
    REQUIRE(v1.x == 3.0f);
    REQUIRE(v1.y == 4.0f);
}

TEST_CASE("Vector2 - Operators")
{
    Vector2f a(1.0f, 2.0f);
    Vector2f b(3.0f, 4.0f);

    Vector2f c = a + b;
    REQUIRE(c.x == 4.0f);
    REQUIRE(c.y == 6.0f);

    c = a - b;
    REQUIRE(c.x == -2.0f);
    REQUIRE(c.y == -2.0f);

    c = a * b;
    REQUIRE(c.x == 3.0f);
    REQUIRE(c.y == 8.0f);

    c = a / b;
    REQUIRE(Catch::Approx(c.x) == 1.0f / 3.0f);
    REQUIRE(Catch::Approx(c.y) == 2.0f / 4.0f);

    c = a * 2.0f;
    REQUIRE(c.x == 2.0f);
    REQUIRE(c.y == 4.0f);

    c = 2.0f * a;
    REQUIRE(c.x == 2.0f);
    REQUIRE(c.y == 4.0f);

    c = a / 2.0f;
    REQUIRE(c.x == 0.5f);
    REQUIRE(c.y == 1.0f);
    
    a += b;
    REQUIRE(a.x == 4.0f);
    REQUIRE(a.y == 6.0f);
    
    a -= b;
    REQUIRE(a.x == 1.0f);
    REQUIRE(a.y == 2.0f);
}

TEST_CASE("Vector2 - Methods")
{
    Vector2f v(3.0f, 4.0f);
    REQUIRE(Magnitude(v) == 5.0f);
    
    Vector2f n = Normalize(v);
    REQUIRE(Catch::Approx(Magnitude(n)) == 1.0f);
    REQUIRE(Catch::Approx(n.x) == 3.0f / 5.0f);
    REQUIRE(Catch::Approx(n.y) == 4.0f / 5.0f);

    Vector2f a(1.0f, 0.0f);
    Vector2f b(0.0f, 1.0f);
    REQUIRE(Dot(a, b) == 0.0f);
    REQUIRE(Dot(a, a) == 1.0f);
    REQUIRE(CosTheta(a, b) == 0.0f);
}

TEST_CASE("Vector3 - Constructors and Element Access")
{
    Vector3f v1;
    Vector3f v2(1.0f);
    Vector3f v3(1.0f, 2.0f, 3.0f);
    Vector3f v4(v3);
    Vector3f v5(v3, v2); // v = b - a, so v2 - v3 = (1-1, 1-2, 1-3) = (0, -1, -2)

    REQUIRE(v5.x == 0.0f);
    REQUIRE(v5.y == -1.0f);
    REQUIRE(v5.z == -2.0f);

    REQUIRE(v2.x == 1.0f);
    REQUIRE(v2.y == 1.0f);
    REQUIRE(v2.z == 1.0f);

    REQUIRE(v3.x == 1.0f);
    REQUIRE(v3.y == 2.0f);
    REQUIRE(v3.z == 3.0f);

    REQUIRE(v3[0] == 1.0f);
    REQUIRE(v3[1] == 2.0f);
    REQUIRE(v3[2] == 3.0f);

    v1[0] = 4.0f;
    v1[1] = 5.0f;
    v1[2] = 6.0f;
    REQUIRE(v1.x == 4.0f);
    REQUIRE(v1.y == 5.0f);
    REQUIRE(v1.z == 6.0f);
    
    REQUIRE(v4 == v3);
}

TEST_CASE("Vector3 - Operators")
{
    Vector3f a(1.0f, 2.0f, 3.0f);
    Vector3f b(4.0f, 5.0f, 6.0f);

    Vector3f c = a + b;
    REQUIRE(c.x == 5.0f);
    REQUIRE(c.y == 7.0f);
    REQUIRE(c.z == 9.0f);

    c = a - b;
    REQUIRE(c.x == -3.0f);
    REQUIRE(c.y == -3.0f);
    REQUIRE(c.z == -3.0f);

    c = a * b;
    REQUIRE(c.x == 4.0f);
    REQUIRE(c.y == 10.0f);
    REQUIRE(c.z == 18.0f);

    c = a / b;
    REQUIRE(Catch::Approx(c.x) == 1.0f / 4.0f);
    REQUIRE(Catch::Approx(c.y) == 2.0f / 5.0f);
    REQUIRE(Catch::Approx(c.z) == 3.0f / 6.0f);

    c = a * 2.0f;
    REQUIRE(c.x == 2.0f);
    REQUIRE(c.y == 4.0f);
    REQUIRE(c.z == 6.0f);

    c = a / 2.0f;
    REQUIRE(c.x == 0.5f);
    REQUIRE(c.y == 1.0f);
    REQUIRE(c.z == 1.5f);
    
    c = -a;
    REQUIRE(c.x == -1.0f);
    REQUIRE(c.y == -2.0f);
    REQUIRE(c.z == -3.0f);
}

TEST_CASE("Vector3 - Methods")
{
    Vector3f v(2.0f, 3.0f, 6.0f);
    REQUIRE(Magnitude(v) == 7.0f);
    REQUIRE(SquareMagnitude(v) == 49.0f);
    
    Vector3f n = Normalize(v);
    REQUIRE(Catch::Approx(Magnitude(n)) == 1.0f);
    REQUIRE(Catch::Approx(n.x) == 2.0f / 7.0f);

    Vector3f a(1.0f, 0.0f, 0.0f);
    Vector3f b(0.0f, 1.0f, 0.0f);
    REQUIRE(Dot(a, b) == 0.0f);
    REQUIRE(Dot(a, a) == 1.0f);
    
    Vector3f cross = Cross(a, b);
    REQUIRE(cross.x == 0.0f);
    REQUIRE(cross.y == 0.0f);
    REQUIRE(cross.z == 1.0f);
    
    Vector3f proj = Project(Vector3f(2.0f, 2.0f, 0.0f), a);
    REQUIRE(proj.x == 2.0f);
    REQUIRE(proj.y == 0.0f);
    REQUIRE(proj.z == 0.0f);
    
    Vector3f absV = Abs(Vector3f(-1.0f, -2.0f, 3.0f));
    REQUIRE(absV.x == 1.0f);
    REQUIRE(absV.y == 2.0f);
    REQUIRE(absV.z == 3.0f);
}

TEST_CASE("Point3 - Constructors and Operations")
{
    Point3f p1(1.0f, 2.0f, 3.0f);
    Point3f p2(4.0f, 5.0f, 6.0f);
    
    Vector3f v = p2 - p1;
    REQUIRE(v.x == 3.0f);
    REQUIRE(v.y == 3.0f);
    REQUIRE(v.z == 3.0f);
    
    Point3f p3 = p1 + v;
    REQUIRE(p3.x == 4.0f);
    REQUIRE(p3.y == 5.0f);
    REQUIRE(p3.z == 6.0f);
    
    REQUIRE(Distance(p1, p2) == Catch::Approx(std::sqrt(27.0f)));
    REQUIRE(SquareDistance(p1, p2) == Catch::Approx(27.0f));
    
    Point3f c = Center(p1, p2);
    REQUIRE(c.x == 2.5f);
    REQUIRE(c.y == 3.5f);
    REQUIRE(c.z == 4.5f);
    
    Point3f m1 = Min(p1, p2);
    REQUIRE(m1.x == 1.0f);
    REQUIRE(m1.y == 2.0f);
    REQUIRE(m1.z == 3.0f);
    
    Point3f m2 = Max(p1, p2);
    REQUIRE(m2.x == 4.0f);
    REQUIRE(m2.y == 5.0f);
    REQUIRE(m2.z == 6.0f);
}

TEST_CASE("Vector4 - Constructors and Element Access")
{
    Vector4f v1;
    Vector4f v2(1.0f);
    Vector4f v3(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f v4(Vector3f(1.0f, 2.0f, 3.0f), 4.0f);
    Vector4f v5(v3, v2); // v = b - a = v2 - v3 = (1-1, 1-2, 1-3, 1-4) = (0, -1, -2, -3)

    REQUIRE(v5.x == 0.0f);
    REQUIRE(v5.y == -1.0f);
    REQUIRE(v5.z == -2.0f);
    REQUIRE(v5.w == -3.0f);

    REQUIRE(v2.x == 1.0f);
    REQUIRE(v2.y == 1.0f);
    REQUIRE(v2.z == 1.0f);
    REQUIRE(v2.w == 1.0f);

    REQUIRE(v3.x == 1.0f);
    REQUIRE(v3.y == 2.0f);
    REQUIRE(v3.z == 3.0f);
    REQUIRE(v3.w == 4.0f);

    REQUIRE(v4.x == 1.0f);
    REQUIRE(v4.y == 2.0f);
    REQUIRE(v4.z == 3.0f);
    REQUIRE(v4.w == 4.0f);

    v1[0] = 5.0f;
    v1[1] = 6.0f;
    v1[2] = 7.0f;
    v1[3] = 8.0f;
    REQUIRE(v1.x == 5.0f);
    REQUIRE(v1.y == 6.0f);
    REQUIRE(v1.z == 7.0f);
    REQUIRE(v1.w == 8.0f);
    
    Vector3f xyz = v1.xyz();
    REQUIRE(xyz.x == 5.0f);
    REQUIRE(xyz.y == 6.0f);
    REQUIRE(xyz.z == 7.0f);
    
    Vector3f yzx = v1.YZX();
    REQUIRE(yzx.x == 6.0f);
    REQUIRE(yzx.y == 7.0f);
    REQUIRE(yzx.z == 5.0f);
}

TEST_CASE("Vector4 - Operators and Methods")
{
    Vector4f a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector4f b(5.0f, 6.0f, 7.0f, 8.0f);

    Vector4f c = a + b;
    REQUIRE(c.x == 6.0f);
    REQUIRE(c.y == 8.0f);
    REQUIRE(c.z == 10.0f);
    REQUIRE(c.w == 12.0f);

    c = a - b;
    REQUIRE(c.x == -4.0f);
    REQUIRE(c.y == -4.0f);
    REQUIRE(c.z == -4.0f);
    REQUIRE(c.w == -4.0f);

    c = a * 2.0f;
    REQUIRE(c.x == 2.0f);
    REQUIRE(c.y == 4.0f);
    REQUIRE(c.z == 6.0f);
    REQUIRE(c.w == 8.0f);
    
    REQUIRE(Dot(a, b) == Catch::Approx(1*5 + 2*6 + 3*7 + 4*8));
}
