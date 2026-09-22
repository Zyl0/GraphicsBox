#pragma once

#include "Math/Functons.h"
#include "Math/Vector.h"
#include "Math/Plane.h"
#include "Math/Matrix.h"
#include "Math/Quaternion.h"
#include "Math/Box.h"
#include "Math/Geometry.h"
#include "Math/Transforms.h"
#include "Math/ComponentTransform.h"
#include "Math/Simd.h"

#include <cmath>

namespace Math
{
    template<typename Type, typename BlendType>
    inline Type LinearInterpolate(const Type& A, const Type& B, const BlendType& alpha)
        {return alpha * B + (1 - alpha) * A;}
    
    template<>
    INLINE Vector2f LinearInterpolate<Vector2f, Vector2f>(const Vector2f& A, const Vector2f& B, const Vector2f& alpha)
    {
        return {
            LinearInterpolate(A.x, B.x, alpha.x),
            LinearInterpolate(A.y, B.y, alpha.y)
        };
    }
    
    template<>
    INLINE Vector2d LinearInterpolate<Vector2d, Vector2d>(const Vector2d& A, const Vector2d& B, const Vector2d& alpha)
    {
        return {
            LinearInterpolate(A.x, B.x, alpha.x),
            LinearInterpolate(A.y, B.y, alpha.y)
        };
    }
    
    template<>
    INLINE Vector3f LinearInterpolate<Vector3f, Vector3f>(const Vector3f& A, const Vector3f& B, const Vector3f& alpha)
    {
        return {
            LinearInterpolate(A.x, B.x, alpha.x),
            LinearInterpolate(A.y, B.y, alpha.y),
            LinearInterpolate(A.z, B.z, alpha.z)
        };
    }
    
    template<>
    INLINE Vector3d LinearInterpolate<Vector3d, Vector3d>(const Vector3d& A, const Vector3d& B, const Vector3d& alpha)
    {
        return {
            LinearInterpolate(A.x, B.x, alpha.x),
            LinearInterpolate(A.y, B.y, alpha.y),
            LinearInterpolate(A.z, B.z, alpha.z)
        };
    }
    
    template<>
    INLINE Point3f LinearInterpolate<Point3f, Point3f>(const Point3f& A, const Point3f& B, const Point3f& alpha)
    {
        return {
            LinearInterpolate(A.x, B.x, alpha.x),
            LinearInterpolate(A.y, B.y, alpha.y),
            LinearInterpolate(A.z, B.z, alpha.z)
        };
    }
    
    template<>
    INLINE Point3d LinearInterpolate<Point3d, Point3d>(const Point3d& A, const Point3d& B, const Point3d& alpha)
    {
        return {
            LinearInterpolate(A.x, B.x, alpha.x),
            LinearInterpolate(A.y, B.y, alpha.y),
            LinearInterpolate(A.z, B.z, alpha.z)
        };
    }
    
    template<>
    INLINE Vector4f LinearInterpolate<Vector4f, Vector4f>(const Vector4f& A, const Vector4f& B, const Vector4f& alpha)
    {
        return {
            LinearInterpolate(A.x, B.x, alpha.x),
            LinearInterpolate(A.y, B.y, alpha.y),
            LinearInterpolate(A.z, B.z, alpha.z),
            LinearInterpolate(A.w, B.w, alpha.w)
        };
    }
    
    template<>
    INLINE Vector4d LinearInterpolate<Vector4d, Vector4d>(const Vector4d& A, const Vector4d& B, const Vector4d& alpha)
    {
        return {
            LinearInterpolate(A.x, B.x, alpha.x),
            LinearInterpolate(A.y, B.y, alpha.y),
            LinearInterpolate(A.z, B.z, alpha.z),
            LinearInterpolate(A.w, B.w, alpha.w)
        };
    }
    
    /*
    template<typename Type>
    inline Type LinearInterpolate(const Type& A, const Type& B, const Type& alpha)
        {return LinearInterpolate<Type,Type>(A, B, alpha);}

    template<typename Type>
    inline Type LinearInterpolate(const Type& A, const Type& B, float alpha)
        {return LinearInterpolate<Type,float>(A, B, alpha);}

    template<typename Type>
    inline Type LinearInterpolate(const Type& A, const Type& B, double alpha)
        {return LinearInterpolate<Type,double>(A, B, alpha);}
    */
    
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
    
    /*
    template<typename Type>
    inline Type BiLinearInterpolate(const Type& p00, const Type& p10, const Type& p01, const Type& p11, const Type& u, const Type& v)
        {return BiLinearInterpolate<Type,Type>(p00, p10, p01, p11, u, v);}

    template<typename Type>
    inline Type BiLinearInterpolate(const Type& p00, const Type& p10, const Type& p01, const Type& p11, float u, float v)
        {return BiLinearInterpolate<Type,float>(p00, p10, p01, p11, u, v);}
    
    template<typename Type>
    inline Type BiLinearInterpolate(const Type& p00, const Type& p10, const Type& p01, const Type& p11, double u, double v)
        {return BiLinearInterpolate<Type,double>(p00, p10, p01, p11, u, v);}
    */
    
    constexpr double Pi = M_PI;
}