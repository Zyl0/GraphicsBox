#include "Modules/Rendering/Tools/DebugModule.h"

#include "Core/Spec.h"
#include "Modules/Rendering/Module.h"
#include "Rendering/Uniforms.h"

namespace Rendering::Debug
{
    void Module::RegisterDependencies(Engine::Spec& spec)
    {
        spec.Register<Rendering::Module>();
    }

    void Module::RegisterComponents()
    {
        IModule::RegisterComponents();
    }

    void Module::Initialize()
    {
        m_DrawFrustum.emplace(PipelineFromFile("Debug Frustum", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Debug/Frustum.glsl"));
        m_DrawBox.emplace(PipelineFromFile("Debug Box", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Debug/Box.glsl"));
        m_DrawRay.emplace(PipelineFromFile("Debug Ray", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Debug/Ray.glsl"));
        m_VAO.emplace();
    }

    void Module::Tick(double deltaTime)
    {
    }

    void Module::Shutdown()
    {
        m_DrawFrustum.reset();
        m_DrawBox.reset();
        m_DrawRay.reset();
        m_VAO.reset();
    }

    void Module::DrawFrustum(const Math::Matrix4f& SourceCamera, const Math::Matrix4f& TargetCamera) const
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Bind(*m_DrawFrustum);
        Bind(*m_VAO);

        SetUniform(*m_DrawFrustum, "useCameraBuffer", false);
        SetUniform(*m_DrawFrustum, "SourceMatrixProjToWorld", SourceCamera);
        SetUniform(*m_DrawFrustum, "TargetMatrixWorldToProj", TargetCamera);

        glDrawArrays(GL_QUADS, 0, 24);

        UnBind(*m_VAO);
        UnBind(*m_DrawFrustum);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }

    void Module::DrawFrustum(const StorageBuffer& CameraBuffer, uint32_t SourceCamera, uint32_t TargetCamera) const
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Bind(*m_DrawFrustum);
        Bind(*m_VAO);

        SetUniform(*m_DrawFrustum, "useCameraBuffer", true);
        SetUniform(0, CameraBuffer);
        SetUniform(*m_DrawFrustum, "SourceCamera", SourceCamera);
        SetUniform(*m_DrawFrustum, "TargetCamera", TargetCamera);

        glDrawArrays(GL_QUADS, 0, 24);

        UnBind(*m_VAO);
        UnBind(*m_DrawFrustum);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }

    void Module::DrawBox(const Math::Matrix4f& Camera, const Math::Box3f& Box, const Math::Vector3f& Color, const Math::Matrix4f& Transform) const
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Bind(*m_DrawBox);
        Bind(*m_VAO);

        SetUniform(*m_DrawBox, "useCameraBuffer", false);
        SetUniform(*m_DrawBox, "CameraWorldToProj", Camera);
        SetUniform(*m_DrawBox, "Model", Transform);
        SetUniform(*m_DrawBox, "BoxMin", Box.a);
        SetUniform(*m_DrawBox, "BoxMax", Box.b);
        SetUniform(*m_DrawBox, "BoxColor", Color);

        glDrawArrays(GL_QUADS, 0, 24);

        UnBind(*m_VAO);
        UnBind(*m_DrawBox);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }

    void Module::DrawBox(const StorageBuffer& CameraBuffer, uint32_t Camera, const Math::Box3f& Box, const Math::Vector3f& Color, const Math::Matrix4f& Transform) const
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Bind(*m_DrawBox);
        Bind(*m_VAO);

        SetUniform(*m_DrawBox, "useCameraBuffer", true);
        SetUniform(0, CameraBuffer);
        SetUniform(*m_DrawBox, "Camera", Camera);
        SetUniform(*m_DrawBox, "Model", Transform);
        SetUniform(*m_DrawBox, "BoxMin", Box.a);
        SetUniform(*m_DrawBox, "BoxMax", Box.b);
        SetUniform(*m_DrawBox, "BoxColor", Color);

        glDrawArrays(GL_QUADS, 0, 24);

        UnBind(*m_VAO);
        UnBind(*m_DrawBox);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }

    void Module::DrawRay(const Math::Matrix4f& WorldToProj, Math::Point3f Origin, float Distance,
                         Math::Vector3f Direction, float HitDistance) const
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Bind(*m_DrawRay);
        Bind(*m_VAO);

        SetUniform(*m_DrawRay, "useCameraBuffer", false);
        SetUniform(*m_DrawRay, "CameraWorldToProj", WorldToProj);
        SetUniform(*m_DrawRay, "Position", Origin);
        SetUniform(*m_DrawRay, "Direction", Direction);
        SetUniform(*m_DrawRay, "Distance", Distance);
        SetUniform(*m_DrawRay, "HitDistance", HitDistance);

        glDrawArrays(GL_LINES, 0, 2);

        UnBind(*m_VAO);
        UnBind(*m_DrawRay);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }

    void Module::DrawRay(const StorageBuffer& CameraBuffer, Math::Point3f Origin, float Distance,
        Math::Vector3f Direction, float HitDistance) const
    {
        glDisable(GL_CULL_FACE);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        Bind(*m_DrawRay);
        Bind(*m_VAO);

        SetUniform(*m_DrawRay, "useCameraBuffer", true);
        SetUniform(0, CameraBuffer);
        SetUniform(*m_DrawRay, "TargetCamera", 0);
        SetUniform(*m_DrawRay, "Position", Origin);
        SetUniform(*m_DrawRay, "Direction", Direction);
        SetUniform(*m_DrawRay, "Distance", Distance);
        SetUniform(*m_DrawRay, "HitDistance", HitDistance);

        glDrawArrays(GL_LINES, 0, 2);

        UnBind(*m_VAO);
        UnBind(*m_DrawRay);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }
}
