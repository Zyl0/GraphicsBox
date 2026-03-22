#pragma once

#include "Camera.h"

//todo fix matrices

class OrbiterCamera : public Camera
{
public:
    OrbiterCamera() : m_center(), m_rotation(), m_size(5.f), m_radius(5.f), m_width(1), m_height(1), m_fov(45) {}
    
    void LookAt(const Math::Point3f &center, const float size);

    void LookAt(const Math::Point3f &pointNear, const Math::Point3f &pointFar);

    void SetProjection(int width, int height, const float fov);

    void Rotate(float x, float y, float z);
    inline void Rotate(Math::Vector3f rotation) {Rotate(rotation.x, rotation.y, rotation.z);}

    void Translate(float x, float y,  float z); 
    inline void Translate(Math::Vector3f translation) {Translate(translation.x, translation.y, translation.z);}
    
private:
    Math::Transform4f ComputeView();
    inline void UpdateView() {SetViewTransform(ComputeView());}

    Math::Matrix4f ComputeProjection();
    inline void UpdateProjection() {SetProjectionTransform(ComputeProjection());}

    Math::Matrix3f Rotator;
    
    float znear() const;
    float zfar() const;
    
    Math::Point3f m_center;
    Math::Vector3f m_rotation;
    float m_size;
    float m_radius;

    float m_width;
    float m_height;
    float m_fov;
};
