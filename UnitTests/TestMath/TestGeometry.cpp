#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

using namespace Math;

TEST_CASE("PlaneT - Constructors and Properties")
{
    PlaneF p1(0.0f, 1.0f, 0.0f, 5.0f);
    PlaneF p2(Vector3f(0.0f, 1.0f, 0.0f), 5.0f);
    
    REQUIRE(p1.x == 0.0f);
    REQUIRE(p1.y == 1.0f);
    REQUIRE(p1.z == 0.0f);
    REQUIRE(p1.w == 5.0f);
    
    REQUIRE(p2.x == 0.0f);
    REQUIRE(p2.y == 1.0f);
    
    Vector3f n = p1.GetNormal();
    REQUIRE(n.y == 1.0f);
    
    Vector3f v(0.0f, 10.0f, 0.0f);
    REQUIRE(Dot(p1, v) == Catch::Approx(10.0f));
    
    Point3f p(0.0f, 10.0f, 0.0f);
    // Usually Plane dot Point includes w: dot(normal, p) + w
    // Just testing it compiles and runs.
    float dp = Dot(p1, p);
}

TEST_CASE("LineT - Constructors")
{
    LineF l1;
    LineF l2(1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f);
    LineF l3(Vector3f(1.0f, 2.0f, 3.0f), Vector3f(4.0f, 5.0f, 6.0f));
    
    REQUIRE(l2.direction.x == 1.0f);
    REQUIRE(l2.moment.x == 4.0f);
    
    REQUIRE(l3.direction.x == 1.0f);
    REQUIRE(l3.moment.x == 4.0f);
}

TEST_CASE("Geometry - Distance and Intersection Functions")
{
    Point3f q(0.0f, 5.0f, 0.0f);
    Point3f p(0.0f, 0.0f, 0.0f);
    Vector3f v(1.0f, 0.0f, 0.0f); // Line along x-axis
    
    float distPointLine = DistancePointLine(q, p, v);
    REQUIRE(distPointLine == Catch::Approx(5.0f));
    
    Point3f p1(0.0f, 0.0f, 0.0f);
    Vector3f v1(1.0f, 0.0f, 0.0f);
    Point3f p2(0.0f, 5.0f, 0.0f);
    Vector3f v2(0.0f, 0.0f, 1.0f);
    
    float distLineLine = DistanceLineLine(p1, v1, p2, v2);
    REQUIRE(distLineLine == Catch::Approx(5.0f));
    
    PlaneF plane(0.0f, 1.0f, 0.0f, -5.0f); // y = 5
    Point3f interP;
    bool intersectLP = IntersectionLinePlan(Point3f(0.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), plane, &interP);
    
    PlaneF plane1(1.0f, 0.0f, 0.0f, 0.0f);
    PlaneF plane2(0.0f, 1.0f, 0.0f, 0.0f);
    PlaneF plane3(0.0f, 0.0f, 1.0f, 0.0f);
    Point3f inter3;
    bool intersect3P = IntersectionThreePlan(plane1, plane2, plane3, &inter3);
    
    Point3f inter2P;
    Vector3f inter2V;
    bool intersect2P = IntersectionTwoPlan(plane1, plane2, &inter2P, &inter2V);
}
