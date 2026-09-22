#include <catch2/catch_all.hpp>
#include "Math/Functons.h"

using namespace Math;

TEST_CASE("Functons - Abs")
{
    REQUIRE(Abs(-5) == 5);
    REQUIRE(Abs(5) == 5);
    REQUIRE(Abs(-5.5f) == 5.5f);
    REQUIRE(Abs(5.5f) == 5.5f);
}

TEST_CASE("Functons - Radians & Degrees")
{
    REQUIRE(Catch::Approx(Radians(180.0f)) == M_PI);
    REQUIRE(Catch::Approx(Degrees((float)M_PI)) == 180.0f);
    REQUIRE(Catch::Approx(Radians(180.0)) == M_PI);
    REQUIRE(Catch::Approx(Degrees(M_PI)) == 180.0);
}

TEST_CASE("Functons - Clamp")
{
    REQUIRE(Clamp(5.0, 0.0, 10.0) == 5.0);
    REQUIRE(Clamp(-5.0, 0.0, 10.0) == 0.0);
    REQUIRE(Clamp(15.0, 0.0, 10.0) == 10.0);

    REQUIRE(Clamp(5.0f, 0.0f, 10.0f) == 5.0f);
    REQUIRE(Clamp(-5.0f, 0.0f, 10.0f) == 0.0f);
    REQUIRE(Clamp(15.0f, 0.0f, 10.0f) == 10.0f);

    REQUIRE(Clamp(5, 0, 10) == 5);
    REQUIRE(Clamp(-5, 0, 10) == 0);
    REQUIRE(Clamp(15, 0, 10) == 10);
    
    unsigned int u5 = 5, u0 = 0, u10 = 10, u15 = 15;
    REQUIRE(Clamp(u5, u0, u10) == u5);
    REQUIRE(Clamp(u15, u0, u10) == u10);
}

TEST_CASE("Functons - SmoothStep")
{
    REQUIRE(SmoothStep(0.0f) == 0.0f);
    REQUIRE(SmoothStep(1.0f) == 1.0f);
    REQUIRE(SmoothStep(0.5f) == 0.5f);
    REQUIRE(SmoothStep(0.0) == 0.0);
    REQUIRE(SmoothStep(1.0) == 1.0);
    REQUIRE(SmoothStep(0.5) == 0.5);
}

TEST_CASE("Functons - InverseLerp")
{
    REQUIRE(InverseLerp(0.0f, 10.0f, 5.0f) == 0.5f);
    REQUIRE(InverseLerp(0.0, 10.0, 5.0) == 0.5);
}

TEST_CASE("Functons - Saturate")
{
    REQUIRE(Saturate(0.5f) == 0.5f);
    REQUIRE(Saturate(-0.5f) == 0.0f);
    REQUIRE(Saturate(1.5f) == 1.0f);

    REQUIRE(Saturate(0.5) == 0.5);
    REQUIRE(Saturate(-0.5) == 0.0);
    REQUIRE(Saturate(1.5) == 1.0);
}
