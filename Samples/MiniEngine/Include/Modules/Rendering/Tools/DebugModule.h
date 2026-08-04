#pragma once

#include <optional>

#include "Rendering/Pipelines.h"
#include "Rendering/StorageBuffer.h"
#include "Rendering/VertexArrayObject.h"

#include "Core/Module.h"
#include "Math/Box.h"
#include "RayTracing/Types.h"

namespace Rendering::Debug
{
    class Module : public Engine::IModule
    {
    public:
        Module() = default;
        ~Module() override = default;

        void RegisterDependencies(Engine::Spec& spec) override;

        void RegisterComponents() override;

        void Initialize() override;

        void Tick(double deltaTime) override;

        // Not needed
        void Shutdown() override;

        void DrawFrustum(const Math::Matrix4f& SourceCamera, const Math::Matrix4f& TargetCamera) const;
        void DrawFrustum(const StorageBuffer& CameraBuffer, uint32_t SourceCamera, uint32_t TargetCamera) const;
        void DrawBox(const Math::Matrix4f& Camera, const Math::Box3f& Box, const Math::Vector3f& Color, const Math::Matrix4f& Transform = Math::MakeMatrix4Identity<float>()) const;
        void DrawBox(const StorageBuffer& CameraBuffer, uint32_t Camera, const Math::Box3f& Box, const Math::Vector3f& Color, const Math::Matrix4f& Transform = Math::MakeMatrix4Identity<float>()) const;
        void DrawRay(const Math::Matrix4f& WorldToProj, Math::Point3f Origin, float Distance, Math::Vector3f Direction, float HitDistance) const;
        void DrawRay(const StorageBuffer& CameraBuffer, Math::Point3f Origin, float Distance, Math::Vector3f Direction, float HitDistance) const;

        INLINE void DrawRay(const Math::Matrix4f& WorldToProj, const Ray& Ray, float HitDistance)
        {
            DrawRay(WorldToProj, Ray.origin, Ray.distance, Ray.direction, HitDistance);
        }
        
        INLINE void DrawRay(const StorageBuffer& CameraBuffer, const Ray& Ray, float HitDistance)
        {
            DrawRay(CameraBuffer, Ray.origin, Ray.distance, Ray.direction, HitDistance);
        }
    private:
        std::optional<Pipeline> m_DrawFrustum;
        std::optional<Pipeline> m_DrawBox;
        std::optional<Pipeline> m_DrawRay;
        std::optional<VertexArrayObject> m_VAO;
    };
}
