#pragma once

#include "Math/Functons.h"
#include "Math/Vector.h"
#include "Math/Plane.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "Math/Gemometry.h"
#include "Math/Transforms.h"
#include "Math/ComponentTransform.h"
#include "Math/Simd.h"

#include <cmath>

namespace Math
{
    template<typename Type, typename BlendType>
    inline Type LinearInterpolate(const Type& A, const Type& B, const BlendType& alpha)
        {return alpha * B + (1 - alpha) * A;}

    template<typename Type>
    inline Type LinearInterpolate(const Type& A, const Type& B, const Type& alpha)
        {return LinearInterpolate<Type,Type>(A, B, alpha);}

    template<typename Type>
    inline Type LinearInterpolate(const Type& A, const Type& B, float alpha)
        {return LinearInterpolate<Type,float>(A, B, alpha);}

    template<typename Type>
    inline Type LinearInterpolate(const Type& A, const Type& B, double alpha)
        {return LinearInterpolate<Type,double>(A, B, alpha);}

    template<typename Type, typename BlendType>
    Type BiLinearInterpolate(const Type& p00, const Type& p10, const Type& p01, const Type& p11, const BlendType& u, const BlendType& v)
    {
        // Interpolate along the v direction
        Type p0 = LinearInterpolate<Type, BlendType>(p00, p10, v);
        Type p1 = LinearInterpolate<Type, BlendType>(p01, p11, v);

        // Interpolate along the u direction
        Type result = LinearInterpolate<Type, BlendType>(p0, p1, u);

        return result;
    }
    
    template<typename Type>
    inline Type BiLinearInterpolate(const Type& p00, const Type& p10, const Type& p01, const Type& p11, const Type& u, const Type& v)
        {return BiLinearInterpolate<Type,Type>(p00, p10, p01, p11, u, v);}

    template<typename Type>
    inline Type BiLinearInterpolate(const Type& p00, const Type& p10, const Type& p01, const Type& p11, float u, float v)
        {return BiLinearInterpolate<Type,float>(p00, p10, p01, p11, u, v);}
    
    template<typename Type>
    inline Type BiLinearInterpolate(const Type& p00, const Type& p10, const Type& p01, const Type& p11, double u, double v)
        {return BiLinearInterpolate<Type,double>(p00, p10, p01, p11, u, v);}

    constexpr double Pi = M_PI;
}