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
        REQUIRE((pTransformed.x == T(11)).All() == true);
        REQUIRE((pTransformed.y == T(-4)).All() == true);
        REQUIRE((pTransformed.z == T(4)).All() == true);

        // Translation applied to a vector should have no effect
        Vector3 v(1.0f, 1.0f, 1.0f);
        Vector3 vTransformed = trans * v;
        REQUIRE((vTransformed.x == T(1)).All() == true);
        REQUIRE((vTransformed.y == T(1)).All() == true);
        REQUIRE((vTransformed.z == T(1)).All() == true);
    }

    SECTION("Transforms - Reference Scale")
    {
        Transform4 scale = Transform4::Scale(2.0f, 0.5f, -1.0f);

        Point3 p(10.0f, 10.0f, 10.0f);
        Point3 pScaled = scale * p;
        REQUIRE((pScaled.x == T(20.0f)).All() == true);
        REQUIRE((pScaled.y == T(5.0f)).All() == true);
        REQUIRE((pScaled.z == T(-10.0f)).All() == true);

        Vector3 v(10.0f, 10.0f, 10.0f);
        Vector3 vScaled = scale * v;
        REQUIRE((vScaled.x == T(20)).All() == true);
        REQUIRE((vScaled.y == T(5)).All() == true);
        REQUIRE((vScaled.z == T(-10)).All() == true);
    }

    SECTION("Transforms - Reference Rotations")
    {
        ScalarType piOver2 = ScalarType(M_PI) / T(2);
    
        // Right-handed rotation around X-axis
        Transform4 rotX = Transform4::RotationX(piOver2);
        Point3 px(0.0f, 1.0f, 0.0f);
        Point3 pxRot = rotX * px;
        // Y unit vector rotated by 90 degrees around X goes to Z
        REQUIRE((pxRot.x == T(0)).All() == true);
        REQUIRE((pxRot.y == T(0)).All() == true);
        REQUIRE((pxRot.z == T(1)).All() == true);

        // Right-handed rotation around Y-axis
        Transform4 rotY = Transform4::RotationY(piOver2);
        Point3 py(1.0f, 0.0f, 0.0f);
        Point3 pyRot = rotY * py;
        // X unit vector rotated by 90 degrees around Y goes to -Z
        REQUIRE((pyRot.x == T(0)).All() == true);
        REQUIRE((pyRot.y == T(0)).All() == true);
        REQUIRE((pyRot.z == T(-1)).All() == true);

        // Right-handed rotation around Z-axis
        Transform4 rotZ = Transform4::RotationZ(piOver2);
        Point3 pz(1.0f, 0.0f, 0.0f);
        Point3 pzRot = rotZ * pz;
        // X unit vector rotated by 90 degrees around Z goes to Y
        REQUIRE((pzRot.x == T(0)).All() == true);
        REQUIRE((pzRot.y == T(1)).All() == true);
        REQUIRE((pzRot.z == T(0)).All() == true);

        // Arbitrary axis rotation (around Y-axis as test)
        Transform4 rotA = Transform4::Rotation(Vector3(0.0f, 1.0f, 0.0f), piOver2);
        Point3 pa(1.0f, 0.0f, 0.0f);
        Point3 paRot = rotA * pa;
        REQUIRE((paRot.x == T(0)).All() == true);
        REQUIRE((paRot.y == T(0)).All() == true);
        REQUIRE((paRot.z == T(-1)).All() == true);
    }

    SECTION("Transforms - Reference Reflection")
    {
        // Reflection across YZ plane (normal is X-axis)
        Plane plane(1.0f, 0.0f, 0.0f, 0.0f); 
        Transform4 refT = Transform4::Reflection(plane);
    
        Point3 p(5.0f, 3.0f, -2.0f);
        Point3 pRef = refT * p;
        REQUIRE((pRef.x == T(-5)).All() == true);
        REQUIRE((pRef.y == T(3)).All() == true);
        REQUIRE((pRef.z == T(-2)).All() == true);

        // Vector reflection
        Transform4 refVec = Transform4::Reflection(Vector3(0.0f, 1.0f, 0.0f));
        Point3 p2(5.0f, 3.0f, -2.0f);
        Point3 p2Ref = refVec * p2;
        REQUIRE((p2Ref.x == T(5)).All() == true);
        REQUIRE((p2Ref.y == T(-3)).All() == true);
        REQUIRE((p2Ref.z == T(-2)).All() == true);
    }

    SECTION("Transforms - Reference Inverse")
    {
        Transform4 trans = Transform4::Translation(10.0f, 20.0f, 30.0f);
        Transform4 invTrans = Inverse(trans);
    
        Point3 p(5.0f, 5.0f, 5.0f);
        Point3 pIdentity = invTrans * (trans * p);
    
        REQUIRE((pIdentity.x == T(5)).All() == true);
        REQUIRE((pIdentity.y == T(5)).All() == true);
        REQUIRE((pIdentity.z == T(5)).All() == true);
    
        Point3 pInv = invTrans * p;
        REQUIRE((pInv.x == T(-5)).All() == true);
        REQUIRE((pInv.y == T(-15)).All() == true);
        REQUIRE((pInv.z == T(-25)).All() == true);
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
        REQUIRE((pCombined.x == T(12.0f)).All() == true);
        REQUIRE((pCombined.y == T(2.0f)).All() == true);
        REQUIRE((pCombined.z == T(2.0f)).All() == true);
    }

    SECTION("Transforms - Reference LookAt")
    {
        Vector3 eye(0.0f, 0.0f, 5.0f);
        Vector3 center(0.0f, 0.0f, 0.0f);
        Vector3 up(0.0f, 1.0f, 0.0f);
    
        Matrix4 lookAt = Matrix4::LookAtView(eye, center, up);
        Transform4 lookAtT(lookAt);
    
        // The eye position should map to the origin in view space
        Point3 pEyePoint(0.0f, 0.0f, 5.0f);
        Point3 pView = lookAtT * pEyePoint;
    
        REQUIRE((pView.x == T(0.0)).All() == true);
        REQUIRE((pView.y == T(0.0)).All() == true);
        REQUIRE((pView.z == T(0.0)).All() == true);
    }

    SECTION("Transforms - Reference Projection Setup")
    {
        // Simply check standard layout of an orthogonal projection matrix
        ScalarType left = -10.0f, right = 10.0f, bottom = -10.0f, top = 10.0f, near = 1.0f, far = 100.0f;
        Matrix4 ortho = Matrix4::OrthoProjection(left, right, top, bottom, near, far);
    
        // Ortho matrix M(0,0) = 2/(right - left) = 2/20 = 0.1
        // M(1,1) = 2/(top - bottom) = 2/20 = 0.1
        // M(2,2) = -2/(far - near) = -2/99 or similar depending on depth convention
        REQUIRE((ortho(0, 0) == T(0.1)).All() == true);
        REQUIRE((ortho(1, 1) == T(0.1)).All() == true);
    
        ScalarType fovY = ScalarType(M_PI) / T(2);
        ScalarType aspect = T(1);
        Matrix4 persp = Transform4::Perspective(fovY, aspect, near, far);
    
        // Perspective M(0,0) = 1 / (aspect * tan(fovY/2)) = 1 / tan(PI/4) = 1.0
        // M(1,1) = 1 / tan(fovY/2) = 1.0
        REQUIRE((persp(0, 0) == T(1.0)).All() == true);
        REQUIRE((persp(1, 1) == T(1.0)).All() == true);
    }
}