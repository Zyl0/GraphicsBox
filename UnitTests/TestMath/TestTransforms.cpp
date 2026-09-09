#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

using namespace Math;

TEST_CASE("Transforms - Reference Translation")
{
    Transform4f trans = MakeHomogeneousTranslation<float>(10.0f, -5.0f, 3.0f);
    
    // Translation applied to a point should shift it
    Point3f p(1.0f, 1.0f, 1.0f);
    Point3f pTransformed = trans * p;
    REQUIRE(pTransformed.x == Catch::Approx(11.0f));
    REQUIRE(pTransformed.y == Catch::Approx(-4.0f));
    REQUIRE(pTransformed.z == Catch::Approx(4.0f));

    // Translation applied to a vector should have no effect
    Vector3f v(1.0f, 1.0f, 1.0f);
    Vector3f vTransformed = trans * v;
    REQUIRE(vTransformed.x == Catch::Approx(1.0f));
    REQUIRE(vTransformed.y == Catch::Approx(1.0f));
    REQUIRE(vTransformed.z == Catch::Approx(1.0f));
}

TEST_CASE("Transforms - Reference Scale")
{
    Transform4f scale = MakeHomogeneousScale<float>(2.0f, 0.5f, -1.0f);

    Point3f p(10.0f, 10.0f, 10.0f);
    Point3f pScaled = scale * p;
    REQUIRE(pScaled.x == Catch::Approx(20.0f));
    REQUIRE(pScaled.y == Catch::Approx(5.0f));
    REQUIRE(pScaled.z == Catch::Approx(-10.0f));

    Vector3f v(10.0f, 10.0f, 10.0f);
    Vector3f vScaled = scale * v;
    REQUIRE(vScaled.x == Catch::Approx(20.0f));
    REQUIRE(vScaled.y == Catch::Approx(5.0f));
    REQUIRE(vScaled.z == Catch::Approx(-10.0f));
}

TEST_CASE("Transforms - Reference Rotations")
{
    float piOver2 = (float)M_PI / 2.0f;
    
    // Right-handed rotation around X-axis
    Transform4f rotX = MakeHomogeneousRotationX<float>(piOver2);
    Point3f px(0.0f, 1.0f, 0.0f);
    Point3f pxRot = rotX * px;
    // Y unit vector rotated by 90 degrees around X goes to Z
    REQUIRE(Catch::Approx(pxRot.x).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(pxRot.y).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(pxRot.z).margin(0.0001f) == 1.0f);

    // Right-handed rotation around Y-axis
    Transform4f rotY = MakeHomogeneousRotationY<float>(piOver2);
    Point3f py(1.0f, 0.0f, 0.0f);
    Point3f pyRot = rotY * py;
    // X unit vector rotated by 90 degrees around Y goes to -Z
    REQUIRE(Catch::Approx(pyRot.x).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(pyRot.y).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(pyRot.z).margin(0.0001f) == -1.0f);

    // Right-handed rotation around Z-axis
    Transform4f rotZ = MakeHomogeneousRotationZ<float>(piOver2);
    Point3f pz(1.0f, 0.0f, 0.0f);
    Point3f pzRot = rotZ * pz;
    // X unit vector rotated by 90 degrees around Z goes to Y
    REQUIRE(Catch::Approx(pzRot.x).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(pzRot.y).margin(0.0001f) == 1.0f);
    REQUIRE(Catch::Approx(pzRot.z).margin(0.0001f) == 0.0f);

    // Arbitrary axis rotation (around Y-axis as test)
    Transform4f rotA = MakeHomogeneousRotation<float>(Vector3f(0.0f, 1.0f, 0.0f), piOver2);
    Point3f pa(1.0f, 0.0f, 0.0f);
    Point3f paRot = rotA * pa;
    REQUIRE(Catch::Approx(paRot.x).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(paRot.y).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(paRot.z).margin(0.0001f) == -1.0f);
}

TEST_CASE("Transforms - Reference Reflection")
{
    // Reflection across YZ plane (normal is X-axis)
    PlaneF plane(1.0f, 0.0f, 0.0f, 0.0f); 
    Transform4f refT = MakeHomogeneousReflection<float>(plane);
    
    Point3f p(5.0f, 3.0f, -2.0f);
    Point3f pRef = refT * p;
    REQUIRE(pRef.x == Catch::Approx(-5.0f));
    REQUIRE(pRef.y == Catch::Approx(3.0f));
    REQUIRE(pRef.z == Catch::Approx(-2.0f));

    // Vector reflection
    Transform4f refVec = MakeHomogeneousReflection<float>(Vector3f(0.0f, 1.0f, 0.0f));
    Point3f p2(5.0f, 3.0f, -2.0f);
    Point3f p2Ref = refVec * p2;
    REQUIRE(p2Ref.x == Catch::Approx(5.0f));
    REQUIRE(p2Ref.y == Catch::Approx(-3.0f));
    REQUIRE(p2Ref.z == Catch::Approx(-2.0f));
}

