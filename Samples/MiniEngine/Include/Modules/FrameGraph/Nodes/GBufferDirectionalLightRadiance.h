#pragma once

#include "Modules/FrameGraph/Commands.h"
#include "Rendering/FrameBuffers.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"
#include "Rendering/Uniforms.h"

namespace FrameGraph
{
    class GBufferDirectionalLightRadiance  : public ICommand
    {
    public:
        GBufferDirectionalLightRadiance(CommandContext& Resources) :
            ICommand(Resources),
            VEmptyVAO(Resources.GetLocation<VertexArrayObject>("Empty VAO")),
            GBufferAlbedo(Resources.GetLocation<Texture2D>("GBufferAlbedo")),
            GBufferNormal(Resources.GetLocation<Texture2D>("GBufferNormal")),
            GBufferProperties(Resources.GetLocation<Texture2D>("GBufferProperties")),
            GBufferDepth(Resources.GetLocation<Texture2D>("Scene Depth")),
            VDirectionalLightDirection(Resources.GetLocation<Math::Vector3f>("Light Direction")),
            VDirectionalLightColor(Resources.GetLocation<Math::Vector3f>("Light Color")),
            VDirectionalLightIntensity(Resources.GetLocation<Float>("Light Intensity")),
            SceneRadianceSize(Resources.GetValue<Size2D>("Output")),
            SceneRadiance(Resources.Add<Texture2D>("Scene Radiance", SceneRadianceSize.x, SceneRadianceSize.y, Texture::Type::Packed_R11F_G11F_B10F, Texture::Layout::RGB)),
            FrameBuffer(FrameBuffer::Attachment(Resources.Get<Texture2D>(SceneRadiance), FrameBuffer::ClearColor(0.0f))),
            Pipeline(PipelineFromFile("GBuffer Radiance Directional", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/GBufferDirectionalLightRadiance.glsl")),
            Sampler({
                .Magnification = Sampler::F_Nearest,
                .Minification = Sampler::F_Nearest,
            })
        {}
        ~GBufferDirectionalLightRadiance() override = default;
        
    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineUpdateFromFile(Pipeline, "Nodes/GBufferDirectionalLightRadiance.glsl");
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Scene Radiance"))
            {
                SceneRadianceSize = Resources.GetValue<Size2D>("Scene Radiance");
                FrameBuffer.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
            }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            Bind(Pipeline);
            Bind(FrameBuffer);
            FrameBuffer.Clear();
            Bind(Resources.Get<VertexArrayObject>(VEmptyVAO));
            
            // Scene storage buffers
            SetUniform(0, Resources.GetCameraBuffer());
        
            // GBuffer
            SetUniform(Pipeline, "GBufferAlbedo", 0, Resources.Get<Texture2D>(GBufferAlbedo), Sampler);
            SetUniform(Pipeline, "GPackedNormalTangent", 1, Resources.Get<Texture2D>(GBufferNormal), Sampler);
            SetUniform(Pipeline, "GBufferProperties", 2, Resources.Get<Texture2D>(GBufferProperties), Sampler);
            SetUniform(Pipeline, "GBufferDepth", 3, Resources.Get<Texture2D>(GBufferDepth), Sampler);
            
            // Light properties
            SetUniform(Pipeline, "LightDirection", Resources.GetValue<Math::Vector3f>(VDirectionalLightDirection));
            SetUniform(Pipeline, "LightColor", Resources.GetValue<Math::Vector3f>(VDirectionalLightColor));
            SetUniform(Pipeline, "LightIntensity", Resources.GetValue<Float>(VDirectionalLightIntensity));
                    
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
        
            UnBind(FrameBuffer);
            UnBind(Pipeline);
        }
        
    private:
        Location VEmptyVAO;
        Location GBufferAlbedo;
        Location GBufferNormal;
        Location GBufferProperties;
        Location GBufferDepth;
        Location VDirectionalLightDirection;
        Location VDirectionalLightColor;
        Location VDirectionalLightIntensity;
        Size2D SceneRadianceSize;
        Location SceneRadiance;
        FrameBuffer FrameBuffer;
        Pipeline Pipeline;
        Sampler Sampler;
    };
}
