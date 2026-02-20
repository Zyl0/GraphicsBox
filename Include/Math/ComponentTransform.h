#pragma once

// todo move to namespace

namespace Math
{
    template<typename type>
    struct WorldTransform
    {
        Point3t<type> Position = Math::Point3f();
        Math::QuaternionT<type> Rotation = Math::QuaternionF();
        Math::Vector3t<type> Scale = Math::Vector3f(static_cast<type>(1));

        Math::Transform4t<type> GetTransform() const;

        Math::Point3t<type> TransformPosition(const Math::Point3t<type>& p) const;
        Math::Point3t<type> operator () (const Math::Point3t<type>& p) const {return TransformPosition(p);}

        Math::Vector3t<type> TransformVector(const Math::Vector3t<type>& v) const;
        Math::Vector3t<type> operator () (const Math::Vector3t<type>& v) const {return TransformVector(v);}
    };

    using WorldTransformF = WorldTransform<float>;
    using WorldTransformD = WorldTransform<double>;
}

template <typename type>
Math::Transform4t<type> Math::WorldTransform<type>::GetTransform() const
{
    return Math::MakeHomogeneousTranslation(Position) *
        Math::Transform4t<type>(Rotation.GetRotationMatrix()) *
        Math::MakeHomogeneousScale(Scale);
}

template <typename type>
Math::Point3t<type> Math::WorldTransform<type>::TransformPosition(const Math::Point3t<type>& p) const
{
    Math::Vector3t<type> result = p * Scale;
    result = Rotation(result);
    return Math::Point3t<type>(result) + Position;
}

template <typename type>
Math::Vector3t<type> Math::WorldTransform<type>::TransformVector(const Math::Vector3t<type>& v) const
{
    Math::Vector3t<type> result = v * Scale;
    result = Rotation(result);
    return result;
}

/**
 * @return Parented Child world transform to Parent world transform
 */
template<typename type>
Math::WorldTransform<type> operator *(const Math::WorldTransform<type> &Parent, const Math::WorldTransform<type> &Child) 
{
    Math::WorldTransform<type> result;
    result.Scale = Parent.Scale * Child.Scale;
    result.Rotation = Parent.Rotation * Child.Rotation;
    result.Position = Parent.Position + Parent.Rotation(Child.Position);

    return result;
}


