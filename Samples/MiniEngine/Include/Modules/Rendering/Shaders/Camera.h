#pragma once

#include <span>

#include "Math/Vector.h"
#include "Math/Simd.h"
#include "Camera/Camera.h"
#include "Rendering/StorageBuffer.h"

namespace Rendering
{
    constexpr uint16_t kBaseCameraCount = 8;
    
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
    
    class CameraArray
    {
    public:
        CameraArray(): m_Cameras(kBaseCameraCount * sizeof(CameraData), nullptr), m_Size(kBaseCameraCount) {}
        CameraArray(size_t CameraCount) : m_Cameras(CameraCount * sizeof(CameraData), nullptr), m_Size(CameraCount) {}
        CameraArray(std::span<CameraData> Cameras) : m_Cameras(Cameras.size() * sizeof(CameraData), Cameras.data()), m_Size(Cameras.size()) {}
        ~CameraArray() = default;
        
        void UpdateCamera(size_t Index, const Camera& camera);
        void UpdateCamera(size_t Index, const CameraData& camera);
        void UpdateCameras(std::span<CameraData> Cameras, size_t Index = 0);
        
        INLINE GLuint Handle() const { return m_Cameras.Handle(); }
        INLINE const StorageBuffer& Buffer() const { return m_Cameras; }
        
        void Resize(size_t Size);
        INLINE size_t Size() const { return m_Size; }
        INLINE size_t SizeInBytes() const { return m_Size * sizeof(CameraData); }
        
    private:
        StorageBuffer m_Cameras;
        size_t m_Size = 0;
    };
}

// Rendering lib extension
INLINE void Bind(const Rendering::CameraArray& Cameras, uint32_t BindingPoint) {Bind(Cameras.Buffer(), BindingPoint);}
INLINE void UnBind(const Rendering::CameraArray& Cameras, uint32_t BindingPoint) {Bind(Cameras.Buffer(), BindingPoint);}
