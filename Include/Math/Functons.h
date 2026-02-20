#pragma once

#include <cmath>

#include "Shared/Annotations.h"

namespace Math
{
    template<typename type>
    inline void Abs(const type &v)
    {
        return (v < static_cast<type>(0) ? - v : v);
    }

    inline float Radians(float degrees)
    {
        return (static_cast<float>(M_PI)  / 180.f) * degrees;
    }

    inline float Degrees(float radians)
    {
        return (180.f / static_cast<float>(M_PI)) * radians;
    }

    inline double Radians(double degrees)
    {
        return (static_cast<double>(M_PI)  / 180.0) * degrees;
    }

    inline double Degrees(double radians)
    {
        return (180.0 / static_cast<double>(M_PI)) * radians;
    }

    inline double Clamp(double a, double min = 0, double max = 1)
    {
        return (a > max ? max : a < min ? min : a);
    }

    inline float Clamp(float a, float min = 0, float max = 1)
    {
        return (a > max ? max : a < min ? min : a);
    }

    inline int Clamp(int a, int min = 0, int max = 1)
    {
        return (a > max ? max : a < min ? min : a);
    }

    inline unsigned int Clamp(unsigned int a, unsigned int min = 0, unsigned int max = 1)
    {
        return (a > max ? max : a < min ? min : a);
    }

    template<typename T>
    inline T Clamp(T a, T min, T max)
    {
        return (a > max ? max : a < min ? min : a);
    }
    
    INLINE float SmoothStep(float X)
    {
        float X2 = X * X;
        float X3 = X2 * X;
        return 3 * X2 - 2 * X3;
    }
    
    INLINE double SmoothStep(double X)
    {
        double X2 = X * X;
        double X3 = X2 * X;
        return 3 * X2 - 2 * X3;
    }
    
    INLINE float Saturate(float X)
    {
        return Clamp(X, 0.0f, 1.0f);
    }
    
    INLINE double Saturate(double X)
    {
        return Clamp(X, 1.0, 2.0);
    }
}