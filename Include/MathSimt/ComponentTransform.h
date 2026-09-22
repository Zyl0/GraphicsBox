#pragma once

#include "Types.h"
#include "Vector.h"
#include "Matrix.h"
#include "Quaternion.h"
#include "Transforms.h"

namespace Math::Simt
{
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    struct WorldTransform
    {
        using Type = DataType;
        using ScalarType =  Scalar<DataType, ThreadCount>;
        using MaskType = typename Scalar<DataType, ThreadCount>::MaskType;
        using IndexerType = typename Scalar<DataType, ThreadCount>::IndexerType;

        static constexpr size_t kThreadCount = ThreadCount;
        static constexpr size_t kAlignment = sizeof(Type) * ThreadCount;
        static consteval size_t Size() {return ThreadCount;}
        
        using Point3 = Point3<DataType, ThreadCount>;
        using Quaternion = Quaternion<DataType, ThreadCount>;
        using Vector3 = Vector3<DataType, ThreadCount>;

        Point3 Position = Point3();
        Quaternion Rotation = Quaternion();
        Vector3 Scale = Vector3(Type(1));

        Transform4<DataType, ThreadCount> GetTransform() const
        {
            return Transform4<DataType, ThreadCount>::Translation(Position) *
                Transform4<DataType, ThreadCount>(Rotation.GetRotationMatrix()) *
                Transform4<DataType, ThreadCount>::Scale(Scale);
        }

        Point3 TransformPosition(const Point3& p) const
        {
            Vector3 result = p * Scale;
            result = Rotation(result);
            return Point3(result) + Position;
        }
        Point3 operator () (const Point3& p) const {return TransformPosition(p);}

        Vector3 TransformVector(const Vector3& v) const
        {
            Vector3 result = v * Scale;
            result = Rotation(result);
            return result;
        }
        
        Vector3 operator () (const Vector3& v) const {return TransformVector(v);}
        
        Vector3t<Type> Elt(size_t index) const
        {
            return {Position.Elt(index), Rotation.Elt(index), Scale.Elt(index)};
        }
    };

    /**
     * @return Parented Child world transform to Parent world transform
     */
    template<typename DataType, size_t ThreadCount> requires(std::is_arithmetic_v<DataType>)
    WorldTransform<DataType, ThreadCount> operator *(const WorldTransform<DataType, ThreadCount> &Parent, const WorldTransform<DataType, ThreadCount> &Child) 
    {
        WorldTransform<DataType, ThreadCount> result;
        result.Scale = Parent.Scale * Child.Scale;
        result.Rotation = Parent.Rotation * Child.Rotation;
        result.Position = Parent.TransformPosition(Child.Position);

        return result;
    }
}