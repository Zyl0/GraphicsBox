#pragma once

namespace Math
{
    struct AlignedVector2f
    {
        INLINE AlignedVector2f operator = (const Vector2f& other) {vector = other; return *this;}
        INLINE Vector2f&  operator () () {return vector;}
        
        alignas(8) Vector2f vector;
    };

    struct AlignedVector3f
    {
        INLINE AlignedVector3f operator = (const Vector3f& other) {vector = other; return *this;}
        INLINE Vector3f&  operator () () {return vector;}
        
        alignas(16) Vector3f vector;
    };

    struct AlignedVector4f
    {
        INLINE AlignedVector4f operator = (const Vector4f& other) {vector = other; return *this;}
        INLINE Vector4f&  operator () () {return vector;}
        
        alignas(16) Vector4f vector;
    };

    struct AlignedQuaternionF
    {
        INLINE AlignedQuaternionF operator = (const QuaternionF& other) {quaternion = other; return *this;}
        INLINE QuaternionF&  operator () () {return quaternion;}
        
        alignas(16) QuaternionF quaternion;
    };

    //todo AlignedMatrix33
    
    struct AlignedMatrix4f
    {
        INLINE AlignedMatrix4f operator = (const Matrix4f& other) {matrix = other; return *this;}
        INLINE AlignedMatrix4f operator = (const Transform4f& other) {matrix = other; return *this;}
        INLINE AlignedMatrix4f operator = (Matrix4f&& other) {matrix = std::move(other); return *this;}
        INLINE AlignedMatrix4f operator = (Transform4f&& other) {matrix = std::move(other); return *this;}
        INLINE Matrix4f&  operator () () {return matrix;}
        
        alignas(16) Matrix4f matrix;
    };

}