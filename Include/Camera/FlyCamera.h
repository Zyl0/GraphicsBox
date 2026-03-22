#pragma once

#include "Shared/Annotations.h"
#include "Camera.h"

class FlyCamera : public Camera
{
public:
    FlyCamera()
    {
        Position().x = 0;
        Position().y = 0;
        Position().z = 0;
        SetRotationRadians(0,0);
    }

    void Translate(float x, float y,  float z);
    
    INLINE void Translate(Math::Vector3f translation) {Translate(translation.x, translation.y, translation.z);}

    void SetTranslation(float x, float y,  float z);
    
    INLINE void SetTranslation(Math::Vector3f translation) {SetTranslation(translation.x, translation.y, translation.z);}

    void RotateRadians(float Pitch, float Yaw);

    INLINE void RotateDegrees(float Pitch, float Yaw) {RotateRadians(Math::Radians(Pitch), Math::Radians(Yaw));}

    void SetRotationRadians(float Pitch, float Yaw);

    INLINE void SetRotationDegrees(float Pitch, float Yaw) {SetRotationRadians(Math::Radians(Pitch), Math::Radians(Yaw));}

    void SetProjection(unsigned Width, unsigned Height, float FieldOfView, float NearDistance = 0.15, float FarDistance = 2.15);

    void SetProjection(float AspectRatio, float FieldOfView, float NearDistance = 0.15, float FarDistance = 2.15);

    const  Math::Matrix3f& GetRotator() const {return m_Rotator;}

    INLINE float NearDistance() const {return m_NearDistance;}
    INLINE float FarDistance() const {return m_FarDistance;}
    INLINE float FieldOfView() const {return m_FieldOfView;}

private:
    Math::Transform4f ComputeView();
    inline void UpdateView() {SetViewTransform(ComputeView());}

    Math::Matrix4f ComputeProjection() const;
    inline void UpdateProjection() {SetProjectionTransform(ComputeProjection());}

    Math::Matrix3f m_Rotator = Math::MakeIdentity<float>();

    float m_NearDistance = 0;
    float m_FarDistance = 0;

    float m_FieldOfView = 0;

    float m_Pitch = 0;
    float m_Yaw = 0;
};
