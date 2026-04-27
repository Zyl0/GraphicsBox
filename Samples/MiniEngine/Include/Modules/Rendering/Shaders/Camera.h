#pragma once

#include "Camera/Camera.h"
#include "Math/Simd.h"
#include "Math/Vector.h"

namespace Rendering
{
    struct CameraData
    {
        // Matrices
        Math::AlignedMatrix4f       Camera_WorldToView;
        Math::AlignedMatrix4f       Camera_WorldToProj;
        Math::AlignedMatrix4f       Camera_ViewToWorld;
        Math::AlignedMatrix4f       Camera_ViewToProj;
        Math::AlignedMatrix4f       Camera_ProjToView;
        Math::AlignedMatrix4f       Camera_ProjToWorld;
    
        // Camera properties
        Math::AlignedVector3f       Camera_WorldPosition;
        Math::Vector3f              Camera_WorldUp;
        float                       Camera_AspectRatio;
        Math::AlignedVector3f       Camera_WorldForward;
        Math::AlignedVector3f       Camera_WorldRight;
    
        // Screen
        Math::Vector2f              Camera_ProjToViewport;
        Math::Vector2f              Camera_ViewportToProj;
    };

    void UpdateCameraData(CameraData& Data, const Camera& camera);
}
