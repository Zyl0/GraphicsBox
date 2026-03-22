#include "Camera/OrbiterCamera.h"


void OrbiterCamera::LookAt(const Math::Point3f& center, const float size)
{
    m_center = center;
    Position() = Math::Vector3f(0, 0, 0);
    m_rotation = Math::Vector3f(0, 180, 0);
    m_size = size;
    m_radius = size;
    Rotator = Math::MakeIdentity<float>();

    UpdateView();
    UpdateProjection();
}

void OrbiterCamera::LookAt(const Math::Point3f& pointNear, const Math::Point3f& pointFar)
{
    LookAt(Math::Center(pointNear, pointFar), Math::Distance(pointNear, pointFar));
}

void OrbiterCamera::SetProjection(int width, int height, const float fov)
{
    m_width = width;
    m_height = height;
    m_fov = Math::Radians(fov);

    UpdateView();
    UpdateProjection();
}

void OrbiterCamera::Rotate(float x, float y, float z)
{
    Rotation() = Rotation() * Math::QuaternionF(x, y, z);
    m_rotation.x = m_rotation.x + y;
    m_rotation.y = m_rotation.y + x;
    m_rotation.z = m_rotation.z + z;

    Rotator = Math::MakeRotationX(m_rotation.x)
            * Math::MakeRotationY(m_rotation.y)
            * Math::MakeRotationZ(m_rotation.z);

    UpdateView();
    UpdateProjection();
}

void OrbiterCamera::Translate(float x, float y, float z)
{
    Math::Vector3f t = Rotator * Math::Vector3f(x,y,z);
    m_center.x = m_center.x - m_size * t.x;
    m_center.y = m_center.y + m_size * t.y;
    m_center.z = m_center.z - m_size * t.z;
    
    Position().x = Position().x - m_size * t.x;
    Position().y = Position().y + m_size * t.y;
    Position().z = Position().z - m_size * t.z;

    Direction() = Math::Normalize(m_center - Position());

    UpdateView();
    UpdateProjection();
}

Math::Transform4f OrbiterCamera::ComputeView()
{
    return Math::MakeHomogeneousTranslation( -Position().x, -Position().y, -m_size ) 
        * Math::Transform4f(Rotator)
        * Math::MakeHomogeneousTranslation( -m_center.x, -m_center.y, -m_center.z ); 
}

Math::Matrix4f OrbiterCamera::ComputeProjection()
{
    return Math::MakeHomogeneousPerspective(m_fov, m_width / m_height, znear(), zfar());
}

float OrbiterCamera::znear() const
{
    float d = Distance(m_center, Math::Point3f(Position().x, Position().y, m_size));
    return std::max(float(0.1), d - m_radius);
}

float OrbiterCamera::zfar() const
{
    float d = Distance(m_center, Math::Point3f(Position().x, Position().y, m_size));
    return std::max(float(1), d + m_radius);
}
