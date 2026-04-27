#include "Modules/Rendering/Shaders/Camera.h"

namespace Rendering
{
    void UpdateCameraData(CameraData& Data, const Camera& camera)
    {
        Data.Camera_WorldToView    = camera.View();
        Data.Camera_ViewToProj     = camera.Projection();

        Data.Camera_ViewToWorld    = camera.InverseView();
        Data.Camera_ProjToView     = camera.InverseProjection();

        Data.Camera_WorldToProj    = camera.Projection() * camera.View();
        Data.Camera_ProjToWorld    = Inverse(camera.Projection() * camera.View());

        Data.Camera_WorldPosition  = camera.GetWorldPosition();
        Data.Camera_WorldForward   = camera.GetWorldDirection();
        Data.Camera_WorldUp        = camera.GetWorldUp();
        Data.Camera_WorldRight     = camera.GetWorldRight();
        Data.Camera_AspectRatio    = camera.GetAspectRatio();
    
        Data.Camera_ProjToViewport = Math::Vector2f(1.0f, -1.0f);
        Data.Camera_ViewportToProj = Math::Vector2f(1.0f, -1.0f);
    }
}