TEST_CASE("Transforms - Reference Inverse")
{
    Transform4f trans = MakeHomogeneousTranslation<float>(10.0f, 20.0f, 30.0f);
    Transform4f invTrans = Inverse(trans);
    
    Point3f p(5.0f, 5.0f, 5.0f);
    Point3f pIdentity = invTrans * (trans * p);
    
    REQUIRE(pIdentity.x == Catch::Approx(5.0f));
    REQUIRE(pIdentity.y == Catch::Approx(5.0f));
    REQUIRE(pIdentity.z == Catch::Approx(5.0f));
    
    Point3f pInv = invTrans * p;
    REQUIRE(pInv.x == Catch::Approx(-5.0f));
    REQUIRE(pInv.y == Catch::Approx(-15.0f));
    REQUIRE(pInv.z == Catch::Approx(-25.0f));
}

TEST_CASE("Transforms - Reference Composition")
{
    Transform4f scale = MakeHomogeneousScale<float>(2.0f, 2.0f, 2.0f);
    Transform4f trans = MakeHomogeneousTranslation<float>(10.0f, 0.0f, 0.0f);
    
    // Expected composition T = trans * scale
    // Applying T to p means trans * (scale * p)
    Transform4f combined = trans * scale;
    Point3f p(1.0f, 1.0f, 1.0f);
    Point3f pCombined = combined * p;
    
    // Scale first (2,2,2) then translate by (10,0,0) => (12, 2, 2)
    REQUIRE(pCombined.x == Catch::Approx(12.0f));
    REQUIRE(pCombined.y == Catch::Approx(2.0f));
    REQUIRE(pCombined.z == Catch::Approx(2.0f));
}

TEST_CASE("Transforms - Reference LookAt")
{
    Vector3f eye(0.0f, 0.0f, 5.0f);
    Vector3f center(0.0f, 0.0f, 0.0f);
    Vector3f up(0.0f, 1.0f, 0.0f);
    
    Matrix4f lookAt = MakeLookAtView<float>(eye, center, up);
    Transform4f lookAtT(lookAt);
    
    // The eye position should map to the origin in view space
    Point3f pEyePoint(0.0f, 0.0f, 5.0f);
    Point3f pView = lookAtT * pEyePoint;
    
    REQUIRE(Catch::Approx(pView.x).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(pView.y).margin(0.0001f) == 0.0f);
    REQUIRE(Catch::Approx(pView.z).margin(0.0001f) == 0.0f);
}

TEST_CASE("Transforms - Reference Projection Setup")
{
    // Simply check standard layout of an orthogonal projection matrix
    float left = -10.0f, right = 10.0f, bottom = -10.0f, top = 10.0f, near = 1.0f, far = 100.0f;
    Matrix4f ortho = MakeOrthoProjection<float>(left, right, top, bottom, near, far);
    
    // Ortho matrix M(0,0) = 2/(right - left) = 2/20 = 0.1
    // M(1,1) = 2/(top - bottom) = 2/20 = 0.1
    // M(2,2) = -2/(far - near) = -2/99 or similar depending on depth convention
    REQUIRE(Catch::Approx(ortho(0, 0)).margin(0.0001f) == 0.1f);
    REQUIRE(Catch::Approx(ortho(1, 1)).margin(0.0001f) == 0.1f);
    
    float fovY = (float)M_PI / 2.0f;
    float aspect = 1.0f;
    Matrix4f persp = MakeHomogeneousPerspective<float>(fovY, aspect, near, far);
    
    // Perspective M(0,0) = 1 / (aspect * tan(fovY/2)) = 1 / tan(PI/4) = 1.0
    // M(1,1) = 1 / tan(fovY/2) = 1.0
    REQUIRE(Catch::Approx(persp(0, 0)).margin(0.0001f) == 1.0f);
    REQUIRE(Catch::Approx(persp(1, 1)).margin(0.0001f) == 1.0f);
}
