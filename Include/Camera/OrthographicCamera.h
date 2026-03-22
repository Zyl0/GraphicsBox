#pragma once
#include "Camera.h"

class OrthographicCamera : public Camera
{
public:
    OrthographicCamera();
    
    void SetOrthographicProjection(float HorizontalSize, float VerticalSize, float ZNear, float ZFar);

    void LookAt(Math::Vector3f from, Math::Vector3f at, Math::Vector3f up);
};
