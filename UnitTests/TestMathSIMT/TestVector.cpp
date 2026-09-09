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
        Vector2 v2(T(1));
        Vector2 v3(T(1), T(2));
        Vector2 v4(v3, v2); // v = b - a, so v2 - v3 = (1-1, 1-2) = (0, -1)

        REQUIRE((v4.x == T(0)).All() == true);
        REQUIRE((v4.y == T(-1)).All() == true);

        REQUIRE((v2.x == T(1)).All() == true);
        REQUIRE((v2.y == T(1)).All() == true);

        REQUIRE((v3.x == T(1)).All() == true);
        REQUIRE((v3.y == T(2)).All() == true);

        REQUIRE((v3[0] == T(1)).All() == true);
        REQUIRE((v3[1] == T(2)).All() == true);

        v1[0] = T(3);
        v1[1] = T(4);
        REQUIRE((v1.x == T(3)).All() == true);
        REQUIRE((v1.y == T(4)).All() == true);
    }

    SECTION("Vector2 - Operators")
    {
        Vector2 a(T(1), T(2));
        Vector2 b(T(3), T(4));

        Vector2 c = a + b;
        REQUIRE((c.x == T(4)).All() == true);
        REQUIRE((c.y == T(6)).All() == true);

        c = a - b;
        REQUIRE((c.x == -T(2)).All() == true);
        REQUIRE((c.y == -T(2)).All() == true);

        c = a * b;
        REQUIRE((c.x == T(3)).All() == true);
        REQUIRE((c.y == T(8)).All() == true);

        c = a / b;
        REQUIRE((c.x == T(1) / T(3)).All() == true);
        REQUIRE((c.y == T(2) / T(4)).All() == true);

        c = a * T(2);
        REQUIRE((c.x == T(2)).All() == true);
        REQUIRE((c.y == T(4)).All() == true);

        c = T(2) * a;
        REQUIRE((c.x == T(2)).All() == true);
        REQUIRE((c.y == T(4)).All() == true);

        c = a / T(2);
        REQUIRE((c.x == T(0.5)).All() == true);
        REQUIRE((c.y == T(1)).All() == true);
    
        a += b;
        REQUIRE((a.x == T(4)).All() == true);
        REQUIRE((a.y == T(6)).All() == true);
    
        a -= b;
        REQUIRE((a.x == T(1)).All() == true);
        REQUIRE((a.y == T(2)).All() == true);
    }

    SECTION("Vector2 - Methods")
    {
        Vector2 v(T(3), T(4));
        REQUIRE((Magnitude(v) == T(5)).All() == true);
    
        Vector2 n = Normalize(v);
        REQUIRE((Magnitude(n) == T(1)).All() == true);
        REQUIRE((n.x == T(3) / T(5)).All() == true);
        REQUIRE((n.y == T(4) / T(5)).All() == true);

        Vector2 a(T(1), T(0));
        Vector2 b(T(0), T(1));
        REQUIRE((Dot(a, b) == T(0)).All() == true);
        REQUIRE((Dot(a, a) == T(1)).All() == true);
        REQUIRE((CosTheta(a, b) == T(0)).All() == true);
    }

    SECTION("Vector3 - Constructors and Element Access")
    {
        Vector3 v1;
        Vector3 v2(T(1));
        Vector3 v3(T(1), T(2), T(3));
        Vector3 v4(v3);
        Vector3 v5(v3, v2); // v = b - a, so v2 - v3 = (1-1, 1-2, 1-3) = (0, -1, -2)

        REQUIRE((v5.x == T(0)).All() == true);
        REQUIRE((v5.y == T(-1)).All() == true);
        REQUIRE((v5.z == T(-2)).All() == true);

        REQUIRE((v2.x == T(1)).All() == true);
        REQUIRE((v2.y == T(1)).All() == true);
        REQUIRE((v2.z == T(1)).All() == true);

        REQUIRE((v3.x == T(1)).All() == true);
        REQUIRE((v3.y == T(2)).All() == true);
        REQUIRE((v3.z == T(3)).All() == true);

        REQUIRE((v3[0] == T(1)).All() == true);
        REQUIRE((v3[1] == T(2)).All() == true);
        REQUIRE((v3[2] == T(3)).All() == true);

        v1[0] = T(4);
        v1[1] = T(5);
        v1[2] = T(6);
        REQUIRE((v1.x == T(4)).All() == true);
        REQUIRE((v1.y == T(5)).All() == true);
        REQUIRE((v1.z == T(6)).All() == true);
    
        REQUIRE((v4.x == v3.x).All() == true);
        REQUIRE((v4.y == v3.y).All() == true);
        REQUIRE((v4.z == v3.z).All() == true);
    }

    SECTION("Vector3 - Operators")
    {
        Vector3 a(T(1), T(2), T(3));
        Vector3 b(T(4), T(5), T(6));

        Vector3 c = a + b;
        REQUIRE((c.x == T(5)).All() == true);
        REQUIRE((c.y == T(7)).All() == true);
        REQUIRE((c.z == T(9)).All() == true);

        c = a - b;
        REQUIRE((c.x == T(-3)).All() == true);
        REQUIRE((c.y == T(-3)).All() == true);
        REQUIRE((c.z == T(-3)).All() == true);

        c = a * b;
        REQUIRE((c.x == T(4)).All() == true);
        REQUIRE((c.y == T(10)).All() == true);
        REQUIRE((c.z == T(18)).All() == true);

        c = a / b;
        REQUIRE((c.x == T(1) / T(4)).All() == true);
        REQUIRE((c.y == T(2) / T(5)).All() == true);
        REQUIRE((c.z == T(3) / T(6)).All() == true);

        c = a * T(2);
        REQUIRE((c.x == T(2)).All() == true);
        REQUIRE((c.y == T(4)).All() == true);
        REQUIRE((c.z == T(6)).All() == true);

        c = a / T(2);
        REQUIRE((c.x == T(0.5)).All() == true);
        REQUIRE((c.y == T(1)).All() == true);
        REQUIRE((c.z == T(1.5)).All() == true);
    
        c = -a;
        REQUIRE((c.x == T(-1)).All() == true);
        REQUIRE((c.y == T(-2)).All() == true);
        REQUIRE((c.z == T(-3)).All() == true);
    }

    SECTION("Vector3 - Methods")
    {
        Vector3 v(T(2), T(3), T(6));
        REQUIRE((Magnitude(v) == T(7)).All() == true);
        REQUIRE((SquareMagnitude(v) == T(49)).All() == true);
    
        Vector3 n = Normalize(v);
        REQUIRE((Magnitude(n) == T(1)).All() == true);
        REQUIRE((n.x == T(2) / T(7)).All() == true);

        Vector3 a(T(1), T(0), T(0));
        Vector3 b(T(0), T(1), T(0));
        REQUIRE((Dot(a, b) == T(0)).All() == true);
        REQUIRE((Dot(a, a) == T(1)).All() == true);
    
        Vector3 cross = Cross(a, b);
        REQUIRE((cross.x == T(0)).All() == true);
        REQUIRE((cross.y == T(0)).All() == true);
        REQUIRE((cross.z == T(1)).All() == true);
    
        Vector3 proj = Project(Vector3(T(2), T(2), T(0)), a);
        REQUIRE((proj.x == T(2)).All() == true);
        REQUIRE((proj.y == T(0)).All() == true);
        REQUIRE((proj.z == T(0)).All() == true);
    
        Vector3 absV = Abs(Vector3(-T(1), -T(2), T(3)));
        REQUIRE((absV.x == T(1)).All() == true);
        REQUIRE((absV.y == T(2)).All() == true);
        REQUIRE((absV.z == T(3)).All() == true);
    }

    SECTION("Point3 - Constructors and Operations")
    {
        Point3 p1(T(1), T(2), T(3));
        Point3 p2(T(4), T(5), T(6));
    
        Vector3 v = p2 - p1;
        REQUIRE((v.x == T(3)).All() == true);
        REQUIRE((v.y == T(3)).All() == true);
        REQUIRE((v.z == T(3)).All() == true);
    
        Point3 p3 = p1 + v;
        REQUIRE((p3.x == T(4)).All() == true);
        REQUIRE((p3.y == T(5)).All() == true);
        REQUIRE((p3.z == T(6)).All() == true);
    
        REQUIRE((Distance(p1, p2) == T(std::sqrt(T(27)))).All() == true);
        REQUIRE((SquareDistance(p1, p2) == T(27)).All() == true);
    
        Point3 c = Center(p1, p2);
        REQUIRE((c.x == T(2.5)).All() == true);
        REQUIRE((c.y == T(3.5)).All() == true);
        REQUIRE((c.z == T(4.5)).All() == true);
    
        Point3 m1 = Min(p1, p2);
        REQUIRE((m1.x == T(1)).All() == true);
        REQUIRE((m1.y == T(2)).All() == true);
        REQUIRE((m1.z == T(3)).All() == true);
    
        Point3 m2 = Max(p1, p2);
        REQUIRE((m2.x == T(4)).All() == true);
        REQUIRE((m2.y == T(5)).All() == true);
        REQUIRE((m2.z == T(6)).All() == true);
    }

    SECTION("Vector4 - Constructors and Element Access")
    {
        Vector4 v1;
        Vector4 v2(T(1));
        Vector4 v3(T(1), T(2), T(3), T(4));
        Vector4 v4(Vector3(T(1), T(2), T(3)), T(4));
        Vector4 v5(v3, v2); // v = b - a = v2 - v3 = (1-1, 1-2, 1-3, 1-4) = (0, -1, -2, -3)

        REQUIRE((v5.x == T(0)).All() == true);
        REQUIRE((v5.y == T(-1)).All() == true);
        REQUIRE((v5.z == T(-2)).All() == true);
        REQUIRE((v5.w == T(-3)).All() == true);

        REQUIRE((v2.x == T(1)).All() == true);
        REQUIRE((v2.y == T(1)).All() == true);
        REQUIRE((v2.z == T(1)).All() == true);
        REQUIRE((v2.w == T(1)).All() == true);

        REQUIRE((v3.x == T(1)).All() == true);
        REQUIRE((v3.y == T(2)).All() == true);
        REQUIRE((v3.z == T(3)).All() == true);
        REQUIRE((v3.w == T(4)).All() == true);

        REQUIRE((v4.x == T(1)).All() == true);
        REQUIRE((v4.y == T(2)).All() == true);
        REQUIRE((v4.z == T(3)).All() == true);
        REQUIRE((v4.w == T(4)).All() == true);

        v1[0] = T(5);
        v1[1] = T(6);
        v1[2] = T(7);
        v1[3] = T(8);
        REQUIRE((v1.x == T(5)).All() == true);
        REQUIRE((v1.y == T(6)).All() == true);
        REQUIRE((v1.z == T(7)).All() == true);
        REQUIRE((v1.w == T(8)).All() == true);
    
        Vector3 xyz = v1.xyz();
        REQUIRE((xyz.x == T(5)).All() == true);
        REQUIRE((xyz.y == T(6)).All() == true);
        REQUIRE((xyz.z == T(7)).All() == true);
    
        Vector3 yzx = v1.YZX();
        REQUIRE((yzx.x == T(6)).All() == true);
        REQUIRE((yzx.y == T(7)).All() == true);
        REQUIRE((yzx.z == T(5)).All() == true);
    }

    SECTION("Vector4 - Operators and Methods")
    {
        Vector4 a(T(1), T(2), T(3), T(4));
        Vector4 b(T(5), T(6), T(7), T(8));

        Vector4 c = a + b;
        REQUIRE((c.x == T(6)).All() == true);
        REQUIRE((c.y == T(8)).All() == true);
        REQUIRE((c.z == T(10)).All() == true);
        REQUIRE((c.w == T(12)).All() == true);

        c = a - b;
        REQUIRE((c.x == T(-4)).All() == true);
        REQUIRE((c.y == T(-4)).All() == true);
        REQUIRE((c.z == T(-4)).All() == true);
        REQUIRE((c.w == T(-4)).All() == true);

        c = a * T(2);
        REQUIRE((c.x == T(2)).All() == true);
        REQUIRE((c.y == T(4)).All() == true);
        REQUIRE((c.z == T(6)).All() == true);
        REQUIRE((c.w == T(8)).All() == true);
    
        REQUIRE((Dot(a, b) == 1*5 + 2*6 + 3*7 + 4*8).All() == true);
    }
}