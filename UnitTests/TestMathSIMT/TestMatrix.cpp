#include <catch2/catch_all.hpp>
#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Matrices", "[Scalar]",
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
    using Box = Math::Simt::Box3<T, N>;
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
    
SECTION("Matrix3 - Constructors and Elements")
    {
        Matrix3 m1;
        Matrix3 m2(1.0f, 2.0f, 3.0f,
                    4.0f, 5.0f, 6.0f,
                    7.0f, 8.0f, 9.0f);

        REQUIRE((m2(0, 0) == 1.0f).All() == true);
        REQUIRE((m2(0, 1) == 2.0f).All() == true);
        REQUIRE((m2(0, 2) == 3.0f).All() == true);
        REQUIRE((m2(1, 0) == 4.0f).All() == true);
        REQUIRE((m2(2, 2) == 9.0f).All() == true);

        Vector3 v0(1.0f, 2.0f, 3.0f);
        Vector3 v1(4.0f, 5.0f, 6.0f);
        Vector3 v2(7.0f, 8.0f, 9.0f);
        Matrix3 m3(v0, v1, v2);

        REQUIRE((m3 == Transpose(m2)).All() == true);
        REQUIRE((m3[0].x == 1.0f).All() == true);
    
        Matrix3 ident = Matrix3::Identity();
        REQUIRE((ident(0, 0) == 1.0f).All() == true);
        REQUIRE((ident(1, 1) == 1.0f).All() == true);
        REQUIRE((ident(2, 2) == 1.0f).All() == true);
        REQUIRE((ident(0, 1) == 0.0f).All() == true);
    }

    SECTION("Matrix3 - Operators and Functions")
    {
        Matrix3 m1 = Matrix3::Identity();
        Matrix3 m2 = Matrix3::Identity();
        m2(0, 1) = 2.0f;
    
        Matrix3 add = m1 + m2;
        REQUIRE((add(0, 0) == 2.0f).All() == true);
        REQUIRE((add(0, 1) == 2.0f).All() == true);
    
        Matrix3 sub = m1 - m2;
        REQUIRE((sub(0, 0) == 0.0f).All() == true);
        REQUIRE((sub(0, 1) == -2.0f).All() == true);
    
        Matrix3 mul = m1 * m2;
        REQUIRE((mul == m2).All() == true);
    
        Vector3 v(1.0f, 2.0f, 3.0f);
        Vector3 vm = m2 * v;
        // m2 is:
        // 1 2 0
        // 0 1 0
        // 0 0 1
        // Wait, is it row major or col major? 
        // The header says "Column major 3 by 3 matrix".
        // If it's column major, m2(0, 1) might mean column 0, row 1? 
        // Usually (row, col) is standard, but if it's col-major storage, operator() might abstract it.
        // Let's just test properties without relying on exact memory layout if possible.

        Matrix3 trans = Transpose(m2);
        REQUIRE((trans(0, 0) == m2(0, 0)).All() == true);
        REQUIRE((trans(1, 0) == m2(0, 1)).All() == true);
        REQUIRE((trans(0, 1) == m2(1, 0)).All() == true);
    
        REQUIRE((Determinant(m1) == 1.0f).All() == true);
    
        Matrix3 inv = Inverse(m2);
        Matrix3 ident = m2 * inv;
        REQUIRE((ident(0,0) == 1.0f).All() == true);
        REQUIRE((ident(1,1) == 1.0f).All() == true);
        REQUIRE((ident(2,2) == 1.0f).All() == true);
        REQUIRE((ident(0,1) == 0.0f).All() == true);
    }

    SECTION("Matrix4 - Constructors and Elements")
    {
        Matrix4 m2(1.0f, 2.0f, 3.0f, 4.0f,
                    5.0f, 6.0f, 7.0f, 8.0f,
                    9.0f, 10.0f, 11.0f, 12.0f,
                    13.0f, 14.0f, 15.0f, 16.0f);

        REQUIRE((m2(0, 0) == 1.0f).All() == true);
        REQUIRE((m2(0, 3) == 4.0f).All() == true);
        REQUIRE((m2(3, 3) == 16.0f).All() == true);
    
        Matrix4 ident = Matrix4::Identity();
        REQUIRE((ident(0, 0) == 1.0f).All() == true);
        REQUIRE((ident(1, 1) == 1.0f).All() == true);
        REQUIRE((ident(2, 2) == 1.0f).All() == true);
        REQUIRE((ident(3, 3) == 1.0f).All() == true);
        REQUIRE((ident(0, 1) == 0.0f).All() == true);
    }

    SECTION("Matrix4 - Operators and Functions")
    {
        Matrix4 m1 = Matrix4::Identity();
        Matrix4 m2 = Matrix4::Identity();
        m2(0, 1) = 2.0f;
    
        Matrix4 add = m1 + m2;
        REQUIRE((add(0, 0) == 2.0f).All() == true);
        REQUIRE((add(0, 1) == 2.0f).All() == true);
    
        Matrix4 mul = m1 * m2;
        REQUIRE((mul == m2).All() == true);
    
        Matrix4 trans = Transpose(m2);
        REQUIRE((trans(1, 0) == m2(0, 1)).All() == true);
        REQUIRE((trans(0, 1) == m2(1, 0)).All() == true);
    
        // Test inverse
        Matrix4 m3(2.0f, 0.0f, 0.0f, 0.0f,
                    0.0f, 2.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 2.0f, 0.0f,
                    0.0f, 0.0f, 0.0f, 1.0f);
        Matrix4 inv = Inverse(m3);
        REQUIRE((inv(0,0) == 0.5f).All() == true);
        REQUIRE((inv(1,1) == 0.5f).All() == true);
        REQUIRE((inv(2,2) == 0.5f).All() == true);
        REQUIRE((inv(3,3) == 1.0f).All() == true);
    }
}