#pragma once

#include "Shared/Annotations.h"
#include "Math/RMath.h"

class Camera
{
public:
    inline const Math::Transform4f &View() const {return m_view;}
    inline const Math::Matrix4f &Projection() const {return m_projection;}

    inline const Math::Transform4f &InverseView() const {return m_inverse_view;}
    inline const Math::Matrix4f &InverseProjection() const {return m_inverse_projection;}

    Math::Vector3f GetWorldPosition() const {return m_Position;}
    Math::Vector3f GetWorldDirection() const {return m_Direction;}
    Math::Vector3f GetWorldUp() const {return m_Up;}
    Math::Vector3f GetWorldRight() const {return m_Right;}
    Math::QuaternionF GetWorldRotation() const {return m_Rotation;}

    float GetAspectRatio() const {return m_AspectRatio;}
    
    Camera() = default;
    
    Camera(const Camera& Other)
        : m_view(Other.m_view),
          m_projection(Other.m_projection),
          m_inverse_view(Other.m_inverse_view),
          m_inverse_projection(Other.m_inverse_projection),
          m_Position(Other.m_Position),
          m_Direction(Other.m_Direction),
          m_Rotation(Other.m_Rotation),
          m_Up(Other.m_Up),
          m_Right(Other.m_Right),
          m_AspectRatio(Other.m_AspectRatio)
    {
    }

    Camera& operator=(const Camera& Other)
    {
        if (this == &Other)
            return *this;
        m_view = Other.m_view;
        m_projection = Other.m_projection;
        m_inverse_view = Other.m_inverse_view;
        m_inverse_projection = Other.m_inverse_projection;
        m_Position = Other.m_Position;
        m_Direction = Other.m_Direction;
        m_Rotation = Other.m_Rotation;
        m_Up = Other.m_Up;
        m_Right = Other.m_Right;
        m_AspectRatio = Other.m_AspectRatio;
        return *this;
    }

protected:
    inline void SetViewTransform(const Math::Transform4f &inView)
    {
        m_view = inView;
        m_inverse_view = Inverse(inView);
    }
    
    inline void SetProjectionTransform(const Math::Matrix4f &inProjection)
    {
        m_projection = inProjection;
        m_inverse_projection = Inverse(inProjection);
    }

    INLINE Math::Vector3f& Position()   {return m_Position;}
    INLINE Math::Vector3f& Direction()  {return m_Direction;}
    INLINE Math::Vector3f& Up()  {return m_Up;}
    INLINE Math::Vector3f& Right()  {return m_Right;}
    INLINE Math::QuaternionF& Rotation()   {return m_Rotation;}
    INLINE float& AspectRatio()   {return m_AspectRatio;}

    INLINE const Math::Vector3f& Position()     const {return m_Position;}
    INLINE const Math::Vector3f& Direction()    const {return m_Direction;}
    INLINE const Math::Vector3f& Up()    const {return m_Up;}
    INLINE const Math::Vector3f& Right()    const {return m_Right;}
    INLINE const Math::QuaternionF& Rotation()     const {return m_Rotation;}
    INLINE const float& AspectRatio()     const {return m_AspectRatio;}

private:
    Math::Transform4f m_view;
    Math::Matrix4f m_projection;
    
    Math::Transform4f m_inverse_view;
    Math::Matrix4f m_inverse_projection;

    Math::Vector3f m_Position = Math::Vector3f(0,0,0);
    Math::Vector3f m_Direction = Math::Vector3f(1,0,0);
    Math::QuaternionF m_Rotation = Math::QuaternionF(0,0,0);

    Math::Vector3f m_Up = Math::Vector3f(0, 1,0);
    Math::Vector3f m_Right = Math::Vector3f(0,0,1);

    float m_AspectRatio = 1.0f;
};
