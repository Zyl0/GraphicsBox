#pragma once

#include "Modules/FrameGraph/Commands.h"
#include "Rendering/FrameBuffers.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"
#include "Rendering/Uniforms.h"

namespace FrameGraph
{
    class GBufferIndirectLightRadiance  : public ICommand
    {
    public:
        GBufferIndirectLightRadiance(CommandContext& Resources) :
            ICommand(Resources),
            VEmptyVAO(Resources.GetLocation<VertexArrayObject>("Empty VAO")),
            VSkylightMethod(Resources.GetLocation<UInt>("Skylight Method")),
            VIndirectLightSamples(Resources.AddVariable<UInt>("Indirect Sample Count", 32)),
            Cubemap(Resources.GetLocation<TextureCube>("Cubemap Skylight")),
            HDRi(Resources.GetLocation<Texture2D>("HDRi Skylight")),
            GBufferAlbedo(Resources.GetLocation<Texture2D>("GBufferAlbedo")),
            GBufferNormal(Resources.GetLocation<Texture2D>("GBufferNormal")),
            GBufferProperties(Resources.GetLocation<Texture2D>("GBufferProperties")),
            GBufferDepth(Resources.GetLocation<Texture2D>("Scene Depth")),
            FrameBuffer(FrameBuffer::Attachment(Resources.Get<Texture2D>("Scene Radiance"), FrameBuffer::ClearColor(0.0f))),
            CubemapPipeline(PipelineFromFile("Skylight To Radiance Cubemap", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/GBufferIndirectLightRadiance.glsl", PipelineCubemapDefines)),
            HDRiPipeline(PipelineFromFile("Skylight To Radiance HDRi", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/GBufferIndirectLightRadiance.glsl", PipelineHDRIDefines)),
            Sampler({
                .Magnification = Sampler::F_Nearest,
                .Minification = Sampler::F_Nearest,
            })
        {}
        ~GBufferIndirectLightRadiance() override = default;
        
    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineUpdateFromFile(CubemapPipeline, "Nodes/GBufferIndirectLightRadiance.glsl", PipelineCubemapDefines);
            PipelineUpdateFromFile(HDRiPipeline, "Nodes/GBufferIndirectLightRadiance.glsl", PipelineHDRIDefines);
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Scene Radiance"))
            {
                Size2D SceneRadianceSize = Resources.GetValue<Size2D>("Scene Radiance");
                FrameBuffer.Resize(SceneRadianceSize.x, SceneRadianceSize.y);
            }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            Bind(FrameBuffer);
            FrameBuffer.Clear();

            Bind(Resources.Get<VertexArrayObject>(VEmptyVAO));
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_COLOR, GL_ONE);
            
            const Pipeline* pipeline = nullptr;
            
            switch (Resources.GetValue<UInt>(VSkylightMethod))
            {
            case 0: // Cubemap Sampling
                Bind(CubemapPipeline);
                
                // Scene texture buffers
                SetUniform(CubemapPipeline, "SkyLightCubeMap", 4, Resources.Get<TextureCube>(Cubemap), Sampler);
                SetUniform(CubemapPipeline, "SkyLightMipCount", Resources.Get<TextureCube>(Cubemap).MipCount());
            
                pipeline = &CubemapPipeline;
                break;
            
            case 1: // HDRI Sampling
                Bind(HDRiPipeline);
                
                // Scene texture buffers
                SetUniform(HDRiPipeline, "SkyLightHDRi", 4, Resources.Get<Texture2D>(HDRi), Sampler);
                SetUniform(HDRiPipeline, "SkyLightMipCount", Resources.Get<Texture2D>(HDRi).MipCount());
            
                pipeline = &HDRiPipeline;
                break;
                        
            default:
                return;
            }

            SetUniform(*pipeline, "IndirectLightingSampleCount", Resources.GetValue<UInt>(VIndirectLightSamples));
            
            // Scene storage buffers
            SetUniform(0, Resources.GetCameraBuffer());
            
            SetUniform(*pipeline, "GBufferAlbedo", 0, Resources.Get<Texture2D>(GBufferAlbedo), Sampler);
            SetUniform(*pipeline, "GPackedNormalTangent", 1, Resources.Get<Texture2D>(GBufferNormal), Sampler);
            SetUniform(*pipeline, "GBufferProperties", 2, Resources.Get<Texture2D>(GBufferProperties), Sampler);
            SetUniform(*pipeline, "GBufferDepth", 3, Resources.Get<Texture2D>(GBufferDepth), Sampler);
            
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            glDisable(GL_BLEND);
            
            UnBind(*pipeline);
            UnBind(FrameBuffer);
        }
        
    private:
        Location VEmptyVAO;
        Location VSkylightMethod;
        Location VIndirectLightSamples;
        Location Cubemap;
        Location HDRi;
        Location GBufferAlbedo;
        Location GBufferNormal;
        Location GBufferProperties;
        Location GBufferDepth;
        FrameBuffer FrameBuffer;
        Shader::DefineArray<1> PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
        Shader::DefineArray<1> PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
        Pipeline CubemapPipeline;
        Pipeline HDRiPipeline;
        Sampler Sampler;
    };
}
