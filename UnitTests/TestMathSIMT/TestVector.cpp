#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Vectors", "[Scalar]",
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
    
    SECTION("Vector2 - Constructors and Element Access")
    {
        Vector2 v1;
        Vector2 v2(1.0f);
        Vector2 v3(1.0f, 2.0f);
        Vector2 v4(v3, v2); // v = b - a, so v2 - v3 = (1-1, 1-2) = (0, -1)

        REQUIRE((v4.x == 0.0f).All() == true);
        REQUIRE((v4.y == -1.0f).All() == true);

        REQUIRE((v2.x == 1.0f).All() == true);
        REQUIRE((v2.y == 1.0f).All() == true);

        REQUIRE((v3.x == 1.0f).All() == true);
        REQUIRE((v3.y == 2.0f).All() == true);

        REQUIRE((v3[0] == 1.0f).All() == true);
        REQUIRE((v3[1] == 2.0f).All() == true);

        v1[0] = 3.0f;
        v1[1] = 4.0f;
        REQUIRE((v1.x == 3.0f).All() == true);
        REQUIRE((v1.y == 4.0f).All() == true);
    }

    SECTION("Vector2 - Operators")
    {
        Vector2 a(1.0f, 2.0f);
        Vector2 b(3.0f, 4.0f);

        Vector2 c = a + b;
        REQUIRE((c.x == 4.0f).All() == true);
        REQUIRE((c.y == 6.0f).All() == true);

        c = a - b;
        REQUIRE((c.x == -2.0f).All() == true);
        REQUIRE((c.y == -2.0f).All() == true);

        c = a * b;
        REQUIRE((c.x == 3.0f).All() == true);
        REQUIRE((c.y == 8.0f).All() == true);

        c = a / b;
        REQUIRE((c.x == 1.0f / 3.0f).All() == true);
        REQUIRE((c.y == 2.0f / 4.0f).All() == true);

        c = a * T(2);
        REQUIRE((c.x == 2.0f).All() == true);
        REQUIRE((c.y == 4.0f).All() == true);

        c = 2.0f * a;
        REQUIRE((c.x == 2.0f).All() == true);
        REQUIRE((c.y == 4.0f).All() == true);

        c = a / T(2);
        REQUIRE((c.x == 0.5f).All() == true);
        REQUIRE((c.y == 1.0f).All() == true);
    
        a += b;
        REQUIRE((a.x == 4.0f).All() == true);
        REQUIRE((a.y == 6.0f).All() == true);
    
        a -= b;
        REQUIRE((a.x == 1.0f).All() == true);
        REQUIRE((a.y == 2.0f).All() == true);
    }

    SECTION("Vector2 - Methods")
    {
        Vector2 v(3.0f, 4.0f);
        REQUIRE((Magnitude(v) == 5.0f).All() == true);
    
        Vector2 n = Normalize(v);
        REQUIRE((Magnitude(n) == 1.0f).All() == true);
        REQUIRE((n.x == 3.0f / 5.0f).All() == true);
        REQUIRE((n.y == 4.0f / 5.0f).All() == true);

        Vector2 a(1.0f, 0.0f);
        Vector2 b(0.0f, 1.0f);
        REQUIRE((Dot(a, b) == 0.0f).All() == true);
        REQUIRE((Dot(a, a) == 1.0f).All() == true);
        REQUIRE((CosTheta(a, b) == 0.0f).All() == true);
    }

    SECTION("Vector3 - Constructors and Element Access")
    {
        Vector3 v1;
        Vector3 v2(1.0f);
        Vector3 v3(1.0f, 2.0f, 3.0f);
        Vector3 v4(v3);
        Vector3 v5(v3, v2); // v = b - a, so v2 - v3 = (1-1, 1-2, 1-3) = (0, -1, -2)

        REQUIRE((v5.x == 0.0f).All() == true);
        REQUIRE((v5.y == -1.0f).All() == true);
        REQUIRE((v5.z == -2.0f).All() == true);

        REQUIRE((v2.x == 1.0f).All() == true);
        REQUIRE((v2.y == 1.0f).All() == true);
        REQUIRE((v2.z == 1.0f).All() == true);

        REQUIRE((v3.x == 1.0f).All() == true);
        REQUIRE((v3.y == 2.0f).All() == true);
        REQUIRE((v3.z == 3.0f).All() == true);

        REQUIRE((v3[0] == 1.0f).All() == true);
        REQUIRE((v3[1] == 2.0f).All() == true);
        REQUIRE((v3[2] == 3.0f).All() == true);

        v1[0] = 4.0f;
        v1[1] = 5.0f;
        v1[2] = 6.0f;
        REQUIRE((v1.x == 4.0f).All() == true);
        REQUIRE((v1.y == 5.0f).All() == true);
        REQUIRE((v1.z == 6.0f).All() == true);
    
        REQUIRE((v4 == v3).All() == true);
    }

    SECTION("Vector3 - Operators")
    {
        Vector3 a(1.0f, 2.0f, 3.0f);
        Vector3 b(4.0f, 5.0f, 6.0f);

        Vector3 c = a + b;
        REQUIRE((c.x == 5.0f).All() == true);
        REQUIRE((c.y == 7.0f).All() == true);
        REQUIRE((c.z == 9.0f).All() == true);

        c = a - b;
        REQUIRE((c.x == -3.0f).All() == true);
        REQUIRE((c.y == -3.0f).All() == true);
        REQUIRE((c.z == -3.0f).All() == true);

        c = a * b;
        REQUIRE((c.x == 4.0f).All() == true);
        REQUIRE((c.y == 10.0f).All() == true);
        REQUIRE((c.z == 18.0f).All() == true);

        c = a / b;
        REQUIRE((c.x == 1.0f / 4.0f).All() == true);
        REQUIRE((c.y == 2.0f / 5.0f).All() == true);
        REQUIRE((c.z == 3.0f / 6.0f).All() == true);

        c = a * 2.0f;
        REQUIRE((c.x == 2.0f).All() == true);
        REQUIRE((c.y == 4.0f).All() == true);
        REQUIRE((c.z == 6.0f).All() == true);

        c = a / 2.0f;
        REQUIRE((c.x == 0.5f).All() == true);
        REQUIRE((c.y == 1.0f).All() == true);
        REQUIRE((c.z == 1.5f).All() == true);
    
        c = -a;
        REQUIRE((c.x == -1.0f).All() == true);
        REQUIRE((c.y == -2.0f).All() == true);
        REQUIRE((c.z == -3.0f).All() == true);
    }

    SECTION("Vector3 - Methods")
    {
        Vector3 v(2.0f, 3.0f, 6.0f);
        REQUIRE((Magnitude(v) == 7.0f).All() == true);
        REQUIRE((SquareMagnitude(v) == 49.0f).All() == true);
    
        Vector3 n = Normalize(v);
        REQUIRE((Magnitude(n) == 1.0f).All() == true);
        REQUIRE((n.x == 2.0f / 7.0f).All() == true);

        Vector3 a(1.0f, 0.0f, 0.0f);
        Vector3 b(0.0f, 1.0f, 0.0f);
        REQUIRE((Dot(a, b) == 0.0f).All() == true);
        REQUIRE((Dot(a, a) == 1.0f).All() == true);
    
        Vector3 cross = Cross(a, b);
        REQUIRE((cross.x == 0.0f).All() == true);
        REQUIRE((cross.y == 0.0f).All() == true);
        REQUIRE((cross.z == 1.0f).All() == true);
    
        Vector3 proj = Project(Vector3(2.0f, 2.0f, 0.0f), a);
        REQUIRE((proj.x == 2.0f).All() == true);
        REQUIRE((proj.y == 0.0f).All() == true);
        REQUIRE((proj.z == 0.0f).All() == true);
    
        Vector3 absV = Abs(Vector3(-1.0f, -2.0f, 3.0f));
        REQUIRE((absV.x == 1.0f).All() == true);
        REQUIRE((absV.y == 2.0f).All() == true);
        REQUIRE((absV.z == 3.0f).All() == true);
    }

    SECTION("Point3 - Constructors and Operations")
    {
        Point3 p1(1.0f, 2.0f, 3.0f);
        Point3 p2(4.0f, 5.0f, 6.0f);
    
        Vector3 v = p2 - p1;
        REQUIRE((v.x == 3.0f).All() == true);
        REQUIRE((v.y == 3.0f).All() == true);
        REQUIRE((v.z == 3.0f).All() == true);
    
        Point3 p3 = p1 + v;
        REQUIRE((p3.x == 4.0f).All() == true);
        REQUIRE((p3.y == 5.0f).All() == true);
        REQUIRE((p3.z == 6.0f).All() == true);
    
        REQUIRE((Distance(p1, p2) == ScalarType(std::sqrt(T(27)))).All() == true);
        REQUIRE((SquareDistance(p1, p2) == 27.0f).All() == true);
    
        Point3 c = Center(p1, p2);
        REQUIRE((c.x == 2.5f).All() == true);
        REQUIRE((c.y == 3.5f).All() == true);
        REQUIRE((c.z == 4.5f).All() == true);
    
        Point3 m1 = Min(p1, p2);
        REQUIRE((m1.x == 1.0f).All() == true);
        REQUIRE((m1.y == 2.0f).All() == true);
        REQUIRE((m1.z == 3.0f).All() == true);
    
        Point3 m2 = Max(p1, p2);
        REQUIRE((m2.x == 4.0f).All() == true);
        REQUIRE((m2.y == 5.0f).All() == true);
        REQUIRE((m2.z == 6.0f).All() == true);
    }

    SECTION("Vector4 - Constructors and Element Access")
    {
        Vector4 v1;
        Vector4 v2(1.0f);
        Vector4 v3(1.0f, 2.0f, 3.0f, 4.0f);
        Vector4 v4(Vector3(1.0f, 2.0f, 3.0f), 4.0f);
        Vector4 v5(v3, v2); // v = b - a = v2 - v3 = (1-1, 1-2, 1-3, 1-4) = (0, -1, -2, -3)

        REQUIRE((v5.x == 0.0f).All() == true);
        REQUIRE((v5.y == -1.0f).All() == true);
        REQUIRE((v5.z == -2.0f).All() == true);
        REQUIRE((v5.w == -3.0f).All() == true);

        REQUIRE((v2.x == 1.0f).All() == true);
        REQUIRE((v2.y == 1.0f).All() == true);
        REQUIRE((v2.z == 1.0f).All() == true);
        REQUIRE((v2.w == 1.0f).All() == true);

        REQUIRE((v3.x == 1.0f).All() == true);
        REQUIRE((v3.y == 2.0f).All() == true);
        REQUIRE((v3.z == 3.0f).All() == true);
        REQUIRE((v3.w == 4.0f).All() == true);

        REQUIRE((v4.x == 1.0f).All() == true);
        REQUIRE((v4.y == 2.0f).All() == true);
        REQUIRE((v4.z == 3.0f).All() == true);
        REQUIRE((v4.w == 4.0f).All() == true);

        v1[0] = 5.0f;
        v1[1] = 6.0f;
        v1[2] = 7.0f;
        v1[3] = 8.0f;
        REQUIRE((v1.x == 5.0f).All() == true);
        REQUIRE((v1.y == 6.0f).All() == true);
        REQUIRE((v1.z == 7.0f).All() == true);
        REQUIRE((v1.w == 8.0f).All() == true);
    
        Vector3 xyz = v1.xyz();
        REQUIRE((xyz.x == 5.0f).All() == true);
        REQUIRE((xyz.y == 6.0f).All() == true);
        REQUIRE((xyz.z == 7.0f).All() == true);
    
        Vector3 yzx = v1.YZX();
        REQUIRE((yzx.x == 6.0f).All() == true);
        REQUIRE((yzx.y == 7.0f).All() == true);
        REQUIRE((yzx.z == 5.0f).All() == true);
    }

    SECTION("Vector4 - Operators and Methods")
    {
        Vector4 a(1.0f, 2.0f, 3.0f, 4.0f);
        Vector4 b(5.0f, 6.0f, 7.0f, 8.0f);

        Vector4 c = a + b;
        REQUIRE((c.x == 6.0f).All() == true);
        REQUIRE((c.y == 8.0f).All() == true);
        REQUIRE((c.z == 10.0f).All() == true);
        REQUIRE((c.w == 12.0f).All() == true);

        c = a - b;
        REQUIRE((c.x == -4.0f).All() == true);
        REQUIRE((c.y == -4.0f).All() == true);
        REQUIRE((c.z == -4.0f).All() == true);
        REQUIRE((c.w == -4.0f).All() == true);

        c = a * 2.0f;
        REQUIRE((c.x == 2.0f).All() == true);
        REQUIRE((c.y == 4.0f).All() == true);
        REQUIRE((c.z == 6.0f).All() == true);
        REQUIRE((c.w == 8.0f).All() == true);
    
        REQUIRE((Dot(a, b) == 1*5 + 2*6 + 3*7 + 4*8).All() == true);
    }
}