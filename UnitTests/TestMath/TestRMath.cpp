#include <catch2/catch_all.hpp>
#include "Math/RMath.h"

using namespace Math;

TEST_CASE("RMath - Interpolations")
{
    float val1 = LinearInterpolate(0.0f, 10.0f, 0.5f);
    REQUIRE(val1 == Catch::Approx(5.0f));
    
    float val2 = LinearInterpolate(0.0f, 10.0f, 0.25f);
    REQUIRE(val2 == Catch::Approx(2.5f));
    
    double val3 = LinearInterpolate(0.0, 10.0, 0.5);
    REQUIRE(val3 == Catch::Approx(5.0));

    // BiLinearInterpolate: p00, p10, p01, p11, u, v
    float bival1 = BiLinearInterpolate(0.0f, 10.0f, 0.0f, 10.0f, 0.5f, 0.5f);
    // (1-u)(1-v)*p00 + u(1-v)*p10 + (1-u)v*p01 + u*v*p11
    // (0.5)(0.5)*0 + (0.5)(0.5)*10 + (0.5)(0.5)*0 + (0.5)(0.5)*10 = 2.5 + 2.5 = 5.0
    REQUIRE(bival1 == Catch::Approx(5.0f));
    
    float bival2 = BiLinearInterpolate(1.0f, 2.0f, 3.0f, 4.0f, 0.0f, 0.0f);
    REQUIRE(bival2 == Catch::Approx(1.0f));
    
    float bival3 = BiLinearInterpolate(1.0f, 2.0f, 3.0f, 4.0f, 1.0f, 1.0f);
    REQUIRE(bival3 == Catch::Approx(4.0f));
    
    REQUIRE(Catch::Approx(Pi) == M_PI);
}
