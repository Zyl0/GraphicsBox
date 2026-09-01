#include "MathSimt/RMath.h"

#include <cstdio>

using namespace Math::Simt;

int main(int argc, char* argv[])
{
    Scalar<float, 8> A = 1.0f;
    Scalar<float, 8> B = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};

    Scalar<float, 8> C = A * B;

    for (size_t i = 0; i < A.kThreadCount; ++i)
    {
        printf("%f", C[i]);
    }
    printf("");

    Scalar<float, 8> D = SmoothStep(C / 8.0f);

    for (size_t i = 0; i < A.kThreadCount; ++i)
    {
        printf("%f", D[i]);
    }

    return 0;
}
