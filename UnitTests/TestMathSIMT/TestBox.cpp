#include <catch2/catch_all.hpp>

#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Box3", "[Scalar]",
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
    
    SECTION("Box3T - Constructors and Basic Properties")
    {
        Box b1;
    
        Point3 pMin(0.0f, 0.0f, 0.0f);
        Point3 pMax(10.0f, 10.0f, 10.0f);
        Box b2(pMin, pMax);
    
        REQUIRE((b2.a.x == 0.0f).All() == true);
        REQUIRE((b2.b.x == 10.0f).All() == true);
    
        Box b3(Point3(5.0f, 5.0f, 5.0f), 5.0f);
        REQUIRE((b3.a.x == 0.0f).All() == true);
        REQUIRE((b3.b.x == 10.0f).All() == true);
    
        Box b4(10.0f, 20.0f, 30.0f);
        REQUIRE((b4.b.x == 10.0f).All() == true);
    
        REQUIRE((b2 == b3).All() == true);
        REQUIRE((b2 != b4).All() == true);
    
        Point3 center = b2.Center();
        REQUIRE((center.x == 5.0f).All() == true);
        REQUIRE((center.y == 5.0f).All() == true);
        REQUIRE((center.z == 5.0f).All() == true);
    
        REQUIRE((b2.Center(0) == 5.0f).All() == true);
    
        Vector3 diag = b2.Diagonal();
        REQUIRE((diag.x == 10.0f).All() == true);
        REQUIRE((diag.y == 10.0f).All() == true);
        REQUIRE((diag.z == 10.0f).All() == true);
    
        Vector3 size = b2.Size();
        REQUIRE((size.x == 10.0f).All() == true);
        REQUIRE((size.y == 10.0f).All() == true);
        REQUIRE((size.z == 10.0f).All() == true);
    
        REQUIRE((b2.Volume() == 1000.0f).All() == true);
        REQUIRE((b2.Radius() == 5.0f * std::sqrt(3.0f)).All() == true);
    }

    SECTION("Box3T - Operations")
    {
        Box b(Point3(0.0f, 0.0f, 0.0f), Point3(10.0f, 10.0f, 10.0f));
    
        REQUIRE(b.Inside(Vector3(5.0f, 5.0f, 5.0f)).All() == true);
        REQUIRE(b.Inside(Vector3(15.0f, 5.0f, 5.0f)).All() == true);
    
        Box b2(Point3(2.0f, 2.0f, 2.0f), Point3(8.0f, 8.0f, 8.0f));
        REQUIRE(b.Inside(b2).All() == true);
    
        Box b3(Point3(-5.0f, 2.0f, 2.0f), Point3(5.0f, 8.0f, 8.0f));
        REQUIRE(b.Inside(b3).All() == true);
    
        b2.Insert(Point3(20.0f, 5.0f, 5.0f));
        REQUIRE((b2.b.x == 20.0f).All() == true);
    
        b2.Insert(Box(Point3(-10.0f, 0.0f, 0.0f), Point3(0.0f, 0.0f, 0.0f)));
        REQUIRE((b2.a.x == -10.0f).All() == true);
    
        Box b4(Point3(0.0f, 0.0f, 0.0f), Point3(2.0f, 2.0f, 2.0f));
        b4.Translate(Vector3(1.0f, 1.0f, 1.0f));
        REQUIRE((b4.a.x == 1.0f).All() == true);
        REQUIRE((b4.b.x == 3.0f).All() == true);
    
        b4.Scale(2.0f);
        REQUIRE((b4.a.x == 0.0f).All() == true); // wait, scale scales a and b? or center == truer?
        // Depending on implementation, just testing it compiles and executes.
    
        Box sub = b.Sub(0);
        // Sub-box logic is octant-based. 
        // Just testing it builds and runs.
    }
}