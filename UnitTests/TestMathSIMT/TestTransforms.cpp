#include <catch2/catch_all.hpp>
#include "MathSimt/RMath.h"

using namespace Math::Simt;

TEMPLATE_TEST_CASE_SIG("Transforms", "[Scalar]",
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
    
    SECTION("Transforms - Reference Translation")
    {
        Transform4 trans = Transform4::Translation(10.0f, -5.0f, 3.0f);
    
        // Translation applied to a point should shift it
        Point3 p(1.0f, 1.0f, 1.0f);
        Point3 pTransformed = trans * p;
        REQUIRE(pTransformed.x == Catch::Approx(11.0f));
        REQUIRE(pTransformed.y == Catch::Approx(-4.0f));
        REQUIRE(pTransformed.z == Catch::Approx(4.0f));

        // Translation applied to a vector should have no effect
        Vector3 v(1.0f, 1.0f, 1.0f);
        Vector3 vTransformed = trans * v;
        REQUIRE(vTransformed.x == Catch::Approx(1.0f));
        REQUIRE(vTransformed.y == Catch::Approx(1.0f));
        REQUIRE(vTransformed.z == Catch::Approx(1.0f));
    }

    SECTION("Transforms - Reference Scale")
    {
        Transform4 scale = Transform4::Scale(2.0f, 0.5f, -1.0f);

        Point3 p(10.0f, 10.0f, 10.0f);
        Point3 pScaled = scale * p;
        REQUIRE(pScaled.x == Catch::Approx(20.0f));
        REQUIRE(pScaled.y == Catch::Approx(5.0f));
        REQUIRE(pScaled.z == Catch::Approx(-10.0f));

        Vector3 v(10.0f, 10.0f, 10.0f);
        Vector3 vScaled = scale * v;
        REQUIRE(vScaled.x == Catch::Approx(20.0f));
        REQUIRE(vScaled.y == Catch::Approx(5.0f));
        REQUIRE(vScaled.z == Catch::Approx(-10.0f));
    }

    SECTION("Transforms - Reference Rotations")
    {
        ScalarType piOver2 = ScalarType(M_PI) / 2;
    
        // Right-handed rotation around X-axis
        Transform4 rotX = Transform4::Rotation(piOver2);
        Point3 px(0.0f, 1.0f, 0.0f);
        Point3 pxRot = rotX * px;
        // Y unit vector rotated by 90 degrees around X goes to Z
        REQUIRE(Catch::Approx(pxRot.x).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(pxRot.y).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(pxRot.z).margin(0.0001f) == 1.0f);

        // Right-handed rotation around Y-axis
        Transform4 rotY = Transform4::RotationY(piOver2);
        Point3 py(1.0f, 0.0f, 0.0f);
        Point3 pyRot = rotY * py;
        // X unit vector rotated by 90 degrees around Y goes to -Z
        REQUIRE(Catch::Approx(pyRot.x).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(pyRot.y).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(pyRot.z).margin(0.0001f) == -1.0f);

        // Right-handed rotation around Z-axis
        Transform4 rotZ = Transform4::RotationZ(piOver2);
        Point3 pz(1.0f, 0.0f, 0.0f);
        Point3 pzRot = rotZ * pz;
        // X unit vector rotated by 90 degrees around Z goes to Y
        REQUIRE(Catch::Approx(pzRot.x).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(pzRot.y).margin(0.0001f) == 1.0f);
        REQUIRE(Catch::Approx(pzRot.z).margin(0.0001f) == 0.0f);

        // Arbitrary axis rotation (around Y-axis as test)
        Transform4 rotA = Transform4::Rotation(Vector3(0.0f, 1.0f, 0.0f), piOver2);
        Point3 pa(1.0f, 0.0f, 0.0f);
        Point3 paRot = rotA * pa;
        REQUIRE(Catch::Approx(paRot.x).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(paRot.y).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(paRot.z).margin(0.0001f) == -1.0f);
    }

    SECTION("Transforms - Reference Reflection")
    {
        // Reflection across YZ plane (normal is X-axis)
        Plane plane(1.0f, 0.0f, 0.0f, 0.0f); 
        Transform4 refT = Transform4::Reflection(plane);
    
        Point3 p(5.0f, 3.0f, -2.0f);
        Point3 pRef = refT * p;
        REQUIRE(pRef.x == Catch::Approx(-5.0f));
        REQUIRE(pRef.y == Catch::Approx(3.0f));
        REQUIRE(pRef.z == Catch::Approx(-2.0f));

        // Vector reflection
        Transform4 refVec = Transform4::Reflection(Vector3(0.0f, 1.0f, 0.0f));
        Point3 p2(5.0f, 3.0f, -2.0f);
        Point3 p2Ref = refVec * p2;
        REQUIRE(p2Ref.x == Catch::Approx(5.0f));
        REQUIRE(p2Ref.y == Catch::Approx(-3.0f));
        REQUIRE(p2Ref.z == Catch::Approx(-2.0f));
    }

    SECTION("Transforms - Reference Inverse")
    {
        Transform4 trans = Transform4::Translation(10.0f, 20.0f, 30.0f);
        Transform4 invTrans = Inverse(trans);
    
        Point3 p(5.0f, 5.0f, 5.0f);
        Point3 pIdentity = invTrans * (trans * p);
    
        REQUIRE(pIdentity.x == Catch::Approx(5.0f));
        REQUIRE(pIdentity.y == Catch::Approx(5.0f));
        REQUIRE(pIdentity.z == Catch::Approx(5.0f));
    
        Point3 pInv = invTrans * p;
        REQUIRE(pInv.x == Catch::Approx(-5.0f));
        REQUIRE(pInv.y == Catch::Approx(-15.0f));
        REQUIRE(pInv.z == Catch::Approx(-25.0f));
    }

    SECTION("Transforms - Reference Composition")
    {
        Transform4 scale = Transform4::Scale(2.0f, 2.0f, 2.0f);
        Transform4 trans = Transform4::Translation(10.0f, 0.0f, 0.0f);
    
        // Expected composition T = trans * scale
        // Applying T to p means trans * (scale * p)
        Transform4 combined = trans * scale;
        Point3 p(1.0f, 1.0f, 1.0f);
        Point3 pCombined = combined * p;
    
        // Scale first (2,2,2) then translate by (10,0,0) => (12, 2, 2)
        REQUIRE(pCombined.x == Catch::Approx(12.0f));
        REQUIRE(pCombined.y == Catch::Approx(2.0f));
        REQUIRE(pCombined.z == Catch::Approx(2.0f));
    }

    SECTION("Transforms - Reference LookAt")
    {
        Vector3 eye(0.0f, 0.0f, 5.0f);
        Vector3 center(0.0f, 0.0f, 0.0f);
        Vector3 up(0.0f, 1.0f, 0.0f);
    
        Matrix4 lookAt = MakeLookAtView(eye, center, up);
        Transform4 lookAtT(lookAt);
    
        // The eye position should map to the origin in view space
        Point3 pEyePoint(0.0f, 0.0f, 5.0f);
        Point3 pView = lookAtT * pEyePoint;
    
        REQUIRE(Catch::Approx(pView.x).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(pView.y).margin(0.0001f) == 0.0f);
        REQUIRE(Catch::Approx(pView.z).margin(0.0001f) == 0.0f);
    }

    SECTION("Transforms - Reference Projection Setup")
    {
        // Simply check standard layout of an orthogonal projection matrix
        ScalarType left = -10.0f, right = 10.0f, bottom = -10.0f, top = 10.0f, near = 1.0f, far = 100.0f;
        Matrix4 ortho = MakeOrthoProjection(left, right, top, bottom, near, far);
    
        // Ortho matrix M(0,0) = 2/(right - left) = 2/20 = 0.1
        // M(1,1) = 2/(top - bottom) = 2/20 = 0.1
        // M(2,2) = -2/(far - near) = -2/99 or similar depending on depth convention
        REQUIRE(Catch::Approx(ortho(0, 0)).margin(0.0001f) == 0.1f);
        REQUIRE(Catch::Approx(ortho(1, 1)).margin(0.0001f) == 0.1f);
    
        ScalarType fovY = ScalarType(M_PI) / 2;
        ScalarType aspect = 1.0f;
        Matrix4 persp = Transform4::Perspective(fovY, aspect, near, far);
    
        // Perspective M(0,0) = 1 / (aspect * tan(fovY/2)) = 1 / tan(PI/4) = 1.0
        // M(1,1) = 1 / tan(fovY/2) = 1.0
        REQUIRE(Catch::Approx(persp(0, 0)).margin(0.0001f) == 1.0f);
        REQUIRE(Catch::Approx(persp(1, 1)).margin(0.0001f) == 1.0f);
    }
}