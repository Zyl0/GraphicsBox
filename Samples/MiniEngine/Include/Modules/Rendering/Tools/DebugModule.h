#pragma once

#include <optional>

#include "Rendering/Pipelines.h"
#include "Rendering/StorageBuffer.h"
#include "Rendering/VertexArrayObject.h"

#include "Core/Module.h"

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
        void DrawRay(const Math::Matrix4f& WorldToProj, Math::Point3f Origin, float Distance, Math::Vector3f Direction, float HitDistance) const;
        void DrawRay(const StorageBuffer& CameraBuffer, Math::Point3f Origin, float Distance, Math::Vector3f Direction, float HitDistance) const;

    private:
        std::optional<Pipeline> m_DrawFrustum;
        std::optional<Pipeline> m_DrawRay;
        std::optional<VertexArrayObject> m_VAO;
    };
}
