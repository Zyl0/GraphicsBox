#include <catch2/catch_all.hpp>
#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Geometry", "[Scalar]",
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
    
    SECTION("PlaneT - Constructors and Properties")
    {
        Plane p1(0.0f, 1.0f, 0.0f, 5.0f);
        Plane p2(Vector3(0.0f, 1.0f, 0.0f), 5.0f);
    
        REQUIRE((p1.x == 0.0f).All() == true);
        REQUIRE((p1.y == 1.0f).All() == true);
        REQUIRE((p1.z == 0.0f).All() == true);
        REQUIRE((p1.w == 5.0f).All() == true);
    
        REQUIRE((p2.x == 0.0f).All() == true);
        REQUIRE((p2.y == 1.0f).All() == true);
    
        Vector3 n = p1.GetNormal();
        REQUIRE((n.y == 1.0f).All() == true);
    
        Vector3 v(0.0f, 10.0f, 0.0f);
        REQUIRE((Dot(p1, v) == 10.0f).All() == true);
    
        Point3 p(0.0f, 10.0f, 0.0f);
        // Usually Plane dot Point includes w: dot(normal, p) + w
        // Just testing it compiles and runs.
        ScalarType dp = Dot(p1, p);
    }

    SECTION("LineT - Constructors")
    {
        Line l1;
        Line l2(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
        Line l3(Vector3(1.0f, 2.0f, 3.0f), Vector3(4.0f, 5.0f, 6.0f));
    
        REQUIRE((l2.direction.x == 1.0f).All() == true);
        REQUIRE((l2.moment.x == 4.0f).All() == true);
    
        REQUIRE((l3.direction.x == 1.0f).All() == true);
        REQUIRE((l3.moment.x == 4.0f).All() == true);
    }

    SECTION("Geometry - Distance and Intersection Functions")
    {
        Point3 q(0.0f, 5.0f, 0.0f);
        Point3 p(0.0f, 0.0f, 0.0f);
        Vector3 v(1.0f, 0.0f, 0.0f); // Line along x-axis
    
        ScalarType distPointLine = DistancePointLine(q, p, v);
        REQUIRE((distPointLine == 5.0f).All() == true);
    
        Point3 p1(0.0f, 0.0f, 0.0f);
        Vector3 v1(1.0f, 0.0f, 0.0f);
        Point3 p2(0.0f, 5.0f, 0.0f);
        Vector3 v2(0.0f, 0.0f, 1.0f);
    
        ScalarType distLineLine = DistanceLineLine(p1, v1, p2, v2);
        REQUIRE((distLineLine == 5.0f).All() == true);
    
        Plane plane(0.0f, 1.0f, 0.0f, -5.0f); // y = 5
        Point3 interP;
        MaskType intersectLP = IntersectionLinePlan(Point3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f), plane, &interP);
    
        Plane plane1(1.0f, 0.0f, 0.0f, 0.0f);
        Plane plane2(0.0f, 1.0f, 0.0f, 0.0f);
        Plane plane3(0.0f, 0.0f, 1.0f, 0.0f);
        Point3 inter3;
        MaskType intersect3P = IntersectionThreePlan(plane1, plane2, plane3, &inter3);
    
        Point3 inter2P;
        Vector3 inter2V;
        MaskType intersect2P = IntersectionTwoPlan(plane1, plane2, &inter2P, &inter2V);
    }
}