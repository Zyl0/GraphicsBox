#include "Modules/Rendering/Shaders/Camera.h"

#include "Rendering/GLHelper.h"

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

    void CameraArray::UpdateCamera(size_t Index, const Camera& camera)
    {
        CameraData Payload;
        UpdateCameraData(Payload, camera);
        UpdateCamera(Index, Payload);
    }

    void CameraArray::UpdateCamera(size_t Index, const CameraData& camera)
    {
        m_Cameras.SubData(&camera, sizeof(CameraData), Index * sizeof(CameraData));
    }

    void CameraArray::UpdateCameras(std::span<CameraData> Cameras, size_t Index)
    {
        m_Cameras.SubData(Cameras.data(),  Cameras.size() * sizeof(CameraData), Index * sizeof(CameraData));
    }

    void CameraArray::Resize(size_t Size)
    {
        m_Cameras.Data(nullptr, Size * sizeof(CameraData));

        if (m_PreviousCameras.has_value())
        {
            m_PreviousCameras->Data(nullptr, Size * sizeof(CameraData));
        }
        
        m_Size = Size;
    }

    void CameraArray::SavePreviousCameras() const
    {
        AssertOrErrorCall(m_PreviousCameras.has_value(), return;, "Tried to copy cameras data to previous cameras data while previous camera is invalid. Missing call to EnablePreviousCameras")

        glCopyNamedBufferSubData(m_Cameras.Handle(), m_PreviousCameras->Handle(), 0, 0, m_Size * sizeof(CameraData));
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
    }
}
