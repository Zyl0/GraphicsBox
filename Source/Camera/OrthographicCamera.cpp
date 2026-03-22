#include "Camera/OrthographicCamera.h"

OrthographicCamera::OrthographicCamera()
{
    SetViewTransform(Math::MakeMatrix4Identity<float>());
    SetProjectionTransform(Math::MakeMatrix4Identity<float>());
}

void OrthographicCamera::SetOrthographicProjection(float HorizontalSize, float VerticalSize, float ZNear, float ZFar)
{
    AspectRatio() = HorizontalSize / VerticalSize;
    
    SetProjectionTransform(Math::MakeOrthoProjection(-HorizontalSize, +HorizontalSize, -VerticalSize, +VerticalSize, ZNear, ZFar));
}

void OrthographicCamera::LookAt(Math::Vector3f from, Math::Vector3f at, Math::Vector3f up)
{
    Right() = Normalize( Cross(at, Normalize(up) ) );
    Up()    = Normalize( Cross(Right(), at) );
    Direction() = at;

    // todo rotation
    
    SetViewTransform(Math::MakeLookAtView(from, at, up));
}