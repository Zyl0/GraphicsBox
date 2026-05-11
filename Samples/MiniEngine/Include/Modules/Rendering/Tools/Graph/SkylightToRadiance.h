#pragma once

#include "Modules/Rendering/Tools/Commands.h"
#include "Rendering/FrameBuffers.h"
#include "Rendering/Pipelines.h"
#include "Rendering/Sampler.h"
#include "Rendering/Uniforms.h"

namespace Rendering::Graph
{
    class SkylightToRadiance : public Command
    {
    public:
        SkylightToRadiance(CommandContext& Resources): 
            Command(Resources),
            VSkylightMethod(Resources.AddVariable<UInt>("Skylight Method", /* HDRi */ 1)),
            Cubemap(Resources.GetLocation<TextureCube>("Cubemap Skylight")),
            HDRi(Resources.GetLocation<Texture2D>("HDRi Skylight")),
            FrameBuffer(FrameBuffer::Attachment(Resources.Get<Texture2D>("Scene Radiance"), FrameBuffer::ClearColor(0.0f))),
            CubemapPipeline(PipelineFromFile("Skylight To Radiance Cubemap", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/SkylightToRadiance.glsl", PipelineCubemapDefines)),
            HDRiPipeline(PipelineFromFile("Skylight To Radiance HDRi", Pipeline::VERTEX_SHADER | Pipeline::FRAGMENT_SHADER, "Nodes/SkylightToRadiance.glsl", PipelineHDRIDefines)),
            Sampler({})
        {}
        ~SkylightToRadiance() override = default;

    protected:
        void OnReloadShaders(CommandContext& Resources) override
        {
            PipelineUpdateFromFile(CubemapPipeline, "Nodes/SkylightToRadiance.glsl", PipelineCubemapDefines);
            PipelineUpdateFromFile(HDRiPipeline, "Nodes/SkylightToRadiance.glsl", PipelineHDRIDefines);
        }
        
        void OnUpdate(CommandContext& Resources, double DeltaTime) override
        {
            if (Resources.HasChanged<Size2D>("Scene Radiance"))
            {
                Size2D size = Resources.GetValue<Size2D>("Scene Radiance");
                FrameBuffer.Resize(size.x, size.y);
            }
        }
        
        void OnExecute(const CommandContext& Resources) override
        {
            Bind(FrameBuffer);
            FrameBuffer.Clear();
            
            const Pipeline* pipeline = nullptr;
            
            switch (Resources.GetValue<UInt>(VSkylightMethod))
            {
            case 0: // Cubemap Sampling
                Bind(CubemapPipeline);
                
                // Scene texture buffers
                SetUniform(CubemapPipeline, "SkyLightCubeMap", 0, Resources.Get<TextureCube>(Cubemap), Sampler);
            
                pipeline = &CubemapPipeline;
                break;
            
            case 1: // HDRI Sampling
                Bind(HDRiPipeline);
                
                // Scene texture buffers
                SetUniform(HDRiPipeline, "SkyLightHDRi", 0, Resources.Get<Texture2D>(HDRi), Sampler);
            
                pipeline = &HDRiPipeline;
                break;
                        
            default:
                return;
            }
            
            // Scene storage buffers
            SetUniform(0, Resources.GetCameraBuffer());
            
            // Draw screen quad
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
            UnBind(*pipeline);
            UnBind(FrameBuffer);
        }
    
    private:
        Location VSkylightMethod;
        Location Cubemap;
        Location HDRi;
        FrameBuffer FrameBuffer;
        Shader::DefineArray<1> PipelineCubemapDefines = {Shader::Define("USE_CUBEMAP_SKYLIGHT", "")};
        Shader::DefineArray<1> PipelineHDRIDefines = {Shader::Define("USE_HDRI_SKYLIGHT", "")};
        Pipeline CubemapPipeline;
        Pipeline HDRiPipeline;
        
        Sampler Sampler;
    };
}
