#include "Camera/FlyCamera.h"

void FlyCamera::Translate(float x, float y, float z)
{
    Position().x += x;
    Position().y += y;
    Position().z += z;
    
    UpdateView();
}

void FlyCamera::SetTranslation(float x, float y, float z)
{
    Position().x = x;
    Position().y = y;
    Position().z = z;
    
    UpdateView();
}

void FlyCamera::RotateRadians(float Pitch, float Yaw)
{
    m_Pitch += Pitch;
    m_Yaw   += Yaw;
    Rotation() = Rotation() * Math::QuaternionF(Pitch, Yaw, 0);
    
    Direction().x = cos(m_Yaw) * cos(m_Pitch);
    Direction().y = sin(m_Pitch);
    Direction().z = sin(m_Yaw) * cos(m_Pitch);

    Right() = Normalize(Cross(Math::Vector3f(0,1,0), Direction()));
    Up()    = Normalize(Cross(Direction(), Right()));

    UpdateView();
}

void FlyCamera::SetRotationRadians(float Pitch, float Yaw)
{
    m_Pitch = Pitch;
    m_Yaw   = Yaw;
    Rotation() = Math::QuaternionF(Pitch, Yaw, 0);
    
    Direction().x = cos(m_Yaw) * cos(m_Pitch);
    Direction().y = sin(m_Pitch);
    Direction().z = sin(m_Yaw) * cos(m_Pitch);

    Right() = Normalize(Cross(Math::Vector3f(0,1,0), Direction()));
    Up()    = Normalize(Cross(Direction(), Right()));

    UpdateView();
}

void FlyCamera::SetProjection(unsigned Width, unsigned Height, float FieldOfView, float NearDistance, float FarDistance)
{
    AspectRatio() = static_cast<float>(Width) / static_cast<float>(Height);
    m_FieldOfView = FieldOfView;
    m_NearDistance = NearDistance;
    m_FarDistance = FarDistance;
    
    UpdateProjection();
}

void FlyCamera::SetProjection(float AspectRatio, float FieldOfView, float NearDistance, float FarDistance)
{
    this->AspectRatio() = AspectRatio;
    m_FieldOfView = FieldOfView;
    m_NearDistance = NearDistance;
    m_FarDistance = FarDistance;
    
    UpdateProjection();
}

Math::Transform4f FlyCamera::ComputeView()
{
    m_Rotator = Math::Matrix3f(
                Right().x, Right().y, Right().z,
                Up().x, Up().y, Up().z,
                Direction().x, Direction().y, Direction().z
                );
    return
        Math::Transform4f(
            m_Rotator
            )
        *
        Math::MakeHomogeneousTranslation(-Position());
}

Math::Matrix4f FlyCamera::ComputeProjection() const
{
    float ZNear = std::max(float(0.1), m_NearDistance);
    float ZFar = std::max(float(1), m_FarDistance);
    
    return
        Math::MakeFrustumProjection(m_FieldOfView, AspectRatio(), ZNear, ZFar);
        //* Math::MakeHomogeneousPerspective(m_FieldOfView, m_AspectRatio, m_NearDistance, m_FarDistance);
}
